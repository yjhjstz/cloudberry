/*-------------------------------------------------------------------------
 *
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
 * CXformGbAgg2ParallelHashAgg.h
 *
 * IDENTIFICATION
 *	  src/backend/gporca/libgpopt/include/gpopt/xforms/CXformGbAgg2ParallelHashAgg.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef GPOPT_CXformGbAgg2ParallelHashAgg_H
#define GPOPT_CXformGbAgg2ParallelHashAgg_H

#include "gpos/base.h"

#include "gpopt/xforms/CXformImplementation.h"

namespace gpopt
{
using namespace gpos;

//---------------------------------------------------------------------------
//	@class:
//		CXformGbAgg2ParallelHashAgg
//
//	@doc:
//		Transform GbAgg to Parallel Hash Aggregate when parallel mode is enabled
//
//---------------------------------------------------------------------------
class CXformGbAgg2ParallelHashAgg : public CXformImplementation
{
private:
	// check if the transformation is applicable
	static BOOL FApplicable(CExpression *pexpr);

public:
	CXformGbAgg2ParallelHashAgg(const CXformGbAgg2ParallelHashAgg &) = delete;

	// ctor
	explicit CXformGbAgg2ParallelHashAgg(CMemoryPool *mp);

	// dtor
	~CXformGbAgg2ParallelHashAgg() override = default;

	// ident accessors
	EXformId
	Exfid() const override
	{
		return ExfGbAgg2ParallelHashAgg;
	}

	// return a string for xform name
	const CHAR *
	SzId() const override
	{
		return "CXformGbAgg2ParallelHashAgg";
	}

	// compute xform promise for a given expression handle
	EXformPromise Exfp(CExpressionHandle &exprhdl) const override;

	// actual transform
	void Transform(CXformContext *pxfctxt, CXformResult *pxfres,
				   CExpression *pexpr) const override;

};	// class CXformGbAgg2ParallelHashAgg

}  // namespace gpopt

#endif	// !GPOPT_CXformGbAgg2ParallelHashAgg_H

// EOF
