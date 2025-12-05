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
 * CDXLPhysicalParallelAgg.h
 *
 */

#ifndef GPDXL_CDXLPhysicalParallelAgg_H
#define GPDXL_CDXLPhysicalParallelAgg_H

#include "gpos/base.h"

#include "naucrates/dxl/operators/CDXLPhysicalAgg.h"

namespace gpdxl
{
using namespace gpos;

//---------------------------------------------------------------------------
//	@class:
//		CDXLPhysicalParallelAgg
//
//	@doc:
//		Class for representing DXL parallel aggregate operators
//
//---------------------------------------------------------------------------
class CDXLPhysicalParallelAgg : public CDXLPhysicalAgg
{
private:
	// number of parallel workers
	ULONG m_parallel_workers;

public:
	CDXLPhysicalParallelAgg(const CDXLPhysicalParallelAgg &) = delete;

	// ctor
	CDXLPhysicalParallelAgg(CMemoryPool *mp, EdxlAggStrategy dxl_agg_strategy,
							BOOL stream_safe, ULONG parallel_workers);

	// dtor
	~CDXLPhysicalParallelAgg() override = default;

	// get operator type
	Edxlopid GetDXLOperator() const override;

	// get operator name
	const CWStringConst *GetOpNameStr() const override;

	// get number of parallel workers
	ULONG
	GetParallelWorkers() const
	{
		return m_parallel_workers;
	}

	// serialize operator in DXL format
	void SerializeToDXL(CXMLSerializer *xml_serializer,
						const CDXLNode *node) const override;

	// conversion function
	static CDXLPhysicalParallelAgg *
	Cast(CDXLOperator *dxl_op)
	{
		GPOS_ASSERT(nullptr != dxl_op);
		GPOS_ASSERT(EdxlopPhysicalParallelAgg == dxl_op->GetDXLOperator());

		return dynamic_cast<CDXLPhysicalParallelAgg *>(dxl_op);
	}

#ifdef GPOS_DEBUG
	// checks whether the operator has valid structure, i.e. number and
	// types of child nodes
	void AssertValid(const CDXLNode *node,
					 BOOL validate_children) const override;
#endif	// GPOS_DEBUG

};	// class CDXLPhysicalParallelAgg

}  // namespace gpdxl

#endif	// !GPDXL_CDXLPhysicalParallelAgg_H

// EOF
