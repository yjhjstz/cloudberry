//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2024 VMware, Inc. or its affiliates.
//
//	@filename:
//		CPhysicalParallelTableScan.cpp
//
//	@doc:
//		Implementation of parallel table scan operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelTableScan.h"

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpec.h"
#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpecRandom.h"
#include "gpopt/base/CDistributionSpecWorkerRandom.h"
#include "gpopt/base/CDistributionSpecSingleton.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/metadata/CName.h"
#include "gpopt/metadata/CTableDescriptor.h"
#include "gpopt/operators/CExpressionHandle.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::CPhysicalParallelTableScan
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelTableScan::CPhysicalParallelTableScan(CMemoryPool *mp)
	: CPhysicalTableScan(mp, GPOS_NEW(mp) CName(GPOS_NEW(mp) CWStringConst(GPOS_WSZ_LIT("parallel_table"))), nullptr, nullptr),
	  m_ulParallelWorkers(1),
	  m_pdsWorkerDistribution(nullptr)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::CPhysicalParallelTableScan
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CPhysicalParallelTableScan::CPhysicalParallelTableScan(CMemoryPool *mp,
													   const CName *pnameAlias,
													   CTableDescriptor *ptabdesc,
													   CColRefArray *pdrgpcrOutput,
													   ULONG ulParallelWorkers)
	: CPhysicalTableScan(mp, pnameAlias, ptabdesc, pdrgpcrOutput),
	  m_ulParallelWorkers(ulParallelWorkers),
	  m_pdsWorkerDistribution(nullptr)
{
	GPOS_ASSERT(ulParallelWorkers > 0);

	// Create worker-level distribution based on table's segment distribution
	if (ulParallelWorkers > 1 && nullptr != m_pds)
	{
		// Create worker-level random distribution using the table's distribution as base
		// The base CPhysicalScan already sets up m_pds from the table descriptor
		m_pdsWorkerDistribution = CDistributionSpecWorkerRandom::PdsCreateWorkerRandom(mp, ulParallelWorkers, m_pds);
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::~CPhysicalParallelTableScan
//
//	@doc:
//		dtor
//
//---------------------------------------------------------------------------
CPhysicalParallelTableScan::~CPhysicalParallelTableScan()
{
	CRefCount::SafeRelease(m_pdsWorkerDistribution);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::HashValue
//
//	@doc:
//		Combine pointer for table descriptor, parallel workers and Eop
//
//---------------------------------------------------------------------------
ULONG
CPhysicalParallelTableScan::HashValue() const
{
	ULONG ulHash = gpos::CombineHashes(CPhysicalTableScan::HashValue(),
									   gpos::HashValue<ULONG>(&m_ulParallelWorkers));
	return ulHash;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::Matches
//
//	@doc:
//		match operator
//
//---------------------------------------------------------------------------
BOOL
CPhysicalParallelTableScan::Matches(COperator *pop) const
{
	if (Eopid() != pop->Eopid())
	{
		return false;
	}

	CPhysicalParallelTableScan *popParallelTableScan = 
		CPhysicalParallelTableScan::PopConvert(pop);
	
	return CPhysicalTableScan::Matches(pop) && 
		   m_ulParallelWorkers == popParallelTableScan->UlParallelWorkers();
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::OsPrint
//
//	@doc:
//		debug print
//
//---------------------------------------------------------------------------
IOstream &
CPhysicalParallelTableScan::OsPrint(IOstream &os) const
{
	os << SzId() << " ";

	// alias of table as referenced in the query
	m_pnameAlias->OsPrint(os);

	// actual name of table in catalog and columns
	os << " (";
	m_ptabdesc->Name().OsPrint(os);
	os << "), Columns: [";

	CUtils::OsPrintDrgPcr(os, m_pdrgpcrOutput);
	os << "], Workers: " << m_ulParallelWorkers;

	return os;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::PstatsDerive
//
//	@doc:
//		Derive statistics for parallel table scan
//
//---------------------------------------------------------------------------
IStatistics *
CPhysicalParallelTableScan::PstatsDerive(CMemoryPool *mp,
										  CExpressionHandle &exprhdl,
										  CReqdPropPlan *prpplan,
										  IStatisticsArray *stats_ctxt) const
{
	// First get the base table statistics (same as regular table scan)
	IStatistics *base_stats = CPhysicalTableScan::PstatsDerive(mp, exprhdl, prpplan, stats_ctxt);
	
	if (nullptr == base_stats || m_ulParallelWorkers <= 1)
	{
		return base_stats;
	}

	// For parallel scan, adjust row count per worker
	// Each worker processes roughly 1/parallel_workers of the data
	CDouble dRows = base_stats->Rows();
	CDouble dAdjustedRows = dRows / CDouble(m_ulParallelWorkers);
	
	// Create new statistics with adjusted row count
	IStatistics *parallel_stats = base_stats->ScaleStats(mp, dAdjustedRows / dRows);
	
	// Release base stats since we created a new one
	base_stats->Release();
	
	return parallel_stats;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalParallelTableScan::PdsDerive
//
//	@doc:
//		Derive distribution for parallel table scan
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalParallelTableScan::PdsDerive(CMemoryPool *mp, CExpressionHandle &exprhdl) const
{
	// If we have a pre-computed worker distribution, use it
	if (nullptr != m_pdsWorkerDistribution)
	{
		m_pdsWorkerDistribution->AddRef();
		return m_pdsWorkerDistribution;
	}

	// Otherwise, derive from the base physical scan
	// This uses the m_pds member from CPhysicalScan
	return CPhysicalScan::PdsDerive(mp, exprhdl);
}

// EOF