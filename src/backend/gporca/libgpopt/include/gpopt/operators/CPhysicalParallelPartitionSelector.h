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
 * CPhysicalParallelPartitionSelector.h
 *
 * IDENTIFICATION
 *	  src/backend/gporca/libgpopt/include/gpopt/operators/CPhysicalParallelPartitionSelector.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef GPOPT_CPhysicalParallelPartitionSelector_H
#define GPOPT_CPhysicalParallelPartitionSelector_H

#include "gpos/base.h"

#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CPhysical.h"


namespace gpopt
{
//---------------------------------------------------------------------------
//	@class:
//		CPhysicalParallelPartitionSelector
//
//	@doc:
//		Physical parallel partition selector operator used for property enforcement
//
//---------------------------------------------------------------------------
class CPhysicalParallelPartitionSelector : public CPhysical
{
private:
	// Scan id
	ULONG m_scan_id;

	// Unique id per Partition Selector created
	ULONG m_selector_id;

	// mdid of partitioned table
	IMDId *m_mdid;

	// partition selection predicate
	CExpression *m_filter_expr;

	// parallel workers
	ULONG m_parallel_workers;

public:
	CPhysicalParallelPartitionSelector(const CPhysicalParallelPartitionSelector &) = delete;

	// ctor
	CPhysicalParallelPartitionSelector(CMemoryPool *mp, ULONG scan_id,
									   ULONG selector_id, IMDId *mdid,
									   CExpression *pexprScalar, ULONG ulParallelWorkers);

	// dtor
	~CPhysicalParallelPartitionSelector() override;

	// ident accessors
	EOperatorId
	Eopid() const override
	{
		return EopPhysicalParallelPartitionSelector;
	}

	// operator name
	const CHAR *
	SzId() const override
	{
		return "CPhysicalParallelPartitionSelector";
	}

	// scan id
	ULONG
	ScanId() const
	{
		return m_scan_id;
	}

	ULONG
	SelectorId() const
	{
		return m_selector_id;
	}

	// partition table mdid
	IMDId *
	MDId() const
	{
		return m_mdid;
	}

	// return the partition selection predicate
	CExpression *
	FilterExpr() const
	{
		return m_filter_expr;
	}

	// parallel workers
	ULONG
	UlParallelWorkers() const
	{
		return m_parallel_workers;
	}

	// match function
	BOOL Matches(COperator *pop) const override;

	// hash function
	ULONG HashValue() const override;

	// sensitivity to order of inputs
	BOOL
	FInputOrderSensitive() const override
	{
		// operator has one child
		return false;
	}

	//-------------------------------------------------------------------------------------
	// Required Plan Properties
	//-------------------------------------------------------------------------------------

	// compute required output columns of the n-th child
	CColRefSet *PcrsRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
							 CColRefSet *pcrsRequired, ULONG child_index,
							 CDrvdPropArray *pdrgpdpCtxt,
							 ULONG ulOptReq) override;

	// compute required ctes of the n-th child
	CCTEReq *PcteRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
						  CCTEReq *pcter, ULONG child_index,
						  CDrvdPropArray *pdrgpdpCtxt,
						  ULONG ulOptReq) const override;

	// compute required sort order of the n-th child
	COrderSpec *PosRequired(CMemoryPool *mp, CExpressionHandle &exprhdl,
							COrderSpec *posRequired, ULONG child_index,
							CDrvdPropArray *pdrgpdpCtxt,
							ULONG ulOptReq) const override;

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

	CPartitionPropagationSpec *PppsRequired(
		CMemoryPool *mp, CExpressionHandle &exprhdl,
		CPartitionPropagationSpec *prsRequired, ULONG child_index,
		CDrvdPropArray *pdrgpdpCtxt, ULONG ulOptReq) const override;

	// check if required columns are included in output columns
	BOOL FProvidesReqdCols(CExpressionHandle &exprhdl, CColRefSet *pcrsRequired,
						   ULONG ulOptReq) const override;

	//-------------------------------------------------------------------------------------
	// Derived Plan Properties
	//-------------------------------------------------------------------------------------

	// derive sort order
	COrderSpec *PosDerive(CMemoryPool *mp,
						  CExpressionHandle &exprhdl) const override;

	// derive distribution
	CDistributionSpec *PdsDerive(CMemoryPool *mp,
								 CExpressionHandle &exprhdl) const override;

	// derive rewindability
	CRewindabilitySpec *PrsDerive(CMemoryPool *mp,
								  CExpressionHandle &exprhdl) const override;

	CPartitionPropagationSpec *PppsDerive(
		CMemoryPool *mp, CExpressionHandle &exprhdl) const override;

	//-------------------------------------------------------------------------------------
	// Enforced Properties
	//-------------------------------------------------------------------------------------

	// return distribution property enforcing type for this operator
	CEnfdProp::EPropEnforcingType EpetDistribution(
		CExpressionHandle &exprhdl,
		const CEnfdDistribution *ped) const override;

	// return rewindability property enforcing type for this operator
	CEnfdProp::EPropEnforcingType EpetRewindability(
		CExpressionHandle &exprhdl,
		const CEnfdRewindability *per) const override;

	// return order property enforcing type for this operator
	CEnfdProp::EPropEnforcingType EpetOrder(
		CExpressionHandle &exprhdl, const CEnfdOrder *peo) const override;

	// return true if operator passes through stats obtained from children,
	// this is used when computing stats during costing
	BOOL
	FPassThruStats() const override
	{
		return true;
	}

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	// conversion function
	static CPhysicalParallelPartitionSelector *
	PopConvert(COperator *pop)
	{
		GPOS_ASSERT(nullptr != pop);
		GPOS_ASSERT(EopPhysicalParallelPartitionSelector == pop->Eopid());

		return dynamic_cast<CPhysicalParallelPartitionSelector *>(pop);
	}

	BOOL IsContextValidInternal(CGroup *pgroup, CBitSet *visited_groups) const;

	BOOL IsContextValid(CGroup *pgroup) const;

	// check if optimization context is valid
	BOOL FValidContext(CMemoryPool *mp, COptimizationContext *poc,
					   COptimizationContextArray *pdrgpocChild) const override;

	// debug print
	IOstream &OsPrint(IOstream &os) const override;
};
}	// namespace gpopt

#endif	//	!GPOPT_CPhysicalParallelPartitionSelector_H

//	EOF

