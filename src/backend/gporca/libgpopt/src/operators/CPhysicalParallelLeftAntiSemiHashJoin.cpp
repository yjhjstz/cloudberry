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
//		CPhysicalParallelLeftAntiSemiHashJoin.cpp
//
//	@doc:
//		Implementation of parallel left anti semi hash join operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelLeftAntiSemiHashJoin.h"

#include "gpos/base.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoin::CPhysicalParallelLeftAntiSemiHashJoin
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftAntiSemiHashJoin::CPhysicalParallelLeftAntiSemiHashJoin(
	CMemoryPool *mp, CExpressionArray *pdrgpexprOuterKeys,
	CExpressionArray *pdrgpexprInnerKeys, IMdIdArray *hash_opfamilies,
	BOOL is_null_aware, CXform::EXformId origin_xform)
	: CPhysicalParallelHashJoin(mp, pdrgpexprOuterKeys, pdrgpexprInnerKeys,
								hash_opfamilies, is_null_aware, origin_xform)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoin::~CPhysicalParallelLeftAntiSemiHashJoin
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftAntiSemiHashJoin::~CPhysicalParallelLeftAntiSemiHashJoin() =
	default;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoin::PppsRequired
//
//	@doc:
//		Compute required partition propagation of the n-th child
//
//---------------------------------------------------------------------------
CPartitionPropagationSpec *
CPhysicalParallelLeftAntiSemiHashJoin::PppsRequired(
	CMemoryPool *mp, CExpressionHandle &exprhdl,
	CPartitionPropagationSpec *pppsRequired, ULONG child_index,
	CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq) const
{
	return PppsRequiredForJoins(mp, exprhdl, pppsRequired, child_index,
								pdrgpdpCtxt, ulOptReq);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoin::PppsDerive
//
//	@doc:
//		Derive partition propagation spec
//
//---------------------------------------------------------------------------
CPartitionPropagationSpec *
CPhysicalParallelLeftAntiSemiHashJoin::PppsDerive(
	CMemoryPool *mp, CExpressionHandle &exprhdl) const
{
	return PppsDeriveForJoins(mp, exprhdl);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftAntiSemiHashJoin::FProvidesReqdCols
//
//	@doc:
//		Check if required columns are included in output columns
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelLeftAntiSemiHashJoin::FProvidesReqdCols(
	CExpressionHandle &exprhdl, CColRefSet *pcrsRequired,
	ULONG  // ulOptReq
) const
{
	// Left anti semi join only propagates columns from outer (left) child
	return FOuterProvidesReqdCols(exprhdl, pcrsRequired);
}

// EOF
