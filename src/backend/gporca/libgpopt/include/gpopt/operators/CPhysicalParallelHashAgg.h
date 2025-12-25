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
//		CPhysicalParallelHashAgg.h
//
//	@doc:
//		Parallel Hash Aggregate operator with explicit worker count
//---------------------------------------------------------------------------
#ifndef GPOS_CPhysicalParallelHashAgg_H
#define GPOS_CPhysicalParallelHashAgg_H

#include "gpos/base.h"

#include "gpopt/operators/CPhysicalHashAgg.h"


namespace gpopt
{
//---------------------------------------------------------------------------
//	@class:
//		CPhysicalParallelHashAgg
//
//	@doc:
//		Parallel hash-based aggregate operator with explicit worker count
//
//---------------------------------------------------------------------------
class CPhysicalParallelHashAgg : public CPhysicalHashAgg
{
private:
	// Number of parallel workers
	//ULONG m_ulParallelWorkers;

	// Helper function for computing required distribution for Local aggregates
	CDistributionSpec *PdsRequiredForLocal(CMemoryPool *mp,
										   CExpressionHandle &exprhdl,
										   CDistributionSpec *pdsRequired,
										   ULONG child_index,
										   ULONG ulOptReq) const;

	// Helper function for computing required distribution for Global aggregates
	CDistributionSpec *PdsRequiredForGlobal(CMemoryPool *mp,
											CExpressionHandle &exprhdl,
											CDistributionSpec *pdsRequired,
											ULONG child_index,
											ULONG ulOptReq) const;

public:
	CPhysicalParallelHashAgg(const CPhysicalParallelHashAgg &) = delete;

	// ctor
	CPhysicalParallelHashAgg(CMemoryPool *mp, CColRefArray *colref_array,
							 CColRefArray *pdrgpcrMinimal,
							 COperator::EGbAggType egbaggtype,
							 BOOL fGeneratesDuplicates,
							 CColRefArray *pdrgpcrArgDQA, BOOL fMultiStage,
							 BOOL isAggFromSplitDQA,
							 CLogicalGbAgg::EAggStage aggStage,
							 BOOL isAggPushdown,
							 BOOL should_enforce_distribution,
							 ULONG ulParallelWorkers);

	// dtor
	~CPhysicalParallelHashAgg() override;

	// ident accessors
	EOperatorId
	Eopid() const override
	{
		return EopPhysicalParallelHashAgg;
	}

	// return a string for operator name
	const CHAR *
	SzId() const override
	{
		return "CPhysicalParallelHashAgg";
	}

	// conversion function
	static CPhysicalParallelHashAgg *
	PopConvert(COperator *pop)
	{
		GPOS_ASSERT(nullptr != pop);
		GPOS_ASSERT(EopPhysicalParallelHashAgg == pop->Eopid());

		return dynamic_cast<CPhysicalParallelHashAgg *>(pop);
	}

	// match function
	BOOL Matches(COperator *pop) const override;

	// hash function
	ULONG HashValue() const override;

	// compute required distribution of the n-th child
	CDistributionSpec *PdsRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
								   CDistributionSpec *pdsRequired,
								   ULONG child_index, CDrvdPropArray *pdrgpdpCtxt,
								   ULONG ulOptReq) const override;

	// check if optimization contexts is valid
	// Reject if parent requires REWINDABLE (e.g., for NL Join inner child)
	BOOL FValidContext(CMemoryPool *mp, COptimizationContext *poc,
					   COptimizationContextArray *pdrgpocChild) const override;


};	// class CPhysicalParallelHashAgg

}  // namespace gpopt

#endif	// !GPOS_CPhysicalParallelHashAgg_H

// EOF
