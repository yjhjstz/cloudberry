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
//		CPhysicalParallelHashJoin.h
//
//	@doc:
//		Base parallel hash join operator
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalParallelHashJoin_H
#define GPOPT_CPhysicalParallelHashJoin_H

#include "gpos/base.h"

#include "gpopt/operators/CPhysicalHashJoin.h"

namespace gpopt
{
//---------------------------------------------------------------------------
//	@class:
//		CPhysicalParallelHashJoin
//
//	@doc:
//		Base parallel hash join operator
//
//---------------------------------------------------------------------------
class CPhysicalParallelHashJoin : public CPhysicalHashJoin
{
private:
	// number of parallel workers
	ULONG m_ulParallelWorkers;

public:
	CPhysicalParallelHashJoin(const CPhysicalParallelHashJoin &) = delete;

	// ctor
	CPhysicalParallelHashJoin(CMemoryPool *mp,
							  CExpressionArray *pdrgpexprOuterKeys,
							  CExpressionArray *pdrgpexprInnerKeys,
							  IMdIdArray *hash_opfamilies, BOOL is_null_aware,
							  CXform::EXformId origin_xform = CXform::ExfSentinel);

	// dtor
	~CPhysicalParallelHashJoin() override;

	// parallel workers accessor
	ULONG
	UlParallelWorkers() const
	{
		return m_ulParallelWorkers;
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

	// compute required rewindability of the n-th child
	CRewindabilitySpec *PrsRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
									CRewindabilitySpec *prsRequired,
									ULONG child_index,
									CDrvdPropArray *pdrgpdpCtxt,
									ULONG ulOptReq) const override;

	//-------------------------------------------------------------------------------------
	// Derived Plan Properties
	//-------------------------------------------------------------------------------------

	// derive distribution
	CDistributionSpec *PdsDerive(CMemoryPool *mp,
								 CExpressionHandle &exprhdl) const override;

	// derive rewindability
	CRewindabilitySpec *PrsDerive(CMemoryPool *mp,
								  CExpressionHandle &exprhdl) const override;

	//-------------------------------------------------------------------------------------
	// Enforced Properties
	//-------------------------------------------------------------------------------------

	// return distribution property enforcing type for this operator
	CEnfdProp::EPropEnforcingType EpetDistribution(
		CExpressionHandle &exprhdl,
		const CEnfdDistribution *ped) const override;

	// check if optimization context is valid
	BOOL FValidContext(CMemoryPool *mp, COptimizationContext *poc,
					   COptimizationContextArray *pdrgpocChild) const override;

	// conversion function
	static CPhysicalParallelHashJoin *
	PopConvert(COperator *pop)
	{
		GPOS_ASSERT(nullptr != pop);
		GPOS_ASSERT(CUtils::FParallelHashJoin(pop));

		return dynamic_cast<CPhysicalParallelHashJoin *>(pop);
	}

};	// class CPhysicalParallelHashJoin

}  // namespace gpopt

#endif	// !GPOPT_CPhysicalParallelHashJoin_H

// EOF
