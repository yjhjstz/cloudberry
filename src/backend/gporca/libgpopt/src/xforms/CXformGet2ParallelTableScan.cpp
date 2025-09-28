//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2024 VMware, Inc. or its affiliates.
//
//	@filename:
//		CXformGet2ParallelTableScan.cpp
//
//	@doc:
//		Implementation of transform Get to Parallel TableScan
//		Uses unified parallel degree from max_parallel_workers_per_gather GUC
//
//---------------------------------------------------------------------------

#include "gpopt/xforms/CXformGet2ParallelTableScan.h"

#include "gpos/base.h"

#include "gpopt/hints/CHintUtils.h"
#include "gpopt/metadata/CTableDescriptor.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CLogicalGet.h"
#include "gpopt/operators/CPhysicalParallelTableScan.h"
#include "gpopt/optimizer/COptimizerConfig.h"
#include "naucrates/md/IMDRelation.h"
#include "gpopt/search/CGroupProxy.h"
#include "gpopt/search/CMemo.h"


// Use gpdbwrappers for parallel checks
extern int max_parallel_workers_per_gather;

// Forward declarations for gpdbwrappers functions
namespace gpdb {
	bool IsParallelModeOK(void);
	bool IsParallelPlansEnabled(void);
}

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CXformGet2ParallelTableScan::FHasParallelIncompatibleOps
//
//	@doc:
//		Check if memo contains logical operators that are incompatible
//		with parallel execution (CTE, Dynamic scans, Foreign scans, etc.)
//
//---------------------------------------------------------------------------
BOOL
CXformGet2ParallelTableScan::FHasParallelIncompatibleOps(CExpressionHandle &exprhdl)
{
	CGroupExpression *pgexprHandle = exprhdl.Pgexpr();
	if (nullptr == pgexprHandle)
	{
		return false;
	}

	CGroup *pgroup = pgexprHandle->Pgroup();
	if (nullptr == pgroup)
	{
		return false;
	}

	CMemo *pmemo = pgroup->Pmemo();
	if (nullptr == pmemo)
	{
		return false;
	}

	// Iterate through all groups in memo to check for parallel-incompatible operations
	const ULONG_PTR ulGroups = pmemo->UlpGroups();
	for (ULONG_PTR ul = 0; ul < ulGroups; ul++)
	{
		CGroup *pgroupCurrent = pmemo->Pgroup(ul);
		if (nullptr == pgroupCurrent)
		{
			continue;
		}

		// Check all group expressions in this group using CGroupProxy
		CGroupProxy gp(pgroupCurrent);
		CGroupExpression *pgexpr = gp.PgexprFirst();
		while (nullptr != pgexpr)
		{
			COperator::EOperatorId eopid = pgexpr->Pop()->Eopid();

			// Check for CTE-related operators (incompatible with parallel execution)
			if (COperator::EopLogicalCTEProducer == eopid ||
				COperator::EopLogicalCTEConsumer == eopid ||
				COperator::EopLogicalSequence == eopid ||
				COperator::EopLogicalSequenceProject == eopid)
			{
				return true;
			}

			if (COperator::EopLogicalConstTableGet == eopid)
			{
				// ConstTableGet is not supported in parallel plans
				return true;
			}
			if (COperator::EopLogicalDynamicGet == eopid ||
				COperator::EopLogicalDynamicIndexGet == eopid ||
				COperator::EopLogicalIndexOnlyGet == eopid ||
				COperator::EopLogicalIndexGet == eopid)
			{
				// DynamicGet is not supported in parallel plans
				return true;
			}

			if (COperator::EopLogicalForeignGet == eopid)
			{
				// ForeignScan is not supported in parallel plans
				return true;
			}

			pgexpr = gp.PgexprNext(pgexpr);
		}
	}

	return false;
}

//---------------------------------------------------------------------------
//	@function:
//		CXformGet2ParallelTableScan::CXformGet2ParallelTableScan
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CXformGet2ParallelTableScan::CXformGet2ParallelTableScan(CMemoryPool *mp)
	: CXformImplementation(
		  // pattern
		  GPOS_NEW(mp) CExpression(mp, GPOS_NEW(mp) CLogicalGet(mp)))
{
}

