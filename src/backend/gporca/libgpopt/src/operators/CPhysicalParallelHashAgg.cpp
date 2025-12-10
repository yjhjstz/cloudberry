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
//		CPhysicalParallelHashAgg.cpp
//
//	@doc:
//		Implementation of parallel hash aggregation operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelHashAgg.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpecAny.h"
#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpecHashedWorker.h"
#include "gpopt/base/CDistributionSpecSingleton.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "naucrates/md/IMDFunction.h"

// Forward declaration for gpdbwrappers function
namespace gpdb
{
bool IsParallelModeOK(void);
}

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashAgg::CPhysicalParallelHashAgg
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelHashAgg::CPhysicalParallelHashAgg(
	CMemoryPool *mp, CColRefArray *colref_array, CColRefArray *pdrgpcrMinimal,
	COperator::EGbAggType egbaggtype, BOOL fGeneratesDuplicates,
	CColRefArray *pdrgpcrArgDQA, BOOL fMultiStage, BOOL isAggFromSplitDQA,
	CLogicalGbAgg::EAggStage aggStage, BOOL isAggPushdown,
	BOOL should_enforce_distribution, ULONG ulParallelWorkers)
	: CPhysicalHashAgg(mp, colref_array, pdrgpcrMinimal, egbaggtype,
					   fGeneratesDuplicates, pdrgpcrArgDQA, fMultiStage,
					   isAggFromSplitDQA, aggStage, isAggPushdown,
					   should_enforce_distribution)
{
	GPOS_ASSERT(ulParallelWorkers > 0 &&
				"CPhysicalParallelHashAgg requires workers > 0");
	m_ulParallelWorkers = ulParallelWorkers;

	if (COperator::EgbaggtypeGlobal == egbaggtype)
	{
		SetDistrRequests(1);
	}
	else
	{
		SetDistrRequests(0);
	}
	// Override parent class distribution requests
	// Parallel hash aggregate only needs one distribution request for WorkerRandom
	
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashAgg::~CPhysicalParallelHashAgg
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelHashAgg::~CPhysicalParallelHashAgg() = default;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashAgg::Matches
//
//	@doc:
//		Match function
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelHashAgg::Matches(COperator *pop) const
{
	if (pop->Eopid() != Eopid())
	{
		return false;
	}

	CPhysicalParallelHashAgg *popParallelHashAgg =
		CPhysicalParallelHashAgg::PopConvert(pop);

	// Match parallel worker count
	if (m_ulParallelWorkers != popParallelHashAgg->UlParallelWorkers())
	{
		return false;
	}

	// Match base aggregate properties via parent class
	return CPhysicalHashAgg::Matches(pop);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashAgg::HashValue
//
//	@doc:
//		Hash function
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelHashAgg::HashValue() const
{
	// Combine base hash with parallel worker count
	ULONG ulHash = CPhysicalHashAgg::HashValue();
	ulHash = gpos::CombineHashes(ulHash, gpos::HashValue(&m_ulParallelWorkers));

	return ulHash;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelHashAgg::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//		Parallel hash aggregate only supports Global aggregate
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelHashAgg::PdsRequired(CMemoryPool *mp,
									   CExpressionHandle &,  // exprhdl
									   CDistributionSpec *,  // pdsRequired
									   ULONG,  // child_index
									   CDrvdPropArray *,  // pdrgpdpCtxt
									   ULONG ulOptReq) const
{
	// With SetDistrRequests(1), we only have one distribution request
	GPOS_ASSERT(0 == ulOptReq &&
				"CPhysicalParallelHashAgg only supports single distribution request");

	// Check if parallel mode is enabled and parallel hashagg is not disabled
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

	// If parallel workers not enabled or hashed distribution failed,
	// fall back to requesting any distribution from child
	return GPOS_NEW(mp) CDistributionSpecAny(this->Eopid());
}

// EOF
