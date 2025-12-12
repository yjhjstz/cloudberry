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
 * CDXLPhysicalParallelAgg.cpp
 *
 */

#include "naucrates/dxl/operators/CDXLPhysicalParallelAgg.h"

#include "naucrates/dxl/operators/CDXLNode.h"
#include "naucrates/dxl/xml/CXMLSerializer.h"

using namespace gpos;
using namespace gpdxl;

//---------------------------------------------------------------------------
//	@function:
//		CDXLPhysicalParallelAgg::CDXLPhysicalParallelAgg
//
//	@doc:
//		Constructor
//
//---------------------------------------------------------------------------
CDXLPhysicalParallelAgg::CDXLPhysicalParallelAgg(CMemoryPool *mp,
												 EdxlAggStrategy dxl_agg_strategy,
												 BOOL stream_safe,
												 ULONG parallel_workers)
	: CDXLPhysicalAgg(mp, dxl_agg_strategy, stream_safe),
	  m_parallel_workers(parallel_workers)
{
	GPOS_ASSERT(parallel_workers > 0);
}

//---------------------------------------------------------------------------
//	@function:
//		CDXLPhysicalParallelAgg::GetDXLOperator
//
//	@doc:
//		Operator type
//
//---------------------------------------------------------------------------
Edxlopid
CDXLPhysicalParallelAgg::GetDXLOperator() const
{
	return EdxlopPhysicalParallelAgg;
}

//---------------------------------------------------------------------------
//	@function:
//		CDXLPhysicalParallelAgg::GetOpNameStr
//
//	@doc:
//		Operator name
//
//---------------------------------------------------------------------------
const CWStringConst *
CDXLPhysicalParallelAgg::GetOpNameStr() const
{
	return CDXLTokens::GetDXLTokenStr(EdxltokenPhysicalAggregate);
}

//---------------------------------------------------------------------------
//	@function:
//		CDXLPhysicalParallelAgg::SerializeToDXL
//
//	@doc:
//		Serialize operator in DXL format
//
//---------------------------------------------------------------------------
void
CDXLPhysicalParallelAgg::SerializeToDXL(CXMLSerializer *xml_serializer,
										const CDXLNode *node) const
{
	const CWStringConst *element_name = GetOpNameStr();

	xml_serializer->OpenElement(
		CDXLTokens::GetDXLTokenStr(EdxltokenNamespacePrefix), element_name);

	// serialize base class attributes
	xml_serializer->AddAttribute(
		CDXLTokens::GetDXLTokenStr(EdxltokenAggStrategy),
		GetAggStrategyNameStr());
	xml_serializer->AddAttribute(
		CDXLTokens::GetDXLTokenStr(EdxltokenAggStreamSafe), IsStreamSafe());

	// serialize parallel workers attribute
	xml_serializer->AddAttribute(
		CDXLTokens::GetDXLTokenStr(EdxltokenParallelWorkers),
		m_parallel_workers);

	// serialize properties
	node->SerializePropertiesToDXL(xml_serializer);

	// serialize grouping columns
	SerializeGroupingColsToDXL(xml_serializer);

	// serialize children (projection list, filter, child)
	node->SerializeChildrenToDXL(xml_serializer);

	xml_serializer->CloseElement(
		CDXLTokens::GetDXLTokenStr(EdxltokenNamespacePrefix), element_name);
}

#ifdef GPOS_DEBUG
//---------------------------------------------------------------------------
//	@function:
//		CDXLPhysicalParallelAgg::AssertValid
//
//	@doc:
//		Checks whether operator node is well-structured
//
//---------------------------------------------------------------------------
void
CDXLPhysicalParallelAgg::AssertValid(const CDXLNode *node,
									 BOOL validate_children) const
{
	// delegate to base class
	CDXLPhysicalAgg::AssertValid(node, validate_children);

	// parallel workers must be greater than 0
	GPOS_ASSERT(m_parallel_workers > 0);
}
#endif	// GPOS_DEBUG

// EOF
