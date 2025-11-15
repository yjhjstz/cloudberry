//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2010 Greenplum, Inc.
//
//	@filename:
//		CParseHandlerBroadcastWorkersMotion.h
//
//	@doc:
//		SAX parse handler class for parsing broadcast workers motion operator nodes.
//---------------------------------------------------------------------------

#ifndef GPDXL_CParseHandlerBroadcastWorkersMotion_H
#define GPDXL_CParseHandlerBroadcastWorkersMotion_H

#include "gpos/base.h"

#include "naucrates/dxl/operators/CDXLPhysicalBroadcastWorkersMotion.h"
#include "naucrates/dxl/parser/CParseHandlerPhysicalOp.h"

namespace gpdxl
{
using namespace gpos;

XERCES_CPP_NAMESPACE_USE

//---------------------------------------------------------------------------
//	@class:
//		CParseHandlerBroadcastWorkersMotion
//
//	@doc:
//		Parse handler for broadcast workers motion operators
//
//---------------------------------------------------------------------------
class CParseHandlerBroadcastWorkersMotion : public CParseHandlerPhysicalOp
{
private:
	// the broadcast workers motion operator
	CDXLPhysicalBroadcastWorkersMotion *m_dxl_op;

	// process the start of an element
	void StartElement(
		const XMLCh *const element_uri,			// URI of element's namespace
		const XMLCh *const element_local_name,	// local part of element's name
		const XMLCh *const element_qname,		// element's qname
		const Attributes &attr					// element's attributes
		) override;

	// process the end of an element
	void EndElement(
		const XMLCh *const element_uri,			// URI of element's namespace
		const XMLCh *const element_local_name,	// local part of element's name
		const XMLCh *const element_qname		// element's qname
		) override;

public:
	CParseHandlerBroadcastWorkersMotion(
		const CParseHandlerBroadcastWorkersMotion &) = delete;

	// ctor
	CParseHandlerBroadcastWorkersMotion(CMemoryPool *mp,
										CParseHandlerManager *parse_handler_mgr,
										CParseHandlerBase *parse_handler_root);
};
}  // namespace gpdxl

#endif	// !GPDXL_CParseHandlerBroadcastWorkersMotion_H

// EOF
