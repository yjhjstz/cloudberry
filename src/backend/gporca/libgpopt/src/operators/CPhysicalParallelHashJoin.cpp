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

// Use gpdbwrappers for parallel workers setting
extern int max_parallel_workers_per_gather;

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
	  m_ulParallelWorkers(2)  // default value
{
	// Read parallel workers degree from GUC setting
	// Priority: GUC max_parallel_workers_per_gather > default (2)
	if (max_parallel_workers_per_gather > 0)
	{
		m_ulParallelWorkers = (ULONG)max_parallel_workers_per_gather; // FIXME:
	}

	GPOS_ASSERT(m_ulParallelWorkers > 0);
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
//		CPhysicalParallelHashJoin::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelHashJoin::PdsRequired(CMemoryPool *mp,
									   CExpressionHandle &exprhdl,
									   CDistributionSpec *pdsRequired,
									   ULONG child_index,
									   CDrvdPropArray *pdrgpdpCtxt,
									   ULONG ulOptReq) const
{
	GPOS_ASSERT(child_index < 2);

	// Inner child (child_index == 1)
	if (1 == child_index)
	{
		// Get the distribution delivered by the outer child
		CDistributionSpec *pdsOuter =
			CDrvdPropPlan::Pdpplan((*pdrgpdpCtxt)[0])->Pds();

		// 🚨 Key constraint: Inner must use PdsSegmentBase, NOT WorkerRandom
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
	// Delegate to base class implementation
	return CPhysicalHashJoin::PdsRequired(mp, exprhdl, pdsRequired,
										  child_index, pdrgpdpCtxt, ulOptReq);
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
//		Derive distribution
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelHashJoin::PdsDerive(CMemoryPool *,  // mp
									 CExpressionHandle &exprhdl) const
{
	// Get distributions from children
	CDistributionSpec *pdsOuter = exprhdl.Pdpplan(0)->Pds();
	CDistributionSpec *pdsInner = exprhdl.Pdpplan(1)->Pds();

	// Defensive programming: Check for replicated distributions
	// (Should be filtered by Xform, but check here as well)
	if (CDistributionSpec::EdtReplicated == pdsOuter->Edt() ||
		CDistributionSpec::EdtStrictReplicated == pdsOuter->Edt() ||
		CDistributionSpec::EdtReplicated == pdsInner->Edt() ||
		CDistributionSpec::EdtStrictReplicated == pdsInner->Edt())
	{
		GPOS_RAISE(CException::ExmaInvalid, CException::ExmiInvalid,
				   GPOS_WSZ_LIT("Parallel Hash Join does not support "
								"replicated distributions"));
	}

	// Case 1: Both children are WorkerRandom
	if (CDistributionSpec::EdtWorkerRandom == pdsOuter->Edt() &&
		CDistributionSpec::EdtWorkerRandom == pdsInner->Edt())
	{
		CDistributionSpecWorkerRandom *pdsWorkerOuter =
			CDistributionSpecWorkerRandom::PdsConvert(pdsOuter);
		CDistributionSpecWorkerRandom *pdsWorkerInner =
			CDistributionSpecWorkerRandom::PdsConvert(pdsInner);

		// Verify worker count matches
		if (pdsWorkerOuter->UlWorkers() != pdsWorkerInner->UlWorkers())
		{
			// Worker count mismatch - fall back to outer distribution
			// This allows enforcement to add Motion if needed
			pdsOuter->AddRef();
			return pdsOuter;
		}

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
					// Compatible! Return outer's WorkerRandom distribution
					pdsOuter->AddRef();
					return pdsOuter;
				}
			}
		}

		// Base distributions not compatible - fall back to outer distribution
		// This allows enforcement to add Motion if needed
		pdsOuter->AddRef();
		return pdsOuter;
	}

	// Case 2: Outer is WorkerRandom, Inner is not
	if (CDistributionSpec::EdtWorkerRandom == pdsOuter->Edt())
	{
		// Return outer's distribution (inner will be matched)
		pdsOuter->AddRef();
		return pdsOuter;
	}

	// Case 3: Default behavior - pass through outer distribution
	// (This handles cases where outer is Hashed, Random, etc.)
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
									 CExpressionHandle &  // exprhdl
) const
{
	// Parallel hash join is NOT rewindable
	// Reason: Parallel execution with shared hash table cannot support
	//         rewinding to arbitrary positions
	return GPOS_NEW(mp) CRewindabilitySpec(CRewindabilitySpec::ErtNone,
										   CRewindabilitySpec::EmhtNoMotion);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::EpetDistribution
//
//	@doc:
//		Return the enforcing type for distribution property based on this
//		operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalParallelHashJoin::EpetDistribution(CExpressionHandle &exprhdl,
											 const CEnfdDistribution *ped) const
{
	GPOS_ASSERT(nullptr != ped);

	// Get our derived distribution (returned by PdsDerive)
	CDistributionSpec *pdsDerived = CDrvdPropPlan::Pdpplan(exprhdl.Pdp())->Pds();

	// Get the required distribution
	CDistributionSpec *pdsRequired = ped->PdsRequired();

	// Case 1: Our derived distribution directly satisfies the requirement
	if (ped->FCompatible(pdsDerived))
	{
		return CEnfdProp::EpetUnnecessary;
	}

	// Case 2: If required is NOT WorkerRandom, but our derived IS WorkerRandom
	// Check if the segment-level base distribution can satisfy the requirement
	// (This handles the case where we're used as inner child of another HashJoin)
	if (CDistributionSpec::EdtWorkerRandom != pdsRequired->Edt() &&
		CDistributionSpec::EdtWorkerRandom == pdsDerived->Edt())
	{
		CDistributionSpecWorkerRandom *pdsWorkerDerived =
			CDistributionSpecWorkerRandom::PdsConvert(pdsDerived);

		CDistributionSpec *pdsSegmentBase = pdsWorkerDerived->PdsSegmentBase();
		if (nullptr != pdsSegmentBase && ped->FCompatible(pdsSegmentBase))
		{
			// Segment-level base distribution satisfies the requirement
			return CEnfdProp::EpetUnnecessary;
		}
	}

	// No distribution satisfies the requirement
	// Motion enforcement will be needed on the output
	return CEnfdProp::EpetRequired;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashJoin::FValidContext
//
//	@doc:
//		Check if optimization context is valid
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelHashJoin::FValidContext(
	CMemoryPool *mp, COptimizationContext *poc,
	COptimizationContextArray *pdrgpocChild) const
{
	GPOS_ASSERT(nullptr != pdrgpocChild);
	GPOS_ASSERT(2 == pdrgpocChild->Size());

	// Parallel hash join is non-rewindable, so it cannot be used as
	// the inner child of a nested loop join
	COptimizationContext *pocChild = (*pdrgpocChild)[0];
	CCostContext *pccBest = pocChild->PccBest();
	GPOS_ASSERT(nullptr != pccBest);

	// Get required rewindability
	CRewindabilitySpec *prs = CDrvdPropPlan::Pdpplan(pccBest->Pdpplan())->Prs();

	// If rewindability is required, reject this context
	if (CRewindabilitySpec::ErtNone != prs->Ert())
	{
		// Context is invalid - parent requires rewindability
		// but parallel hash join is non-rewindable
		return false;
	}

	// Otherwise, use base class validation
	return CPhysicalHashJoin::FValidContext(mp, poc, pdrgpocChild);
}

// EOF
