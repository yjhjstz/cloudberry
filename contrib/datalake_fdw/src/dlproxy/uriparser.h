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
 */

#ifndef _URIPARSER_H_
#define _URIPARSER_H_

#include "postgres.h"
#include "fmgr.h"
#include "nodes/pg_list.h"

/*
 * Path constants for accessing dlproxy.
 * All dlproxy's resources are under /DLPROXY_SERVICE_PREFIX/...
 */
#define DLPROXY_SERVICE_PREFIX "dlproxy"

/*
 * Structure to store options data, such as types of fragmenters, accessors and resolvers
 */
typedef struct datalakeOptionData
{
	char	   *key;
	char	   *value;
} datalakeOptionData;

/*
 * DatalakeGPHDUri - Describes the contents of a hadoop uri.
 */
typedef struct DatalakeGPHDUri
{
	char	   *uri;			/* the unparsed user uri    */
	char	   *protocol;		/* the protocol name        */
	char	   *host;			/* host name str            */
	char	   *port;			/* port number as string    */
	char	   *data;			/* data location (path)     */
	char	   *profile;		/* profile option           */
	List	   *options;		/* list of datalakeOptionData       */
} DatalakeGPHDUri;

/*
 * Parses a string URI into a data structure
 */
DatalakeGPHDUri *datalake_parseGPHDUri(const char *uri_str);
DatalakeGPHDUri *datalake_parseGPHDUriHostPort(const char *uri_str, const char *host, const int port);

/*
 * Validation functions
 */
bool datalake_GPHDUri_opt_exists(DatalakeGPHDUri *uri, char *key);
void datalake_GPHDUri_verify_no_duplicate_options(DatalakeGPHDUri *uri);
void datalake_GPHDUri_verify_core_options_exist(DatalakeGPHDUri *uri, List *coreOptions);
const char *datalake_getOptionValue(DatalakeGPHDUri *uri, const char *key);

/*
 * Frees the elements of the data structure
 */
void datalake_freeGPHDUri(DatalakeGPHDUri *uri);

#endif /* _URIPARSER_H_ */
