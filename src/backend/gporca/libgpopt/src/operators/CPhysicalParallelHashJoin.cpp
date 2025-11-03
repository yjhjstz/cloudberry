/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

//---------------------------------------------------------------------------
//	@filename:
//		CPhysicalParallelHashJoin.cpp
//
//	@doc:
//		Implementation of parallel hash join operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelHashJoin.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpecAny.h"
#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpecWorkerRandom.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CPhysicalParallelTableScan.h"
#include "gpopt/search/CGroupProxy.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::CPhysicalParallelHashJoin
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelHashJoin::CPhysicalParallelHashJoin(
	CMemoryPool *mp, CExpressionArray *pdrgpexprOuterKeys,
	CExpressionArray *pdrgpexprInnerKeys, IMdIdArray *hash_opfamilies,
	BOOL is_null_aware, CXform::EXformId origin_xform)
	: CPhysicalHashJoin(mp, pdrgpexprOuterKeys, pdrgpexprInnerKeys,
						hash_opfamilies, is_null_aware, origin_xform),
	  m_ulProbeWorkers(0),
	  m_ulBuildWorkers(0),
	  m_fWorkersExtracted(false)  // Workers will be extracted lazily
{
	// m_ulProbeWorkers and m_ulBuildWorkers will be extracted from child distributions in PdsDerive()
	// They must be extracted successfully, otherwise UlProbeWorkers()/UlBuildWorkers() will assert
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::~CPhysicalParallelHashJoin
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelHashJoin::~CPhysicalParallelHashJoin()
{
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::UlExtractWorkersFromGroup
//
//	@doc:
//		Extract worker count from a child group by scanning for parallel operators
//		This handles cases where Motion nodes hide WorkerRandom distributions
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelHashJoin::UlExtractWorkersFromGroup(CGroup *pgroup) const
{
	GPOS_ASSERT(nullptr != pgroup && "Child group is null - parallel hash join requires valid child groups");

	// Scan through all physical expressions in the child group
	CGroupProxy gpChild(pgroup);
	CGroupExpression *pgexprChild = gpChild.PgexprFirst();

	while (nullptr != pgexprChild)
	{
		COperator *popChild = pgexprChild->Pop();

		// Check for parallel table scan
		if (COperator::EopPhysicalParallelTableScan == popChild->Eopid())
		{
			CPhysicalParallelTableScan *popScan =
				CPhysicalParallelTableScan::PopConvert(popChild);
			return popScan->UlParallelWorkers();
		}

		// Check for parallel hash join (recursive parallel execution)
		if (CUtils::FParallelHashJoin(popChild))
		{
			CPhysicalParallelHashJoin *popJoin =
				CPhysicalParallelHashJoin::PopConvert(popChild);
			// For nested parallel joins, use probe workers
			return popJoin->UlProbeWorkers();
		}

		pgexprChild = gpChild.PgexprNext(pgexprChild);
	}

	//GPOS_ASSERT(!"No parallel operators found in child group - parallel hash join transformation should not have been applied");
	return 0;  // Unreachable, but satisfies compiler
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::ExtractWorkersIfNeeded
//
//	@doc:
//		Extract probe/build workers from child distributions (lazy initialization)
//		Called by PdsDerive() on first invocation
//
//		Strategy:
//		1. Try to extract from derived distribution (EdtWorkerRandom)
//		2. If distribution is not WorkerRandom (e.g., Motion with EdtHashed),
//		   scan the child group to find parallel operators and extract their
//		   worker counts - this "penetrates" through Motion nodes
//
//---------------------------------------------------------------------------
void
CPhysicalParallelHashJoin::ExtractWorkersIfNeeded(
	CExpressionHandle &exprhdl) const
{
	if (m_fWorkersExtracted)
	{
		// Already extracted
		return;
	}

	// Extract workers by scanning child groups for parallel operators
	// This approach works in all contexts (including lower bound calculation)
	// because it only relies on CGroupExpression, not on Pdpplan() which
	// requires child optimization contexts to be complete
	GPOS_ASSERT(nullptr != exprhdl.Pgexpr() &&
		"ExtractWorkersIfNeeded requires group expression to access child groups");

	CGroup *pgroupOuter = exprhdl.Pgexpr()->Pdrgpgroup()->operator[](0);
	m_ulProbeWorkers = UlExtractWorkersFromGroup(pgroupOuter);

	CGroup *pgroupInner = exprhdl.Pgexpr()->Pdrgpgroup()->operator[](1);
	m_ulBuildWorkers = UlExtractWorkersFromGroup(pgroupInner);

	m_fWorkersExtracted = true;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::Ped
//
//	@doc:
//		Compute enforced distribution property for the n-th child.
//		Overridden to preserve WorkerRandom distributions and avoid unnecessary
//		redistribute motion nodes that would collapse worker-level parallelism.
//
//		Key Strategy:
//		- When both children deliver WorkerRandom with compatible segment-level
//		  hashed distributions, we request "any" distribution (EdtAny) which
//		  allows the existing WorkerRandom distributions to pass through
//		- This prevents ORCA from inserting redistribute Motion nodes that
//		  would collapse workers from 6→3
//
//---------------------------------------------------------------------------
CEnfdDistribution *
CPhysicalParallelHashJoin::Ped(CMemoryPool *mp, CExpressionHandle &exprhdl,
							   CReqdPropPlan *prppInput, ULONG child_index,
							   CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq)
{
	GPOS_ASSERT(2 > child_index);
	GPOS_ASSERT(ulOptReq < UlDistrRequests());

	// Strategy: Check if we can avoid Motion by leveraging existing distributions
	//
	// Case 1: Optimizing the SECOND child (first child already optimized)
	//         Check if first child's WorkerRandom distribution can match join requirements
	//
	// Case 2: Optimizing the FIRST child
	//         Request "Any" distribution to allow WorkerRandom from child scans
	//         The second child will then be required to match

	if (FFirstChildToOptimize(child_index))
	{
		// Only relax distribution for (redistribute,redistribute) requests to allow WorkerRandom to flow.
		ULONG ulHashDistReqs = NumDistrReq();
		if (ulOptReq < ulHashDistReqs)
		{
			CEnfdDistribution::EDistributionMatching dmatch =
				Edm(prppInput, child_index, pdrgpdpCtxt, ulOptReq);
			return GPOS_NEW(mp) CEnfdDistribution(
				GPOS_NEW(mp) CDistributionSpecAny(this->Eopid()), dmatch);
		}
		// For replicate/singleton requests, defer to base class to preserve contracts.
		return CPhysicalHashJoin::Ped(mp, exprhdl, prppInput, child_index,
								  pdrgpdpCtxt, ulOptReq);
	}

	// Optimizing the second child: Only special-case when first child is WorkerRandom; otherwise defer to base.
	if (nullptr != pdrgpdpCtxt && pdrgpdpCtxt->Size() > 0)
	{
		CDistributionSpec *pdsFirst = CDrvdPropPlan::Pdpplan((*pdrgpdpCtxt)[0])->Pds();
		if (CDistributionSpec::EdtWorkerRandom == pdsFirst->Edt())
		{
			CDistributionSpecWorkerRandom *pdsWorkerFirst = CDistributionSpecWorkerRandom::PdsConvert(pdsFirst);
			CDistributionSpec *pdsSegmentBase = pdsWorkerFirst->PdsSegmentBase();
			if (nullptr != pdsSegmentBase && CDistributionSpec::EdtHashed == pdsSegmentBase->Edt())
			{
				CDistributionSpecHashed *pdsHashedBase = CDistributionSpecHashed::PdsConvert(pdsSegmentBase);
				// Ensure first child's base hashed is covered by its own join keys; otherwise, defer to base logic
				const CExpressionArray *pdrgpexprFirstKeys = (EceoRightToLeft == Eceo()) ? PdrgpexprInnerKeys() : PdrgpexprOuterKeys();
				if (!pdsHashedBase->IsCoveredBy(pdrgpexprFirstKeys))
				{
					// Fall back to requesting hashed on the second child based on its own join keys
					CDistributionSpecHashed *pdsHashedReq = PdshashedRequired(mp, child_index, ulOptReq);
					pdsHashedReq->ComputeEquivHashExprs(mp, exprhdl);
					CEnfdDistribution::EDistributionMatching dmatch = Edm(prppInput, child_index, pdrgpdpCtxt, ulOptReq);
					return GPOS_NEW(mp) CEnfdDistribution(pdsHashedReq, dmatch);
				}
				ULONG ulFirstChild = (EceoRightToLeft == Eceo()) ? 1 : 0;
				CDistributionSpecHashed *pdsMatch = PdshashedMatching(mp, pdsHashedBase, ulFirstChild);
				pdsMatch->ComputeEquivHashExprs(mp, exprhdl);
				CEnfdDistribution::EDistributionMatching dmatch = Edm(prppInput, child_index, pdrgpdpCtxt, ulOptReq);
				// Require WorkerRandom on second child, with base hashed matching first child's base
				ULONG ulWorkers = pdsWorkerFirst->UlWorkers();
				CDistributionSpecWorkerRandom *pdsWorkerReq = CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(mp, ulWorkers, pdsMatch);
				return GPOS_NEW(mp) CEnfdDistribution(pdsWorkerReq, dmatch);
			}
		}
	}

	// For remaining cases (second child with non-WorkerRandom first child,
	// or replicate/singleton requests), use standard hash join logic
	//
	// However, we cannot blindly call base class because if we returned EdtAny
	// for the first child, base class's PdsRequiredSingleton() will assert.
	//
	// Solution: Only call base class if first child is singleton/replicated,
	// otherwise handle redistribute ourselves

	if (!FFirstChildToOptimize(child_index) && nullptr != pdrgpdpCtxt &&
		pdrgpdpCtxt->Size() > 0)
	{
		CDistributionSpec *pdsFirst =
			CDrvdPropPlan::Pdpplan((*pdrgpdpCtxt)[0])->Pds();

		// Check if first child is singleton or replicated
		// In these cases, it's safe to call base class
		if (CDistributionSpec::EdtSingleton == pdsFirst->Edt() ||
			CDistributionSpec::EdtStrictSingleton == pdsFirst->Edt() ||
			CDistributionSpec::EdtReplicated == pdsFirst->Edt() ||
			CDistributionSpec::EdtStrictReplicated == pdsFirst->Edt())
		{
			return CPhysicalHashJoin::Ped(mp, exprhdl, prppInput, child_index,
										  pdrgpdpCtxt, ulOptReq);
		}
	}

	// For all other cases, use default behavior: request any distribution
	// This allows the optimizer to choose the best option
	CEnfdDistribution::EDistributionMatching dmatch =
		Edm(prppInput, child_index, pdrgpdpCtxt, ulOptReq);

	return GPOS_NEW(mp) CEnfdDistribution(
		GPOS_NEW(mp) CDistributionSpecAny(this->Eopid()),
		dmatch);
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelHashJoin::PdsRequired(CMemoryPool *mp GPOS_UNUSED,
									   CExpressionHandle &exprhdl GPOS_UNUSED,
									   CDistributionSpec *pdsRequired GPOS_UNUSED,
									   ULONG child_index GPOS_UNUSED,
									   CDrvdPropArray *pdrgpdpCtxt GPOS_UNUSED,
									   ULONG ulOptReq GPOS_UNUSED) const
{
	GPOS_RAISE(
		CException::ExmaInvalid, CException::ExmiInvalid,
		GPOS_WSZ_LIT("PdsRequired should not be called for CPhysicalParallelHashJoin"));
	return nullptr;
#if 0
	GPOS_ASSERT(child_index < 2);

	// Inner child (child_index == 1)
	if (1 == child_index)
	{
		// Get the distribution delivered by the outer child
		CDistributionSpec *pdsOuter =
			CDrvdPropPlan::Pdpplan((*pdrgpdpCtxt)[0])->Pds();

		// Key constraint: Inner must use PdsSegmentBase, NOT WorkerRandom
		// Reason: All workers must see the complete segment data to build
		//         a shared hash table. If inner uses WorkerRandom, each worker
		//         would only scan a subset, leading to missing join matches.
		if (CDistributionSpec::EdtWorkerRandom == pdsOuter->Edt())
		{
			CDistributionSpecWorkerRandom *pdsWorkerOuter =
				CDistributionSpecWorkerRandom::PdsConvert(pdsOuter);

			// Extract the segment-level base distribution
			CDistributionSpec *pdsSegmentBase =
				pdsWorkerOuter->PdsSegmentBase();

			if (nullptr != pdsSegmentBase &&
				CDistributionSpec::EdtHashed == pdsSegmentBase->Edt())
			{
				CDistributionSpecHashed *pdsHashedBase =
					CDistributionSpecHashed::PdsConvert(pdsSegmentBase);

				// Check if the base distribution covers inner join keys
				if (pdsHashedBase->IsCoveredBy(PdrgpexprInnerKeys()))
				{
					// Return the segment-level hashed distribution
					pdsSegmentBase->AddRef();
					return pdsSegmentBase;
				}
			}
		}

		// Otherwise, require hashed distribution on inner join keys
		return PdshashedRequired(mp, child_index, ulOptReq);
	}

	// Outer child (child_index == 0)
	// For parallel hash join, we want the outer child to deliver WorkerRandom
	// distribution to enable parallel probe phase
	if (0 == child_index)
	{
		// If the required distribution from parent is already satisfied by
		// WorkerRandom, we can propagate a WorkerRandom requirement
		// Otherwise, we need to create a WorkerRandom requirement based on
		// the outer join keys

		// Create a hashed distribution on outer join keys as the segment base
		CDistributionSpecHashed *pdsHashedOuter =
			PdshashedRequired(mp, child_index, ulOptReq);

		// Wrap it in WorkerRandom distribution
		CDistributionSpecWorkerRandom *pdsWorkerRandom =
			GPOS_NEW(mp) CDistributionSpecWorkerRandom(UlProbeWorkers(),
													   pdsHashedOuter);

		return pdsWorkerRandom;
	}

	// Fallback: delegate to base class
	return CPhysicalHashJoin::PdsRequired(mp, exprhdl, pdsRequired,
										  child_index, pdrgpdpCtxt, ulOptReq);
#endif
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::PrsRequired
//
//	@doc:
//		Compute required rewindability of the n-th child
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalParallelHashJoin::PrsRequired(CMemoryPool *mp,
									   CExpressionHandle &exprhdl,
									   CRewindabilitySpec *prsRequired,
									   ULONG child_index,
									   CDrvdPropArray *pdrgpdpCtxt,
									   ULONG ulOptReq) const
{
	GPOS_ASSERT(child_index < 2);

	// Parallel hash join requires same rewindability as traditional hash join
	// Inner child is materialized by Hash node, outer child passes through
	return CPhysicalHashJoin::PrsRequired(mp, exprhdl, prsRequired,
										  child_index, pdrgpdpCtxt, ulOptReq);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::PdsDerive
//
//	@doc:
//		Derive distribution for parallel hash join
//
//		Strategy (two-tier priority):
//		1. Handle WorkerRandom distributions (parallel-specific)
//		2. Fall back to traditional hash join logic (from CPhysicalHashJoin)
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelHashJoin::PdsDerive(CMemoryPool *,  // mp
									 CExpressionHandle &exprhdl) const
{
	// Extract workers from child distributions on first call
	ExtractWorkersIfNeeded(exprhdl);

	// Get distributions from children
	CDistributionSpec *pdsOuter = exprhdl.Pdpplan(0)->Pds();
	CDistributionSpec *pdsInner = exprhdl.Pdpplan(1)->Pds();

	// ========== Priority 1: Handle WorkerRandom Distributions ==========

	// Case 1: Both children are WorkerRandom (optimal parallel scenario)
	if (CDistributionSpec::EdtWorkerRandom == pdsOuter->Edt() &&
		CDistributionSpec::EdtWorkerRandom == pdsInner->Edt())
	{
		CDistributionSpecWorkerRandom *pdsWorkerOuter =
			CDistributionSpecWorkerRandom::PdsConvert(pdsOuter);
		CDistributionSpecWorkerRandom *pdsWorkerInner =
			CDistributionSpecWorkerRandom::PdsConvert(pdsInner);

		// Verify worker count matches
		if (pdsWorkerOuter->UlWorkers() == pdsWorkerInner->UlWorkers())
		{
			// Verify segment-level base distributions are compatible
			CDistributionSpec *pdsBaseOuter = pdsWorkerOuter->PdsSegmentBase();
			CDistributionSpec *pdsBaseInner = pdsWorkerInner->PdsSegmentBase();

			if (nullptr != pdsBaseOuter && nullptr != pdsBaseInner)
			{
				// If both bases are hashed, check if they cover join keys
				if (CDistributionSpec::EdtHashed == pdsBaseOuter->Edt() &&
					CDistributionSpec::EdtHashed == pdsBaseInner->Edt())
				{
					CDistributionSpecHashed *pdsHashedOuter =
						CDistributionSpecHashed::PdsConvert(pdsBaseOuter);
					CDistributionSpecHashed *pdsHashedInner =
						CDistributionSpecHashed::PdsConvert(pdsBaseInner);

					// Check segment-level co-location
					if (pdsHashedOuter->IsCoveredBy(PdrgpexprOuterKeys()) &&
						pdsHashedInner->IsCoveredBy(PdrgpexprInnerKeys()))
					{
						// Perfect match! Return outer's WorkerRandom distribution
						pdsOuter->AddRef();
						return pdsOuter;
					}
				}
				// If both bases are Random, they're compatible
				else if (CDistributionSpec::EdtRandom == pdsBaseOuter->Edt() &&
						 CDistributionSpec::EdtRandom == pdsBaseInner->Edt())
				{
					pdsOuter->AddRef();
					return pdsOuter;
				}
			}
		}

		// Worker count mismatch or base incompatible - fall back to outer
		// Enforcement will add Motion if needed
		pdsOuter->AddRef();
		return pdsOuter;
	}

	// Case 2: Only outer is WorkerRandom
	if (CDistributionSpec::EdtWorkerRandom == pdsOuter->Edt())
	{
		// Return outer's distribution
		// Inner will be redistributed to match (handled by PdsRequired)
		pdsOuter->AddRef();
		return pdsOuter;
	}

	// ========== Priority 2: Traditional Distributions ==========
	// For non-WorkerRandom distributions, pass through outer distribution
	// The enforcement mechanism will handle any required redistributions

	// Note: CPhysicalHashJoin::PdsDeriveFromHashedChildren() and other
	// helper methods are private, so we cannot reuse them directly here.
	// Instead, we rely on the enforcement mechanism to handle traditional
	// distributions through Motion nodes if needed.

	// Default: Pass through outer distribution
	pdsOuter->AddRef();
	return pdsOuter;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::PrsDerive
//
//	@doc:
//		Derive rewindability
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalParallelHashJoin::PrsDerive(CMemoryPool *mp,
									 CExpressionHandle &exprhdl
) const
{
	return CPhysicalJoin::PrsDerive(mp, exprhdl);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::EpetDistribution
//
//	@doc:
//		Return the enforcing type for distribution property based on this
//		operator
//
//		Key scenarios handled:
//		1. Direct match: Required WorkerRandom[N] == Derived WorkerRandom[N]
//		2. Segment base match: Required Hashed(a) matches our WorkerRandom's
//		   segment base Hashed(a) - critical for being used as inner child
//		   of another hash join (see doc section 8.2)
//		3. Mismatch: Motion required to enforce required distribution
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalParallelHashJoin::EpetDistribution(CExpressionHandle &exprhdl,
											 const CEnfdDistribution *ped) const
{
	GPOS_ASSERT(nullptr != ped);

	// Note: We do NOT reject plans where inner child is not WorkerRandom here.
	// The base distribution matching logic (lines 570-583) can handle cases where
	// inner child has regular HASHED distribution but our derived WorkerRandom's
	// base distribution satisfies the parent's HASHED requirement.
	// Rejecting too early would prevent valid plans from being considered.

	// Get our derived distribution (returned by PdsDerive)
	CDistributionSpec *pdsDerived = CDrvdPropPlan::Pdpplan(exprhdl.Pdp())->Pds();

	// Case 1: Our derived distribution directly satisfies the requirement
	// Examples:
	//   - Required: WorkerRandom[2] base:Hashed(a)
	//     Derived:  WorkerRandom[2] base:Hashed(a) → Match!
	if (ped->FCompatible(pdsDerived))
	{
		return CEnfdProp::EpetUnnecessary;
	}
#if 0
	// Case 2: Required is traditional (Hashed), but we deliver WorkerRandom
	// Check if our segment-level base distribution can satisfy the requirement
	//
	// Critical scenario: We're used as inner child of another hash join
	// Example:
	//   - Required: Hashed(b) (from outer hash join's inner requirement)
	//   - Derived:  WorkerRandom[2] base:Hashed(b)
	//   → Segment base Hashed(b) satisfies Hashed(b) requirement!
	//   → No Motion needed (workers will share the hash table)
	//
	if (CDistributionSpec::EdtWorkerRandom != pdsRequired->Edt() &&
		CDistributionSpec::EdtWorkerRandom == pdsDerived->Edt())
	{
		CDistributionSpecWorkerRandom *pdsWorkerDerived =
			CDistributionSpecWorkerRandom::PdsConvert(pdsDerived);

		CDistributionSpec *pdsSegmentBase = pdsWorkerDerived->PdsSegmentBase();
		if (nullptr != pdsSegmentBase && ped->FCompatible(pdsSegmentBase))
		{
			// Segment-level base distribution satisfies the requirement
			// This avoids unnecessary Motion for worker-level parallelism
			return CEnfdProp::EpetRequired; //FIXME
		}
	}
#endif
	// Case 3: No distribution satisfies the requirement
	// Motion enforcement will be needed on the output
	// Examples:
	//   - Required: WorkerRandom[4], Derived: WorkerRandom[2] (worker mismatch)
	//   - Required: Hashed(c), Derived: WorkerRandom[2] base:Hashed(a) (key mismatch)
	return CEnfdProp::EpetRequired;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::FValidContext
//
//	@doc:
//		Check if optimization context is valid;
//		Reject if parent requires REWINDABLE (e.g., for NL Join inner child)
//		because ParallelHashJoin derives NONE (not rewindable)
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelHashJoin::FValidContext(
	CMemoryPool *,  // mp
	COptimizationContext *poc,
	COptimizationContextArray *pdrgpocChild) const
{
	GPOS_ASSERT(nullptr != poc);

	// Check if parent requires rewindability
	CReqdPropPlan *prpp = poc->Prpp();
	CRewindabilitySpec *prsRequired = prpp->Per()->PrsRequired();

	// Parallel hash join is non-rewindable (derives ErtNone)
	// If parent requires REWINDABLE (e.g., NL join inner child), reject early.
	if (prsRequired->IsOriginNLJoin())
	{
		// Parent requires rewindability but ParallelHashJoin cannot provide it.
		// Reject this plan so the optimizer can choose alternatives or add Spool.
		return false;
	}

	if (m_fWorkersExtracted)
	{
		if (0 == m_ulProbeWorkers || 0 == m_ulBuildWorkers)
		{
			// Invalid worker counts extracted previously
			return false;
		}
	}

	// Reject if direct child is a Nested Loop Join (non-recursive check)
	// NL Join is inherently sequential and cannot execute in parallel
	if (nullptr != pdrgpocChild && pdrgpocChild->Size() >= 2)
	{
		for (ULONG ulChild = 0; ulChild < 2; ulChild++)
		{
			COptimizationContext *pocChild = (*pdrgpocChild)[ulChild];
			if (nullptr != pocChild)
			{
				CGroupExpression *pgexprChild = pocChild->PgexprBest();
				if (nullptr != pgexprChild)
				{
					COperator *popChild = pgexprChild->Pop();
					// Check if the best child expression is an NL Join
					if (CUtils::FNLJoin(popChild))
					{
						// Direct child is NLJoin - incompatible with parallel execution
						return false;
					}
				}
			}
		}
	}

	// Lightweight pruning based on children required distributions (when available).
	// Do NOT attempt to inspect derived child properties here. We only reject
	// obviously incompatible contexts to reduce search:
	//   - Disallow singleton/replicated requirements for either child (parallel hash
	//     join expects segment-distributed inputs).
	//   - The outer/probe side should allow parallelism: its required distribution
	//     must be Either Any or WorkerRandom. If it is explicitly constrained to a
	//     non-parallel distribution, prune this context.
	if (nullptr != pdrgpocChild && pdrgpocChild->Size() >= 2)
	{
		CEnfdDistribution *pedOuter = (*pdrgpocChild)[0]->Prpp()->Ped();
		CEnfdDistribution *pedInner = (*pdrgpocChild)[1]->Prpp()->Ped();
		CDistributionSpec *pdsOuterReq = pedOuter->PdsRequired();
		CDistributionSpec *pdsInnerReq = pedInner->PdsRequired();

		const CDistributionSpec::EDistributionType dtOuter = pdsOuterReq->Edt();
		const CDistributionSpec::EDistributionType dtInner = pdsInnerReq->Edt();

		// Reject singleton or replicated requirements on either child
		if (dtOuter == CDistributionSpec::EdtSingleton ||
			dtOuter == CDistributionSpec::EdtStrictSingleton ||
			dtOuter == CDistributionSpec::EdtReplicated ||
			dtOuter == CDistributionSpec::EdtStrictReplicated ||
			dtInner == CDistributionSpec::EdtSingleton ||
			dtInner == CDistributionSpec::EdtStrictSingleton ||
			dtInner == CDistributionSpec::EdtReplicated ||
			dtInner == CDistributionSpec::EdtStrictReplicated)
		{
			return false;
		}

		// Outer/probe side must allow parallelism: require Any or WorkerRandom.
		if (dtOuter != CDistributionSpec::EdtAny &&
			dtOuter != CDistributionSpec::EdtWorkerRandom)
		{
			return false;
		}

		// Inner/build side validation:
		// - We allow Any, WorkerRandom, or Hashed distributions
		// - Hashed is allowed because parallel hash join can work when:
		//   1. Both children have compatible segment-level hashed distributions
		//   2. Workers share the hash table built from segment-local data
		// - We only reject obviously incompatible distributions
		//
		// Note: We do NOT require inner to be WorkerRandom. The EpetDistribution()
		// logic (lines 541-593) handles cases where inner has HASHED distribution
		// and our derived WorkerRandom's base distribution satisfies parent requirements.
		if (dtInner != CDistributionSpec::EdtAny &&
			dtInner != CDistributionSpec::EdtWorkerRandom &&
			dtInner != CDistributionSpec::EdtHashed)
		{
			// Inner child requires a distribution incompatible with parallel hash join
			// (e.g., coordinator-only distribution, universal distribution, etc.)
			return false;
		}
	}

	return true;
}

// EOF
