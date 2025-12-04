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
 *
 * CDistributionSpecHashedWorker.cpp
 *
 */

#include "gpopt/base/CDistributionSpecHashedWorker.h"

#include "gpopt/base/CColRefSet.h"
#include "gpopt/base/CDrvdPropPlan.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/base/CDistributionSpecWorkerRandom.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CPhysicalMotionHashDistribute.h"
#include "gpopt/operators/CPhysicalMotionHashDistributeWorkers.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::CDistributionSpecHashedWorker
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CDistributionSpecHashedWorker::CDistributionSpecHashedWorker(
	CExpressionArray *pdrgpexpr, BOOL fNullsColocated, ULONG ulWorkers,
	IMdIdArray *opfamilies)
	: CDistributionSpecHashed(pdrgpexpr, fNullsColocated, opfamilies),
	  m_ulWorkers(ulWorkers)
{
	GPOS_ASSERT(ulWorkers > 0);

	if (COptCtxt::PoctxtFromTLS()->FDMLQuery())
	{
		// set duplicate sensitive flag for DML queries
		MarkDuplicateSensitive();
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::Matches
//
//	@doc:
//		Match function
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecHashedWorker::Matches(const CDistributionSpec *pds) const
{
	if (EdtHashedWorker != pds->Edt())
	{
		return false;
	}

	const CDistributionSpecHashedWorker *pdsHashedWorker =
		CDistributionSpecHashedWorker::PdsConvert(pds);

	// Must have same worker count and matching base hashed distribution
	return (m_ulWorkers == pdsHashedWorker->m_ulWorkers &&
			CDistributionSpecHashed::Matches(pds));
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::FSatisfies
//
//	@doc:
//		Check if this distribution satisfies the given one
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecHashedWorker::FSatisfies(const CDistributionSpec *pds) const
{
	if (Matches(pds))
	{
		return true;
	}

	if (EdtHashedWorker == pds->Edt())
	{
		const CDistributionSpecHashedWorker *pdsHashedWorker =
			CDistributionSpecHashedWorker::PdsConvert(pds);

		// Can satisfy if worker counts match and base hashed distribution satisfies
		return (m_ulWorkers == pdsHashedWorker->m_ulWorkers &&
				CDistributionSpecHashed::FSatisfies(pds));
	}

	// Can satisfy general non-restrictive requirements
	return EdtAny == pds->Edt() || EdtNonSingleton == pds->Edt() ||
		   EdtNonReplicated == pds->Edt();
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::AppendEnforcers
//
//	@doc:
//		Add required enforcers to dynamic array
//
//---------------------------------------------------------------------------
void
CDistributionSpecHashedWorker::AppendEnforcers(CMemoryPool *mp,
											   CExpressionHandle &exprhdl,
											   CReqdPropPlan *prpp,
											   CExpressionArray *pdrgpexpr,
											   CExpression *pexpr)
{
	GPOS_ASSERT(nullptr != mp);
	GPOS_ASSERT(nullptr != prpp);
	GPOS_ASSERT(nullptr != pdrgpexpr);
	GPOS_ASSERT(nullptr != pexpr);
	GPOS_ASSERT(!GPOS_FTRACE(EopttraceDisableMotions));
	GPOS_ASSERT(
		this == prpp->Ped()->PdsRequired() &&
		"required plan properties don't match enforced distribution spec");

	// Get child's distribution for duplicate hazard checking
	CDistributionSpec *expr_dist_spec =
		CDrvdPropPlan::Pdpplan(exprhdl.Pdp())->Pds();
	BOOL fDuplicateHazard =
		CUtils::FDuplicateHazardDistributionSpec(expr_dist_spec);

	// Get the required distribution specification
	CDistributionSpec *pdsRequired = prpp->Ped()->PdsRequired();
	GPOS_ASSERT(nullptr != pdsRequired);

	pexpr->AddRef();
	CExpression *pexprMotion = nullptr;

	// Generate appropriate motion based on required distribution type
	switch (pdsRequired->Edt())
	{
		case CDistributionSpec::EdtHashed:
		{
			// Required: Segment-level hashed distribution
			// Consolidate worker outputs to segment-level hashed distribution
			if (GPOS_FTRACE(EopttraceDisableMotionHashDistribute))
			{
				pexpr->Release();
				return;
			}

			CDistributionSpecHashed *pdsHashed =
				CDistributionSpecHashed::PdsConvert(pdsRequired);
			pdsHashed->AddRef();

			if (fDuplicateHazard)
			{
				pdsHashed->MarkDuplicateSensitive();
			}

			pexprMotion = GPOS_NEW(mp) CExpression(
				mp, GPOS_NEW(mp) CPhysicalMotionHashDistribute(mp, pdsHashed),
				pexpr);
			break;
		}

		case CDistributionSpec::EdtHashedWorker:
		{
			// Required: Worker-level hashed distribution with partial aggregation
			// This requires redistributing data among workers within segments
			if (GPOS_FTRACE(EopttraceDisableMotionHashDistributeWorkers))
			{
				pexpr->Release();
				return;
			}

			CDistributionSpecHashedWorker *pdsHashedWorker =
				CDistributionSpecHashedWorker::PdsConvert(pdsRequired);
			// Extract hash expressions and properties
			CExpressionArray *pdrgpexpr = pdsHashedWorker->Pdrgpexpr();
			pdrgpexpr->AddRef();
			IMdIdArray *opfamilies = pdsHashedWorker->Opfamilies();
			if (nullptr != opfamilies)
			{
				opfamilies->AddRef();
			}

			// Create base segment-level hashed distribution
			CDistributionSpecHashed *pdsHashed = GPOS_NEW(mp)
				CDistributionSpecHashed(pdrgpexpr, pdsHashedWorker->FNullsColocated(), opfamilies);

			// Create WorkerRandom distribution with hashed base for the Motion operator
			CDistributionSpecWorkerRandom *pdsWorkerRandom = GPOS_NEW(mp)
				CDistributionSpecWorkerRandom(pdsHashedWorker->UlWorkers(), pdsHashed);

			if (fDuplicateHazard)
			{
				pdsWorkerRandom->MarkDuplicateSensitive();
			}

			pexprMotion = GPOS_NEW(mp) CExpression(
				mp,
				GPOS_NEW(mp) CPhysicalMotionHashDistributeWorkers(mp, pdsWorkerRandom), pexpr);
			break;
		}

		default:
		{
			// For other distribution requirements, no motion can be enforced
			// from HashedWorker distribution
			pexpr->Release();
			return;
		}
	}

	if (nullptr != pexprMotion)
	{
		pdrgpexpr->Append(pexprMotion);
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::HashValue
//
//	@doc:
//		Hash function
//
//---------------------------------------------------------------------------
ULONG
CDistributionSpecHashedWorker::HashValue() const
{
	// Combine worker count with base hashed distribution hash
	ULONG ulHash = gpos::CombineHashes(
		gpos::HashValue<ULONG>(&m_ulWorkers),
		CDistributionSpecHashed::HashValue());

	return ulHash;
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashedWorker::OsPrint
//
//	@doc:
//		Print function
//
//---------------------------------------------------------------------------
IOstream &
CDistributionSpecHashedWorker::OsPrint(IOstream &os) const
{
	os << "HASHED WORKER [workers:" << m_ulWorkers << "] ";
	// Call base class print for hash expressions
	CDistributionSpecHashed::OsPrint(os);
	return os;
}

// EOF
