//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2010 Greenplum, Inc.
//
//	@filename:
//		CDXLPhysicalBroadcastWorkersMotion.h
//
//	@doc:
//		Class for representing DXL Broadcast Workers motion operators.
//---------------------------------------------------------------------------



#ifndef GPDXL_CDXLPhysicalBroadcastWorkersMotion_H
#define GPDXL_CDXLPhysicalBroadcastWorkersMotion_H

#include "gpos/base.h"

#include "naucrates/dxl/operators/CDXLPhysicalMotion.h"

namespace gpdxl
{
// indices of broadcast workers motion elements in the children array
enum Edxlbwm
{
	EdxlbwmIndexProjList = 0,
	EdxlbwmIndexFilter,
	EdxlbwmIndexSortColList,
	EdxlbwmIndexChild,
	EdxlbwmIndexSentinel
};

//---------------------------------------------------------------------------
//	@class:
//		CDXLPhysicalBroadcastWorkersMotion
//
//	@doc:
//		Class for representing DXL broadcast workers motion operators
//		This motion broadcasts data across workers within each segment
//		(not across segments like regular broadcast)
//
//---------------------------------------------------------------------------
class CDXLPhysicalBroadcastWorkersMotion : public CDXLPhysicalMotion
{
private:
public:
	CDXLPhysicalBroadcastWorkersMotion(const CDXLPhysicalBroadcastWorkersMotion &) = delete;

	// ctor/dtor
	explicit CDXLPhysicalBroadcastWorkersMotion(CMemoryPool *mp);

	// accessors
	Edxlopid GetDXLOperator() const override;
	const CWStringConst *GetOpNameStr() const override;

	// index of relational child node in the children array
	ULONG
	GetRelationChildIdx() const override
	{
		return EdxlbwmIndexChild;
	}

	// serialize operator in DXL format
	void SerializeToDXL(CXMLSerializer *xml_serializer,
						const CDXLNode *dxlnode) const override;

	// conversion function
	static CDXLPhysicalBroadcastWorkersMotion *
	Cast(CDXLOperator *dxl_op)
	{
		GPOS_ASSERT(nullptr != dxl_op);
		GPOS_ASSERT(EdxlopPhysicalMotionBroadcastWorkers == dxl_op->GetDXLOperator());
		return dynamic_cast<CDXLPhysicalBroadcastWorkersMotion *>(dxl_op);
	}

#ifdef GPOS_DEBUG
	// checks whether the operator has valid structure, i.e. number and
	// types of child nodes
	void AssertValid(const CDXLNode *, BOOL validate_children) const override;
#endif	// GPOS_DEBUG
};
}  // namespace gpdxl
#endif	// !GPDXL_CDXLPhysicalBroadcastWorkersMotion_H

// EOF
