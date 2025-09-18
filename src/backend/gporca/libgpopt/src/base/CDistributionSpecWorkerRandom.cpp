//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2024 VMware, Inc. or its affiliates.
//
//	@filename:
//		CDistributionSpecWorkerRandom.cpp
//
//	@doc:
//		Specification of worker-level random distribution
//---------------------------------------------------------------------------

#include "gpopt/base/CDistributionSpecWorkerRandom.h"

#include "gpopt/base/CColRefSet.h"
#include "gpopt/base/CDistributionSpecStrictRandom.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CPhysicalMotionRandom.h"
#include "naucrates/traceflags/traceflags.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::CDistributionSpecWorkerRandom
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CDistributionSpecWorkerRandom::CDistributionSpecWorkerRandom(ULONG ulWorkers, CDistributionSpec *pdsSegmentBase)
	: m_ulWorkers(ulWorkers), m_pdsSegmentBase(pdsSegmentBase)
{
	GPOS_ASSERT(ulWorkers > 0);

	if (nullptr == m_pdsSegmentBase)
	{
		// Use regular random distribution as base if not specified
		m_pdsSegmentBase = GPOS_NEW(COptCtxt::PoctxtFromTLS()->Pmp()) CDistributionSpecRandom();
	}

	m_pdsSegmentBase->AddRef();

	if (COptCtxt::PoctxtFromTLS()->FDMLQuery())
	{
		// set duplicate sensitive flag to enforce Hash-Distribution of
		// Const Tables in DML queries
		MarkDuplicateSensitive();
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::~CDistributionSpecWorkerRandom
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CDistributionSpecWorkerRandom::~CDistributionSpecWorkerRandom()
{
	CRefCount::SafeRelease(m_pdsSegmentBase);
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::PdsCreateWorkerRandom
//
//	@doc:
//		Factory method for creating worker-level random distribution
//
//---------------------------------------------------------------------------
CDistributionSpecWorkerRandom *
CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(CMemoryPool *mp, ULONG ulWorkers, CDistributionSpec *pdsBase)
{
	GPOS_ASSERT(nullptr != mp);
	GPOS_ASSERT(ulWorkers > 0);

	return GPOS_NEW(mp) CDistributionSpecWorkerRandom(ulWorkers, pdsBase);
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::Matches
//
//	@doc:
//		Match function
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecWorkerRandom::Matches(const CDistributionSpec *pds) const
{
	if (pds->Edt() == CDistributionSpec::EdtWorkerRandom)
	{
		const CDistributionSpecWorkerRandom *pdsWorkerRandom =
			CDistributionSpecWorkerRandom::PdsConvert(pds);

		// Check if worker counts match and base distributions are compatible
		return (m_ulWorkers == pdsWorkerRandom->m_ulWorkers &&
				IsDuplicateSensitive() == pdsWorkerRandom->IsDuplicateSensitive() &&
				((nullptr == m_pdsSegmentBase && nullptr == pdsWorkerRandom->m_pdsSegmentBase) ||
				 (nullptr != m_pdsSegmentBase && nullptr != pdsWorkerRandom->m_pdsSegmentBase &&
				  m_pdsSegmentBase->Matches(pdsWorkerRandom->m_pdsSegmentBase))));
	}
	else if (pds->Edt() == CDistributionSpec::EdtRandom)
	{
		// Worker random can match regular random if base distribution matches
		const CDistributionSpecRandom *pdsRandom =
			CDistributionSpecRandom::PdsConvert(pds);

		return (nullptr != m_pdsSegmentBase &&
				m_pdsSegmentBase->Matches(pds) &&
				IsDuplicateSensitive() == pdsRandom->IsDuplicateSensitive());
	}

	return false;
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::FSatisfies
//
//	@doc:
//		Check if this distribution spec satisfies the given one
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecWorkerRandom::FSatisfies(const CDistributionSpec *pds) const
{
	if (Matches(pds))
	{
		return true;
	}

	// Handle different distribution types
	if (EdtWorkerRandom == pds->Edt())
	{
		const CDistributionSpecWorkerRandom *pdsWorkerRandom =
			CDistributionSpecWorkerRandom::PdsConvert(pds);

		// Worker-level can satisfy another worker-level if it has the same number of workers
		// and the base segment distribution is compatible
		return (m_ulWorkers == pdsWorkerRandom->m_ulWorkers &&
				(nullptr == m_pdsSegmentBase || nullptr == pdsWorkerRandom->m_pdsSegmentBase ||
				 m_pdsSegmentBase->FSatisfies(pdsWorkerRandom->m_pdsSegmentBase)) &&
				(IsDuplicateSensitive() || !pdsWorkerRandom->IsDuplicateSensitive()));
	}
	else if (EdtRandom == pds->Edt())
	{
		const CDistributionSpecRandom *pdsRandom = CDistributionSpecRandom::PdsConvert(pds);

		// Worker-level can satisfy segment-level requirement
		// if the base segment distribution satisfies it
		return (nullptr != m_pdsSegmentBase &&
				m_pdsSegmentBase->FSatisfies(pds) &&
				(IsDuplicateSensitive() || !pdsRandom->IsDuplicateSensitive()));
	}

	// Standard satisfaction logic for other distribution types
	return EdtAny == pds->Edt() || EdtNonSingleton == pds->Edt() ||
		   EdtNonReplicated == pds->Edt();
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::AppendEnforcers
//
//	@doc:
//		Add required enforcers to dynamic array
//
//---------------------------------------------------------------------------
void
CDistributionSpecWorkerRandom::AppendEnforcers(CMemoryPool *mp,
											   CExpressionHandle &exprhdl,
											   CReqdPropPlan *
#ifdef GPOS_DEBUG
												   prpp
#endif	// GPOS_DEBUG
											   ,
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

	if (GPOS_FTRACE(EopttraceDisableMotionRandom))
	{
		// random Motion is disabled
		return;
	}

	// For worker-level distribution, we create a specialized motion
	// that handles worker-level redistribution
	CDistributionSpec *expr_dist_spec =
		CDrvdPropPlan::Pdpplan(exprhdl.Pdp())->Pds();
	CDistributionSpecRandom *random_dist_spec = nullptr;

	if (CUtils::FDuplicateHazardDistributionSpec(expr_dist_spec))
	{
		// the motion node is enforced on top of a child
		// deriving universal spec or replicated distribution, this motion node
		// will be translated to a result node with hash filter to remove
		// duplicates, therefore we also need to mark it as duplicate sensitive
		random_dist_spec = GPOS_NEW(mp) CDistributionSpecWorkerRandom(m_ulWorkers, m_pdsSegmentBase);
		random_dist_spec->MarkDuplicateSensitive();
	}
	else
	{
		// For worker-level enforcement, we still use worker random distribution
		// but create a specialized motion that will translate to
		// a redistribute motion with worker-level parallelism
		random_dist_spec = GPOS_NEW(mp) CDistributionSpecWorkerRandom(m_ulWorkers, m_pdsSegmentBase);
	}

	// add a distribution enforcer
	pexpr->AddRef();
	CExpression *pexprMotion = GPOS_NEW(mp) CExpression(
		mp, GPOS_NEW(mp) CPhysicalMotionRandom(mp, random_dist_spec), pexpr);
	pdrgpexpr->Append(pexprMotion);
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecWorkerRandom::OsPrint
//
//	@doc:
//		Print function
//
//---------------------------------------------------------------------------
IOstream &
CDistributionSpecWorkerRandom::OsPrint(IOstream &os) const
{
	os << SzId() << "[workers:" << m_ulWorkers << "]";
	if (nullptr != m_pdsSegmentBase)
	{
		os << " base:";
		m_pdsSegmentBase->OsPrint(os);
	}
	if (IsDuplicateSensitive())
	{
		os << " (duplicate sensitive)";
	}
	return os;
}

// EOF