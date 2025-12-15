/*-------------------------------------------------------------------------
 *
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
 *
 * CPhysicalParallelPartitionSelector.cpp
 *
 * IDENTIFICATION
 *	  src/backend/gporca/libgpopt/src/operators/CPhysicalParallelPartitionSelector.cpp
 *
 *-------------------------------------------------------------------------
 */

#include "gpopt/operators/CPhysicalParallelPartitionSelector.h"

#include "gpos/base.h"

#include "gpopt/base/CColRef.h"
#include "gpopt/base/CDistributionSpecAny.h"
#include "gpopt/base/CDistributionSpecWorkerRandom.h"
#include "gpopt/base/CDrvdPropCtxtPlan.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CPhysicalParallelTableScan.h"
#include "gpopt/operators/CPhysicalParallelAppendTableScan.h"
#include "gpopt/search/CGroupProxy.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::CPhysicalParallelPartitionSelector
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelPartitionSelector::CPhysicalParallelPartitionSelector(CMemoryPool *mp,
																	   ULONG scan_id,
																	   ULONG selector_id,
																	   IMDId *mdid,
																	   CExpression *pexprScalar,
																	   ULONG ulParallelWorkers)
	: CPhysical(mp),
	  m_scan_id(scan_id),
	  m_selector_id(selector_id),
	  m_mdid(mdid),
	  m_filter_expr(pexprScalar),
	  m_parallel_workers(ulParallelWorkers)
{
	GPOS_ASSERT(0 < scan_id);
	GPOS_ASSERT(mdid->IsValid());
	GPOS_ASSERT(ulParallelWorkers > 0);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::~CPhysicalParallelPartitionSelector
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelPartitionSelector::~CPhysicalParallelPartitionSelector()
{
	m_mdid->Release();
	CRefCount::SafeRelease(m_filter_expr);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::Matches
//
//	@doc:
//		Match operators
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelPartitionSelector::Matches(COperator *pop) const
{
	if (Eopid() != pop->Eopid())
	{
		return false;
	}

	CPhysicalParallelPartitionSelector *popPartSelector =
		CPhysicalParallelPartitionSelector::PopConvert(pop);

	BOOL fScanIdCmp = popPartSelector->ScanId() == m_scan_id;
	BOOL fMdidCmp = popPartSelector->MDId()->Equals(MDId());
	BOOL fParallelWorkersCmp = popPartSelector->UlParallelWorkers() == m_parallel_workers;

	return fScanIdCmp && fMdidCmp && fParallelWorkersCmp;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::HashValue
//
//	@doc:
//		Hash operator
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelPartitionSelector::HashValue() const
{
	return gpos::CombineHashes(
		Eopid(), gpos::CombineHashes(m_scan_id, MDId()->HashValue()));
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PcrsRequired
//
//	@doc:
//		Compute required columns of the n-th child;
//		we only compute required columns for the relational child;
//
//---------------------------------------------------------------------------
CColRefSet *
CPhysicalParallelPartitionSelector::PcrsRequired(CMemoryPool *mp,
										 CExpressionHandle &exprhdl,
										 CColRefSet *pcrsInput,
										 ULONG child_index,
										 CDrvdPropArray *,	// pdrgpdpCtxt
										 ULONG				// ulOptReq
)
{
	GPOS_ASSERT(
		0 == child_index &&
			"Required properties can only be computed on the relational child");

	CColRefSet *pcrs = GPOS_NEW(mp) CColRefSet(mp, *pcrsInput);
	pcrs->Union(m_filter_expr->DeriveUsedColumns());
	pcrs->Intersection(exprhdl.DeriveOutputColumns(child_index));

	return pcrs;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PosRequired
//
//	@doc:
//		Compute required sort order of the n-th child
//
//---------------------------------------------------------------------------
COrderSpec *
CPhysicalParallelPartitionSelector::PosRequired(CMemoryPool *mp,
										CExpressionHandle &exprhdl,
										COrderSpec *posRequired,
										ULONG child_index,
										CDrvdPropArray *,  // pdrgpdpCtxt
										ULONG			   // ulOptReq
) const
{
	GPOS_ASSERT(0 == child_index);

	return PosPassThru(mp, exprhdl, posRequired, child_index);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child跑
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelPartitionSelector::PdsRequired(CMemoryPool *mp,
										CExpressionHandle &exprhdl,
										CDistributionSpec *pdsInput,	// pdsInput
										ULONG child_index,	// child_index
										CDrvdPropArray *,  // pdrgpdpCtxt
										ULONG			   // ulOptReq
) const
{
	CPartInfo *ppartinfo = exprhdl.DerivePartitionInfo();
	BOOL fCovered = ppartinfo->FContainsScanId(m_scan_id);

	if (fCovered)
	{
		// if partition consumer is defined below, do not pass distribution
		// requirements down as this will cause the consumer and enforcer to be
		// in separate slices
		return GPOS_NEW(mp) CDistributionSpecAny(this->Eopid());
	}

	return PdsPassThru(mp, exprhdl, pdsInput, child_index);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PrsRequired
//
//	@doc:
//		Compute required rewindability of the n-th child
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalParallelPartitionSelector::PrsRequired(CMemoryPool *mp,
										CExpressionHandle &exprhdl,
										CRewindabilitySpec *prsRequired,
										ULONG child_index,
										CDrvdPropArray *,  // pdrgpdpCtxt
										ULONG			   // ulOptReq
) const
{
	GPOS_ASSERT(0 == child_index);

	return PrsPassThru(mp, exprhdl, prsRequired, child_index);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PcteRequired
//
//	@doc:
//		Compute required CTE map of the n-th child
//
//---------------------------------------------------------------------------
CCTEReq *
CPhysicalParallelPartitionSelector::PcteRequired(CMemoryPool *,		   //mp,
										 CExpressionHandle &,  //exprhdl,
										 CCTEReq *pcter,
										 ULONG
#ifdef GPOS_DEBUG
child_index
#endif
	,
										 CDrvdPropArray *,	//pdrgpdpCtxt,
										 ULONG				//ulOptReq
) const
{
	GPOS_ASSERT(0 == child_index);
	return PcterPushThru(pcter);
}

CPartitionPropagationSpec *
CPhysicalParallelPartitionSelector::PppsRequired(
	CMemoryPool *mp, CExpressionHandle &,
	CPartitionPropagationSpec *pppsRequired,
	ULONG child_index GPOS_ASSERTS_ONLY, CDrvdPropArray *, ULONG) const
{
	GPOS_ASSERT(child_index == 0);

	CPartitionPropagationSpec *pps_result =
		GPOS_NEW(mp) CPartitionPropagationSpec(mp);
	pps_result->InsertAllExcept(pppsRequired, m_scan_id);
	return pps_result;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::FProvidesReqdCols
//
//	@doc:
//		Check if required columns are included in output columns
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelPartitionSelector::FProvidesReqdCols(CExpressionHandle &exprhdl,
											  CColRefSet *pcrsRequired,
											  ULONG	 // ulOptReq
) const
{
	return FUnaryProvidesReqdCols(exprhdl, pcrsRequired);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PosDerive
//
//	@doc:
//		Derive sort order
//
//---------------------------------------------------------------------------
COrderSpec *
CPhysicalParallelPartitionSelector::PosDerive(CMemoryPool *,  // mp
									  CExpressionHandle &exprhdl) const
{
	return PosDerivePassThruOuter(exprhdl);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PdsDerive
//
//	@doc:
//		Derive distribution
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelPartitionSelector::PdsDerive(CMemoryPool *,  // mp
									  CExpressionHandle &exprhdl) const
{
	return PdsDerivePassThruOuter(exprhdl);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::PrsDerive
//
//	@doc:
//		Derive rewindability
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalParallelPartitionSelector::PrsDerive(CMemoryPool *mp,
									  CExpressionHandle &exprhdl) const
{
	return PrsDerivePassThruOuter(mp, exprhdl);
}

CPartitionPropagationSpec *
CPhysicalParallelPartitionSelector::PppsDerive(CMemoryPool *mp,
									   CExpressionHandle &exprhdl) const
{
	CPartitionPropagationSpec *pps_result =
		GPOS_NEW(mp) CPartitionPropagationSpec(mp);
	CPartitionPropagationSpec *pps_child =
		exprhdl.Pdpplan(0 /* child_index */)->Ppps();

	CBitSet *selector_ids = GPOS_NEW(mp) CBitSet(mp);
	selector_ids->ExchangeSet(m_selector_id);

	pps_result->InsertAll(pps_child);
	pps_result->Insert(m_scan_id, CPartitionPropagationSpec::EpptPropagator,
					   m_mdid, selector_ids, nullptr);
	selector_ids->Release();

	return pps_result;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::EpetDistribution
//
//	@doc:
//		Return the enforcing type for distribution property based on this operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalParallelPartitionSelector::EpetDistribution(CExpressionHandle &exprhdl,
											 const CEnfdDistribution *ped) const
{
	CDrvdPropPlan *pdpplan = exprhdl.Pdpplan(0 /* child_index */);

	if (ped->FCompatible(pdpplan->Pds()))
	{
		// required distribution established by the operator
		return CEnfdProp::EpetUnnecessary;
	}

	CPartitionPropagationSpec *ppps = pdpplan->Ppps();

	if (!ppps->Contains(m_scan_id))
	{
		// ensure we don't create a plan with a motion on top of a partition selector
		return CEnfdProp::EpetProhibited;
	}
	// part consumer found below: enforce distribution on top of part resolver
	return CEnfdProp::EpetRequired;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalPartitionSelector::EpetRewindability
//
//	@doc:
//		Return the enforcing type for rewindability property based on this operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalParallelPartitionSelector::EpetRewindability(
	CExpressionHandle &exprhdl, const CEnfdRewindability *per) const
{
	// get rewindability delivered by the node
	CRewindabilitySpec *prs = CDrvdPropPlan::Pdpplan(exprhdl.Pdp())->Prs();
	if (per->FCompatible(prs))
	{
		// required rewindability is already provided
		return CEnfdProp::EpetUnnecessary;
	}

	// always force spool to be on top of filter
	return CEnfdProp::EpetRequired;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::EpetOrder
//
//	@doc:
//		Return the enforcing type for order property based on this operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalParallelPartitionSelector::EpetOrder(CExpressionHandle &,	// exprhdl,
									  const CEnfdOrder *	// ped
) const
{
	return CEnfdProp::EpetOptional;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::IsContextValidInternal
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelPartitionSelector::IsContextValidInternal(
	CGroup *pgroup, CBitSet *visited_groups) const
{
	if (nullptr == pgroup || visited_groups->Get(pgroup->Id()))
	{
		return false;  // Already visited or null - avoid infinite recursion
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

			if (popScan->UlParallelWorkers() > 1)
				return true;
			else
				return false;
		}

		if (COperator::EopPhysicalParallelAppendTableScan == popChild->Eopid())
		{
			CPhysicalParallelAppendTableScan *popScan =
				CPhysicalParallelAppendTableScan::PopConvert(popChild);

			if (popScan->UlParallelWorkers() > 1)
				return true;
			else
				return false;
		}

		if (CUtils::FPhysicalMotion(popChild))
		{
			// Only allow MotionBroadcastWorkers or MotionHashDistributeWorkers
			if (COperator::EopPhysicalMotionBroadcastWorkers == popChild->Eopid() ||
				COperator::EopPhysicalMotionHashDistributeWorkers == popChild->Eopid())
				return true;
			else
				return false;
		}

		// Recursively check all children (Motion nodes, joins, etc.)
		for (ULONG ul = 0; ul < pgexprChild->Arity(); ul++)
		{
			BOOL fValid = IsContextValidInternal(
				(*pgexprChild)[ul], visited_groups);
			if (fValid)
				return fValid;
		}

		pgexprChild = gpChild.PgexprNext(pgexprChild);
	}

	return false;  // No parallel operators found
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::IsContextValid
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelPartitionSelector::IsContextValid(CGroup *pgroup) const
{
	if (nullptr == pgroup)
	{
		return false;
	}

	// Create visited set to track groups and avoid infinite recursion
	CMemoryPool *mp = COptCtxt::PoctxtFromTLS()->Pmp();
	CBitSet *visited = GPOS_NEW(mp) CBitSet(mp);
	BOOL fVliad = IsContextValidInternal(pgroup, visited);
	visited->Release();

	return fVliad;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::FValidContext
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelPartitionSelector::FValidContext(
	CMemoryPool *,  // mp
	COptimizationContext *poc,
	COptimizationContextArray *) const
{
	// Regular hash join should reject WorkerRandom distributions from children.
	// Only ParallelHashJoin can handle WorkerRandom distributions.
	BOOL fValid = IsContextValid(poc->Pgroup());

	return fValid;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelPartitionSelector::OsPrint
//
//	@doc:
//		Debug print
//
//---------------------------------------------------------------------------
IOstream &
CPhysicalParallelPartitionSelector::OsPrint(IOstream &os) const
{
	os << SzId() << ", Id: " << SelectorId() << ", Scan Id: " << m_scan_id
	   << ", Parallel workers: " << m_parallel_workers << ", Part Table: ";
	MDId()->OsPrint(os);

	return os;
}

// EOF
