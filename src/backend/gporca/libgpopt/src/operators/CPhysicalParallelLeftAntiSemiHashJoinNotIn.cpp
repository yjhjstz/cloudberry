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
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn.cpp
//
//	@doc:
//		Implementation of parallel left anti semi hash join operator with NotIn semantics
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelLeftAntiSemiHashJoinNotIn.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpecReplicated.h"
#include "gpopt/base/CDistributionSpecReplicatedWorkers.h"
#include "gpopt/operators/CExpressionHandle.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::CPhysicalParallelLeftAntiSemiHashJoinNotIn
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftAntiSemiHashJoinNotIn::CPhysicalParallelLeftAntiSemiHashJoinNotIn(
	CMemoryPool *mp, CExpressionArray *pdrgpexprOuterKeys,
	CExpressionArray *pdrgpexprInnerKeys, IMdIdArray *hash_opfamilies,
	BOOL is_null_aware, CXform::EXformId origin_xform)
	: CPhysicalParallelHashJoin(mp, pdrgpexprOuterKeys, pdrgpexprInnerKeys,
								hash_opfamilies, is_null_aware, origin_xform)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::~CPhysicalParallelLeftAntiSemiHashJoinNotIn
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftAntiSemiHashJoinNotIn::~CPhysicalParallelLeftAntiSemiHashJoinNotIn() =
	default;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelLeftAntiSemiHashJoinNotIn::PdsRequired(
	CMemoryPool *mp GPOS_UNUSED, CExpressionHandle &exprhdl GPOS_UNUSED,
	CDistributionSpec *pdsInput GPOS_UNUSED, ULONG child_index GPOS_UNUSED,
	CDrvdPropArray *pdrgpdpCtxt GPOS_UNUSED,
	ULONG ulOptReq
		GPOS_UNUSED	 // identifies which optimization request should be created
) const
{
	GPOS_RAISE(
		CException::ExmaInvalid, CException::ExmiInvalid,
		GPOS_WSZ_LIT(
			"PdsRequired should not be called for CPhysicalParallelLeftAntiSemiHashJoinNotIn"));
	return nullptr;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::Ped
//
//	@doc:
//		Compute required distribution enforcement
//
//---------------------------------------------------------------------------
CEnfdDistribution *
CPhysicalParallelLeftAntiSemiHashJoinNotIn::Ped(
	CMemoryPool *mp, CExpressionHandle &exprhdl, CReqdPropPlan *prppInput,
	ULONG child_index, CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq)
{
	GPOS_ASSERT(2 > child_index);
	GPOS_ASSERT(ulOptReq < UlDistrRequests());

	CEnfdDistribution *enfd_dist = nullptr;

	// Check if this is a broadcast workers request (NumDistrReq() onwards)
	// For LEFT ANTI SEMI JOIN NOT IN, the first broadcast request should use ReplicatedWorkers
	//const ULONG ulHashDistributeRequests = NumDistrReq();

	if ((ulOptReq == 0) &&
		1 == child_index &&
		(FNullableHashKeys(exprhdl.DeriveNotNullColumns(0), false /*fInner*/) ||
		 FNullableHashKeys(exprhdl.DeriveNotNullColumns(1), true /*fInner*/)))
	{
		// we need to replicate the inner if any of the following is true:
		// a. if the outer hash keys are nullable, because the executor needs to detect
		//	  whether the inner is empty, and this needs to be detected everywhere
		// b. if the inner hash keys are nullable, because every segment needs to
		//	  detect nulls coming from the inner child

		// For parallel execution, use ReplicatedWorkers instead of regular Replicated
		// This ensures that the inner relation is replicated to all workers within each segment
		ULONG ulWorkers = UlExtractRequestedWorkers(exprhdl, child_index);
		enfd_dist = GPOS_NEW(mp) CEnfdDistribution(
			CDistributionSpecReplicatedWorkers::PdsCreate(mp, ulWorkers),
			CEnfdDistribution::EdmSatisfy);
	}
	else
	{
		enfd_dist = CPhysicalParallelHashJoin::Ped(mp, exprhdl, prppInput, child_index,
										   pdrgpdpCtxt, ulOptReq);
	}

	// If the LASJ requires a replicated distribution (which will generate
	// a broadcast enforcer), we want to ignore the
	// `optimizer_penalize_broadcast_threshold` value.  Otherwise, we may
	// gather both of its children and do all processing on the
	// coordinator. This will be less performant at best, and cause OOM in
	// the worst case. Between these 2 options, broadcasting one side will
	// always be better.
	if (enfd_dist->PdsRequired()->Edt() == CDistributionSpec::EdtReplicatedWorkers)
	{
		// Extract worker count from the current distribution spec
		CDistributionSpecReplicatedWorkers *pds_current =
			CDistributionSpecReplicatedWorkers::PdsConvert(enfd_dist->PdsRequired());
		ULONG ulWorkers = pds_current->UlWorkers();

		// Create new replicated workers distribution spec
		// Note: CDistributionSpecReplicatedWorkers doesn't have ignore_broadcast_threshold parameter
		CDistributionSpecReplicatedWorkers *pds_rep =
			CDistributionSpecReplicatedWorkers::PdsCreate(mp, ulWorkers);
		CEnfdDistribution::EDistributionMatching matches = enfd_dist->Edm();
		enfd_dist->Release();
		return GPOS_NEW(mp) CEnfdDistribution(pds_rep, matches);
	}

	return enfd_dist;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::PppsRequired
//
//	@doc:
//		Compute required partition propagation of the n-th child
//
//---------------------------------------------------------------------------
CPartitionPropagationSpec *
CPhysicalParallelLeftAntiSemiHashJoinNotIn::PppsRequired(
	CMemoryPool *mp, CExpressionHandle &exprhdl,
	CPartitionPropagationSpec *pppsRequired, ULONG child_index,
	CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq) const
{
	return PppsRequiredForJoins(mp, exprhdl, pppsRequired, child_index,
								pdrgpdpCtxt, ulOptReq);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::PppsDerive
//
//	@doc:
//		Derive partition propagation spec
//
//---------------------------------------------------------------------------
CPartitionPropagationSpec *
CPhysicalParallelLeftAntiSemiHashJoinNotIn::PppsDerive(
	CMemoryPool *mp, CExpressionHandle &exprhdl) const
{
	return PppsDeriveForJoins(mp, exprhdl);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn::FProvidesReqdCols
//
//	@doc:
//		Check if required columns are included in output columns
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelLeftAntiSemiHashJoinNotIn::FProvidesReqdCols(
	CExpressionHandle &exprhdl, CColRefSet *pcrsRequired,
	ULONG  // ulOptReq
) const
{
	// Left anti semi join only propagates columns from outer (left) child
	return FOuterProvidesReqdCols(exprhdl, pcrsRequired);
}

// EOF
