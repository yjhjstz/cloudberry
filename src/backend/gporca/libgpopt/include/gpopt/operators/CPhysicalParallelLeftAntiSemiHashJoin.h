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
//		CPhysicalParallelLeftAntiSemiHashJoin.h
//
//	@doc:
//		Parallel left anti semi hash join operator
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalParallelLeftAntiSemiHashJoin_H
#define GPOPT_CPhysicalParallelLeftAntiSemiHashJoin_H

#include "gpos/base.h"

#include "gpopt/operators/CPhysicalParallelHashJoin.h"

namespace gpopt
{
//---------------------------------------------------------------------------
//	@class:
//		CPhysicalParallelLeftAntiSemiHashJoin
//
//	@doc:
//		Parallel left anti semi hash join operator
//
//---------------------------------------------------------------------------
class CPhysicalParallelLeftAntiSemiHashJoin : public CPhysicalParallelHashJoin
{
public:
	CPhysicalParallelLeftAntiSemiHashJoin(
		const CPhysicalParallelLeftAntiSemiHashJoin &) = delete;

	// ctor
	CPhysicalParallelLeftAntiSemiHashJoin(CMemoryPool *mp,
										  CExpressionArray *pdrgpexprOuterKeys,
										  CExpressionArray *pdrgpexprInnerKeys,
										  IMdIdArray *hash_opfamilies,
										  BOOL is_null_aware = true,
										  CXform::EXformId origin_xform =
											  CXform::ExfSentinel);

	// dtor
	~CPhysicalParallelLeftAntiSemiHashJoin() override;

	// ident accessors
	EOperatorId
	Eopid() const override
	{
		return EopPhysicalParallelLeftAntiSemiHashJoin;
	}

	// return a string for operator name
	const CHAR *
	SzId() const override
	{
		return "CPhysicalParallelLeftAntiSemiHashJoin";
	}

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

	// conversion function
	static CPhysicalParallelLeftAntiSemiHashJoin *
	PopConvert(COperator *pop)
	{
		GPOS_ASSERT(nullptr != pop);
		GPOS_ASSERT(EopPhysicalParallelLeftAntiSemiHashJoin == pop->Eopid());

		return dynamic_cast<CPhysicalParallelLeftAntiSemiHashJoin *>(pop);
	}

};	// class CPhysicalParallelLeftAntiSemiHashJoin

}  // namespace gpopt

#endif	// !GPOPT_CPhysicalParallelLeftAntiSemiHashJoin_H

// EOF