//---------------------------------------------------------------------------
//	@function:
//		CXformGet2ParallelTableScan::Exfp
//
//	@doc:
//		Compute promise of xform based on GUC enable_parallel
//		Uses unified parallel degree from max_parallel_workers_per_gather
//
//---------------------------------------------------------------------------
CXform::EXformPromise
CXformGet2ParallelTableScan::Exfp(CExpressionHandle &exprhdl) const
{
	// Check if parallel plans are enabled in context and parallel processing is safe
	if (!gpdb::IsParallelModeOK())
	{
		return CXform::ExfpNone;
	}

	// Check for parallel-incompatible operations that would conflict with parallel scans
	if (FHasParallelIncompatibleOps(exprhdl))
	{
		return CXform::ExfpNone;
	}
#if 0
	if (nullptr != exprhdl.Pgexpr())
	{
		CGroupExpression *pgexprOrigin = exprhdl.Pgexpr();

		while (nullptr != pgexprOrigin)
		{
			// Check for CTE related transformations
			CXform::EXformId exfid = pgexprOrigin->ExfidOrigin();
			if (CXform::ExfImplementCTEProducer == exfid ||
				CXform::ExfCTEAnchor2Sequence == exfid)
			{
				return CXform::ExfpNone;
			}

			// COperator::EOperatorId eopid = pgexprOrigin->Pop()->Eopid();
			// if (COperator::EopLogicalCTEProducer == eopid ||
			// 	COperator::EopPhysicalCTEProducer == eopid) {
			// 	return CXform::ExfpNone;
			// }

			if (CXform::ExfInvalid == exfid)
			{
				break;
			}

			pgexprOrigin = pgexprOrigin->PgexprOrigin();
		}
	}
#endif
	CLogicalGet *popGet = CLogicalGet::PopConvert(exprhdl.Pop());
	CTableDescriptor *ptabdesc = popGet->Ptabdesc();

	// Don't use parallel scan for partitioned tables (FIXME)
	// if (ptabdesc->IsPartitioned())
	// {
	// 	return CXform::ExfpNone;
	// }

	// Don't use parallel scan for replicated tables
	if (ptabdesc->GetRelDistribution() == IMDRelation::EreldistrReplicated)
	{
		//FIXME: Should we consider replicated tables.
		return CXform::ExfpNone;
	}

	// High promise for parallel scan when enabled
	// All tables will use the same parallel degree from max_parallel_workers_per_gather
	return CXform::ExfpHigh;
}

//---------------------------------------------------------------------------
//	@function:
//		CXformGet2ParallelTableScan::Transform
//
//	@doc:
//		Actual transformation
//
//---------------------------------------------------------------------------
void
CXformGet2ParallelTableScan::Transform(CXformContext *pxfctxt, CXformResult *pxfres,
									   CExpression *pexpr) const
{
	GPOS_ASSERT(nullptr != pxfctxt);
	GPOS_ASSERT(FPromising(pxfctxt->Pmp(), this, pexpr));
	GPOS_ASSERT(FCheckPattern(pexpr));

	CLogicalGet *popGet = CLogicalGet::PopConvert(pexpr->Pop());

	CMemoryPool *mp = pxfctxt->Pmp();

	// create/extract components for alternative
	CName *pname = GPOS_NEW(mp) CName(mp, popGet->Name());

	CTableDescriptor *ptabdesc = popGet->Ptabdesc();
	ptabdesc->AddRef();

	CColRefArray *pdrgpcrOutput = popGet->PdrgpcrOutput();
	GPOS_ASSERT(nullptr != pdrgpcrOutput);
	pdrgpcrOutput->AddRef();

	// Use unified parallel degree from GUC parameter
	ULONG ulParallelWorkers = 1;
	if (gpdb::IsParallelModeOK())
	{
		// Use max_parallel_workers_per_gather as the unified parallel degree for all tables
		// This ensures consistent parallelism across all table scans
		if (max_parallel_workers_per_gather > 0)
		{
			ulParallelWorkers = (ULONG)max_parallel_workers_per_gather;
		}
		else
		{
			// If GUC is not set or is 0, use default value of 2
			// This matches PostgreSQL's default behavior
			ulParallelWorkers = 2;
		}
	}

	// create alternative expression
	CExpression *pexprAlt = GPOS_NEW(mp) CExpression(
		mp,
		GPOS_NEW(mp) CPhysicalParallelTableScan(mp, pname, ptabdesc, pdrgpcrOutput, ulParallelWorkers));
	
	// add alternative to transformation result
	pxfres->Add(pexprAlt);
}

// EOF