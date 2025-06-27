#ifndef DATALAKE_H
#define DATALAKE_H

#include "nodes/pg_list.h"
#include "utils/relcache.h"
#include "src/datalake_def.h"
#include "src/datalake_type.h"

#define CATALOG_TYPE       "catalog_type"
#define SERVER_NAME	       "server_name"
#define TABLE_IDENTIFIER   "table_identifier"

typedef struct datalakeTableFieldDefination
{
	char *fieldName;
	char *fieldTypeName;
	Oid   fieldTypeOid;
	int32 fieldTypeMod1;
	int32 fieldTypeMod2;
} datalakeTableFieldDefination;

typedef struct datalakeCombinedScanTask
{
	List *fileTasks;
} datalakeCombinedScanTask;

extern List *
datalake_get_external_schema(char *profile,
					char *relName,
					char *schemaName,
					List *locations);

extern List *
datalake_get_external_fragments(Oid relid,
					   Index relno,
					   List *restrictInfo,
					   List *targetList,
					   List *locations,
					   DLTblFmt formatType,
					   bool iswritable);

extern List *
datalakeParsePartitionResponse(char *buffer, size_t buffer_size);
extern void
datalakeFreePartitionList(List *partitions);

#endif   /* DATALAKE_H */

