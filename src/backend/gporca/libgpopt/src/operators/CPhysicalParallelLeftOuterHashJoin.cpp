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
//		CPhysicalParallelLeftOuterHashJoin.cpp
//
//	@doc:
//		Implementation of parallel left outer hash join operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelLeftOuterHashJoin.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpecWorkerRandom.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CExpressionHandle.h"

using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftOuterHashJoin::CPhysicalParallelLeftOuterHashJoin
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftOuterHashJoin::CPhysicalParallelLeftOuterHashJoin(
	CMemoryPool *mp,
	CExpressionArray *pdrgpexprOuterKeys,
	CExpressionArray *pdrgpexprInnerKeys,
	IMdIdArray *hash_opfamilies,
	BOOL is_null_aware,
	CXform::EXformId origin_xform
)
	: CPhysicalParallelHashJoin(mp, pdrgpexprOuterKeys, pdrgpexprInnerKeys,
								hash_opfamilies, is_null_aware, origin_xform)
{
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftOuterHashJoin::~CPhysicalParallelLeftOuterHashJoin
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelLeftOuterHashJoin::~CPhysicalParallelLeftOuterHashJoin() = default;


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelLeftOuterHashJoin::PdsDerive
//
//	@doc:
//		Derive distribution
//
//		For Left Outer Join, output distribution MUST be based on the left
//		(outer) child's distribution because all left table rows must be
//		preserved in the output.
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelLeftOuterHashJoin::PdsDerive(
	CMemoryPool *mp,
	CExpressionHandle &exprhdl
) const
{
	// Get left (probe) child's distribution
	CDistributionSpec *pdsLeft = exprhdl.Pdpplan(0 /*child_index*/)->Pds();

	GPOS_ASSERT(nullptr != pdsLeft);

	// Left Outer Join output distribution is always based on left child
	// This ensures all left table rows are preserved

	if (CDistributionSpec::EdtWorkerRandom == pdsLeft->Edt())
	{
		// Left child already has WorkerRandom distribution
		// Pass it through
		pdsLeft->AddRef();
		return pdsLeft;
	}

	// If left child is not WorkerRandom, derive from segment distribution
	// This handles Motion nodes that hide WorkerRandom
	if (CDistributionSpec::EdtHashed == pdsLeft->Edt() ||
		CDistributionSpec::EdtStrictRandom == pdsLeft->Edt())
	{
		// Create WorkerRandom distribution based on left child
		ULONG ulWorkers = UlProbeWorkers();
		GPOS_ASSERT(ulWorkers > 0);
		pdsLeft->AddRef();
		return GPOS_NEW(mp) CDistributionSpecWorkerRandom(ulWorkers, pdsLeft);
	}

	// Fallback: pass through left child's distribution
	pdsLeft->AddRef();
	return pdsLeft;
}

// EOF
