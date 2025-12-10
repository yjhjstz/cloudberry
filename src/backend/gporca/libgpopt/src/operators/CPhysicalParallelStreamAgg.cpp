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
//		CPhysicalParallelStreamAgg.cpp
//
//	@doc:
//		Implementation of parallel stream aggregation operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelStreamAgg.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpecAny.h"
#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpecHashedWorker.h"
#include "gpopt/base/CDistributionSpecSingleton.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "naucrates/md/IMDFunction.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelStreamAgg::CPhysicalParallelStreamAgg
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelStreamAgg::CPhysicalParallelStreamAgg(
	CMemoryPool *mp, CColRefArray *colref_array, CColRefArray *pdrgpcrMinimal,
	COperator::EGbAggType egbaggtype, BOOL fGeneratesDuplicates,
	CColRefArray *pdrgpcrArgDQA, BOOL fMultiStage, BOOL isAggFromSplitDQA,
	CLogicalGbAgg::EAggStage aggStage, BOOL isAggPushdown,
	BOOL should_enforce_distribution, ULONG ulParallelWorkers)
	: CPhysicalStreamAgg(mp, colref_array, pdrgpcrMinimal, egbaggtype,
						 fGeneratesDuplicates, pdrgpcrArgDQA, fMultiStage,
						 isAggFromSplitDQA, aggStage, isAggPushdown,
						 should_enforce_distribution)
{
	GPOS_ASSERT(ulParallelWorkers > 0 &&
				"CPhysicalParallelStreamAgg requires workers > 0");
	m_ulParallelWorkers = ulParallelWorkers;

	if (COperator::EgbaggtypeGlobal == egbaggtype &&
		COptCtxt::PoctxtFromTLS()->HasParallelOperators())
	{
		SetDistrRequests(1);
	}
	else
	{
		SetDistrRequests(0);
	}
	// Override parent class distribution requests
	// Parallel stream aggregate only needs one distribution request for WorkerRandom
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelStreamAgg::~CPhysicalParallelStreamAgg
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelStreamAgg::~CPhysicalParallelStreamAgg() = default;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelStreamAgg::Matches
//
//	@doc:
//		Match function
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelStreamAgg::Matches(COperator *pop) const
{
	if (pop->Eopid() != Eopid())
	{
		return false;
	}

	CPhysicalParallelStreamAgg *popParallelStreamAgg =
		CPhysicalParallelStreamAgg::PopConvert(pop);

	// Match parallel worker count
	if (m_ulParallelWorkers != popParallelStreamAgg->UlParallelWorkers())
	{
		return false;
	}

	// Match base aggregate properties via parent class
	return CPhysicalStreamAgg::Matches(pop);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelStreamAgg::HashValue
//
//	@doc:
//		Hash function
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelStreamAgg::HashValue() const
{
	// Combine base hash with parallel worker count
	ULONG ulHash = CPhysicalStreamAgg::HashValue();
	ulHash = gpos::CombineHashes(ulHash, gpos::HashValue(&m_ulParallelWorkers));

	return ulHash;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelStreamAgg::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//		Parallel stream aggregate only supports Global aggregate
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelStreamAgg::PdsRequired(CMemoryPool *mp,
										CExpressionHandle &exprhdl,
										CDistributionSpec *pdsRequired,
										ULONG child_index,
										CDrvdPropArray *pdrgpdpCtxt GPOS_UNUSED,
										ULONG ulOptReq) const
{
	// With SetDistrRequests(1), we only have one distribution request
	GPOS_ASSERT(0 == ulOptReq &&
				"CPhysicalParallelStreamAgg only supports single distribution request");

	if (exprhdl.HasOuterRefs())
	{
		return PdsPassThru(mp, exprhdl, pdsRequired, child_index);
	}

	if (0 == m_pdrgpcrMinimal->Size())
	{
		if (CDistributionSpec::EdtSingleton == pdsRequired->Edt())
		{
			// pass through input distribution if it is a singleton
			pdsRequired->AddRef();
			return pdsRequired;
		}

		// otherwise, require a singleton explicitly
		return GPOS_NEW(mp) CDistributionSpecSingleton();
	}

	if (0 == ulOptReq && (IMDFunction::EfsVolatile ==
						  exprhdl.DeriveFunctionProperties(0)->Efs()))
	{
		// request a singleton distribution if child has volatile functions
		return GPOS_NEW(mp) CDistributionSpecSingleton();
	}

	if (FGlobal())
	{
		ULONG ulWorkers = m_ulParallelWorkers;

		// Create hashed distribution on minimal grouping columns
		CDistributionSpec *pdsSpec = PdsMaximalHashed(mp, m_pdrgpcrMinimal);
		if (pdsSpec->Edt() == CDistributionSpec::EdtHashed)
		{
			CDistributionSpecHashed *pdsHashed =
				CDistributionSpecHashed::PdsConvert(pdsSpec);

			// Extract hash expressions and properties
			CExpressionArray *pdrgpexpr = pdsHashed->Pdrgpexpr();
			pdrgpexpr->AddRef();
			BOOL fNullsColocated = pdsHashed->FNullsColocated();
			IMdIdArray *opfamilies = pdsHashed->Opfamilies();
			if (nullptr != opfamilies)
			{
				opfamilies->AddRef();
			}

			// Release the temporary hashed distribution
			pdsHashed->Release();

			// Create HashedWorker distribution to require partial aggregation results
			return GPOS_NEW(mp) CDistributionSpecHashedWorker(
				pdrgpexpr, fNullsColocated, ulWorkers, opfamilies);
		}
		pdsSpec->Release();
	}

	// if there are grouping columns, require a hash distribution explicitly
	return PdsMaximalHashed(mp, m_pdrgpcrMinimal);
}

// EOF
