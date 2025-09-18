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

// Forward declare GUC variables (defined in PostgreSQL)
extern bool enable_parallel;
extern int max_parallel_workers_per_gather;

using namespace gpopt;

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
	// Check if parallel processing is enabled via GUC
	if (!enable_parallel)
	{
		return CXform::ExfpNone;
	}

	CLogicalGet *popGet = CLogicalGet::PopConvert(exprhdl.Pop());
	CTableDescriptor *ptabdesc = popGet->Ptabdesc();

	// Don't use parallel scan for partitioned tables (handled elsewhere)
	if (ptabdesc->IsPartitioned())
	{
		return CXform::ExfpNone;
	}

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
	if (enable_parallel)
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