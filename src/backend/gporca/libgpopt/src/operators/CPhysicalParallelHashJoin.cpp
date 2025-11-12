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

// GUC variable from PostgreSQL
extern int max_parallel_workers_per_gather;

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
	  m_ulBuildWorkers(0)
{
	// m_ulProbeWorkers and m_ulBuildWorkers will be extracted in FValidContext()
	// from child groups when optimization contexts are available
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
//		CPhysicalParallelHashJoin::UlExtractWorkersFromGroupInternal
//
//	@doc:
//		Internal recursive function to extract worker count from a child group
//		Uses visited set to avoid infinite loops from circular references
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelHashJoin::UlExtractWorkersFromGroupInternal(
	CGroup *pgroup, CBitSet *visited_groups) const
{
	if (nullptr == pgroup || visited_groups->Get(pgroup->Id()))
	{
		return 0;  // Already visited or null - avoid infinite recursion
	}

	// Mark this group as visited
	visited_groups->ExchangeSet(pgroup->Id());

	// Scan through all physical expressions in the child group
	CGroupProxy gpChild(pgroup);
	CGroupExpression *pgexprChild = gpChild.PgexprFirst();

	while (nullptr != pgexprChild)
	{
		COperator *popChild = pgexprChild->Pop();

		// Check for parallel table scan - this is the source of worker count
		if (COperator::EopPhysicalParallelTableScan == popChild->Eopid())
		{
			CPhysicalParallelTableScan *popScan =
				CPhysicalParallelTableScan::PopConvert(popChild);
			return popScan->UlParallelWorkers();
		}

		// Recursively check all children (Motion nodes, joins, etc.)
		for (ULONG ul = 0; ul < pgexprChild->Arity(); ul++)
		{
			ULONG ulChildWorkers = UlExtractWorkersFromGroupInternal(
				(*pgexprChild)[ul], visited_groups);
			if (ulChildWorkers > 0)
			{
				return ulChildWorkers;
			}
		}

		pgexprChild = gpChild.PgexprNext(pgexprChild);
	}

	return 0;  // No parallel operators found
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::UlExtractWorkersFromGroup
//
//	@doc:
//		Extract worker count from a child group by scanning for parallel operators
//		This handles cases where Motion nodes hide parallel scans
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelHashJoin::UlExtractWorkersFromGroup(CGroup *pgroup) const
{
	if (nullptr == pgroup)
	{
		return 0;
	}

	// Create visited set to track groups and avoid infinite recursion
	CMemoryPool *mp = COptCtxt::PoctxtFromTLS()->Pmp();
	CBitSet *visited = GPOS_NEW(mp) CBitSet(mp);
	ULONG ulWorkers = UlExtractWorkersFromGroupInternal(pgroup, visited);
	visited->Release();

	return ulWorkers;
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::UlExtractRequestedWorkers
//
//	@doc:
//		Extract requested worker count for distribution requirement.
//		Priority:
//		1. Child group's parallel operators (table scans, nested joins)
//		2. GUC max_parallel_workers_per_gather
//		3. Default fallback (2 workers)
//
//		This is used when requesting WorkerRandom distribution for the first child,
//		before it has been optimized.
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelHashJoin::UlExtractRequestedWorkers(
	CExpressionHandle &exprhdl, ULONG child_index) const
{
	ULONG ulWorkers = 0;
	// Try to extract from child group if available
	CGroupExpression *pgexpr = exprhdl.Pgexpr();
	if (nullptr != pgexpr && child_index < pgexpr->Arity())
	{
		CGroup *pgroupChild = (*pgexpr)[child_index];
		if (nullptr != pgroupChild)
		{
			ulWorkers = UlExtractWorkersFromGroup(pgroupChild);
			if (ulWorkers > 0)
			{
				return ulWorkers;
			}
		}
	}

	//GPOS_ASSERT(ulWorkers > 0);
	// Fall back to GUC setting if no parallel operators found in child
	if (max_parallel_workers_per_gather > 0)
	{
		return (ULONG)max_parallel_workers_per_gather;
	}

	// Default fallback
	return 2;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::ExtractWorkersIfNeeded
//
//	@doc:
//		Extract probe/build workers from child distributions
//		This is a legacy function kept for compatibility but no longer used
//		Worker extraction now happens in FValidContext()
//
//---------------------------------------------------------------------------
void
CPhysicalParallelHashJoin::ExtractWorkersIfNeeded(
	CExpressionHandle &exprhdl) const
{
	// Extract workers by scanning child groups for parallel operators
	GPOS_ASSERT(nullptr != exprhdl.Pgexpr() &&
		"ExtractWorkersIfNeeded requires group expression to access child groups");

	CGroup *pgroupOuter = exprhdl.Pgexpr()->Pdrgpgroup()->operator[](0);
	m_ulProbeWorkers = UlExtractWorkersFromGroup(pgroupOuter);

	CGroup *pgroupInner = exprhdl.Pgexpr()->Pdrgpgroup()->operator[](1);
	m_ulBuildWorkers = UlExtractWorkersFromGroup(pgroupInner);
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

	CEnfdDistribution::EDistributionMatching dmatch =
		Edm(prppInput, child_index, pdrgpdpCtxt, ulOptReq);
	CDistributionSpec *const pdsInput = prppInput->Ped()->PdsRequired();

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
		// For redistribute requests, require first child to be hashed on join keys
		ULONG ulHashDistReqs = NumDistrReq();
		if (ulOptReq < ulHashDistReqs)
		{
			CDistributionSpec *pds = PdsRequiredRedistribute(
				mp, exprhdl, pdsInput, child_index, pdrgpdpCtxt, ulOptReq);
			// Compute equivalent hash expressions if needed (for hashed distributions)
			if (CDistributionSpec::EdtHashed == pds->Edt())
			{
				CDistributionSpecHashed::PdsConvert(pds)->ComputeEquivHashExprs(mp, exprhdl);
			}
			// Extract worker count dynamically:
			// Priority: 1) child group's parallel operators (table scans with parallel_workers setting)
			//           2) GUC max_parallel_workers_per_gather
			//           3) Default fallback (2)
			ULONG ulWorkers = UlExtractRequestedWorkers(exprhdl, child_index);
			return GPOS_NEW(mp) CEnfdDistribution(
				CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(mp, ulWorkers, pds), dmatch);
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
			ULONG ulWorkers = pdsWorkerFirst->UlWorkers();
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
					return GPOS_NEW(mp) CEnfdDistribution(pdsHashedReq, dmatch);
				}
				ULONG ulFirstChild = (EceoRightToLeft == Eceo()) ? 1 : 0;
				CDistributionSpecHashed *pdsMatch = PdshashedMatching(mp, pdsHashedBase, ulFirstChild);
				pdsMatch->ComputeEquivHashExprs(mp, exprhdl);
				// Require WorkerRandom on second child, with base hashed matching first child's base
				return GPOS_NEW(mp) CEnfdDistribution(
					CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(mp, ulWorkers, pdsMatch), dmatch);
			}
			// Handle random distribution case
			else if (nullptr != pdsSegmentBase &&
					 (CDistributionSpec::EdtRandom == pdsSegmentBase->Edt() ||
					  CDistributionSpec::EdtStrictRandom == pdsSegmentBase->Edt()))
			{
				// For random distributed tables, we need to redistribute by join keys
				// Request WorkerRandom on the second child with hashed base matching join keys
				CDistributionSpecHashed *pdsHashedReq = PdshashedRequired(mp, child_index, ulOptReq);
				pdsHashedReq->ComputeEquivHashExprs(mp, exprhdl);
				return GPOS_NEW(mp) CEnfdDistribution(
					CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(mp, ulWorkers, pdsHashedReq), dmatch);
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
	// Note: Worker extraction has been moved to FValidContext()
	// where we have guaranteed access to child groups via optimization contexts.
	// This avoids the issue where exprhdl.Pgexpr() might be null in some contexts.

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
		//if (pdsWorkerOuter->UlWorkers() == pdsWorkerInner->UlWorkers())
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

	// Case 2: Only outer is WorkerRandom - check if base matches join keys
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
	// This handles singleton, replicated, and other compatible cases
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

	// Case 2: No distribution satisfies the requirement
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

	// Extract workers from child groups if not already extracted
	// This is the correct place to extract workers because:
	// 1. FValidContext is called during optimization context validation
	// 2. We have access to child optimization contexts (pdrgpocChild)
	// 3. We can directly access child groups via pocChild->Pgroup()
	// 4. If extraction fails, we can reject this optimization context early
	if (0 == m_ulProbeWorkers || 0 == m_ulBuildWorkers)
	{
		if (nullptr != pdrgpocChild && pdrgpocChild->Size() >= 2)
		{
			COptimizationContext *pocOuter = (*pdrgpocChild)[0];
			COptimizationContext *pocInner = (*pdrgpocChild)[1];

			if (nullptr != pocOuter && nullptr != pocInner)
			{
				CGroup *pgroupOuter = pocOuter->Pgroup();
				CGroup *pgroupInner = pocInner->Pgroup();

				if (nullptr != pgroupOuter && nullptr != pgroupInner)
				{
					m_ulProbeWorkers = UlExtractWorkersFromGroup(pgroupOuter);
					m_ulBuildWorkers = UlExtractWorkersFromGroup(pgroupInner);
				}
			}
		}

		// Validate extracted worker counts
		if (0 == m_ulProbeWorkers || 0 == m_ulBuildWorkers)
		{
			// Invalid worker counts extracted - reject this optimization context
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
