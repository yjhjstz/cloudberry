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
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn.h
//
//	@doc:
//		Parallel left anti semi hash join operator with NotIn semantics
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalParallelLeftAntiSemiHashJoinNotIn_H
#define GPOPT_CPhysicalParallelLeftAntiSemiHashJoinNotIn_H

#include "gpos/base.h"

#include "gpopt/operators/CPhysicalParallelHashJoin.h"

namespace gpopt
{
//---------------------------------------------------------------------------
//	@class:
//		CPhysicalParallelLeftAntiSemiHashJoinNotIn
//
//	@doc:
//		Parallel left anti semi hash join operator with NotIn semantics
//
//---------------------------------------------------------------------------
class CPhysicalParallelLeftAntiSemiHashJoinNotIn : public CPhysicalParallelHashJoin
{
public:
	CPhysicalParallelLeftAntiSemiHashJoinNotIn(
		const CPhysicalParallelLeftAntiSemiHashJoinNotIn &) = delete;

	// ctor
	CPhysicalParallelLeftAntiSemiHashJoinNotIn(
		CMemoryPool *mp, CExpressionArray *pdrgpexprOuterKeys,
		CExpressionArray *pdrgpexprInnerKeys, IMdIdArray *hash_opfamilies,
		BOOL is_null_aware = true,
		CXform::EXformId origin_xform = CXform::ExfSentinel);

	// dtor
	~CPhysicalParallelLeftAntiSemiHashJoinNotIn() override;

	// ident accessors
	EOperatorId
	Eopid() const override
	{
		return EopPhysicalParallelLeftAntiSemiHashJoinNotIn;
	}

	// return a string for operator name
	const CHAR *
	SzId() const override
	{
		return "CPhysicalParallelLeftAntiSemiHashJoinNotIn";
	}

	//-------------------------------------------------------------------------------------
	// Required Plan Properties
	//-------------------------------------------------------------------------------------

	// compute required distribution of the n-th child
	CDistributionSpec *PdsRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
								   CDistributionSpec *pdsRequired,
								   ULONG child_index,
								   CDrvdPropArray *pdrgpdpCtxt,
								   ULONG ulOptReq) const override;

	CEnfdDistribution *Ped(CMemoryPool *mp, CExpressionHandle &exprhdl,
						   CReqdPropPlan *prppInput, ULONG child_index,
						   CDrvdPropArray *pdrgpdpCtxt,
						   ULONG ulDistrReq) override;

	// partition propagation
	CPartitionPropagationSpec *PppsRequired(
		CMemoryPool *mp, CExpressionHandle &exprhdl,
		CPartitionPropagationSpec *pppsRequired, ULONG child_index,
		CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq) const override;

	CPartitionPropagationSpec *PppsDerive(
		CMemoryPool *mp, CExpressionHandle &exprhdl) const override;

	// check if required columns are included in output columns
	// Left anti semi join only propagates columns from outer (left) child
	BOOL FProvidesReqdCols(CExpressionHandle &exprhdl,
						   CColRefSet *pcrsRequired,
						   ULONG ulOptReq) const override;

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	// conversion function
	static CPhysicalParallelLeftAntiSemiHashJoinNotIn *
	PopConvert(COperator *pop)
	{
		GPOS_ASSERT(nullptr != pop);
		GPOS_ASSERT(EopPhysicalParallelLeftAntiSemiHashJoinNotIn == pop->Eopid());

		return dynamic_cast<CPhysicalParallelLeftAntiSemiHashJoinNotIn *>(pop);
	}

};	// class CPhysicalParallelLeftAntiSemiHashJoinNotIn

}  // namespace gpopt

#endif	// !GPOPT_CPhysicalParallelLeftAntiSemiHashJoinNotIn_H

// EOF
