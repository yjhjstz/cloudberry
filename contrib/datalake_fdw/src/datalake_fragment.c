#include "datalake_fragment.h"
#include "common/fileSystemWrapper.h"
#include "common/partition_selector.h"
#include "dlproxy/datalake.h"
#include <curl/curl.h>


static List* SerializeFragmentList(gopherFileInfo* lists, int count, int64_t *totalSize);
static List *get_partition_values(Relation relation, dataLakeOptions *options);
static List *convert_iceberg_hudi_options(dataLakeOptions *options);
static bool ignore_hidden_file(char* name);

static List*
SerializeFragmentList(gopherFileInfo* lists, int count, int64_t *totalSize)
{
	List	   *serializedFragment = NIL;
	for (int i = 0; i < count; i++)
	{
		if (ignore_hidden_file(lists[i].mPath))
		{
			if (external_table_debug)
			{
				elog(LOG, "set guc datalake.external_table_ignore_hidden_file ignore hidden path %s", lists[i].mPath);
			}
			continue;
		}
		if (lists[i].mLength > 0)
		{
			List *fragment = NIL;
			fragment = lappend(fragment, makeString(pstrdup(lists[i].mPath)));
			char buf[20] = {0};
			sprintf(buf, "%ld", lists[i].mLength);
			fragment = lappend(fragment, makeString(pstrdup(buf)));
			serializedFragment = lappend(serializedFragment, fragment);
			if (totalSize)
				*totalSize += lists[i].mLength;
		}
	}
	return serializedFragment;
}

List *
datalakeGetFragmentList(dataLakeOptions *options, int64_t *totalSize)
{
	List *fragment = NIL;
	gopherConfig* conf = datalakeCreateGopherConfig((void*) options->gopher);
	ossFileStream stream = datalakeCreateFileSystem(conf);
	int count = 0;
	gopherFileInfo* lists = datalakeListDir(stream, options->prefix, &count, true);
	fragment = SerializeFragmentList(lists, count, totalSize);
	datalakeFreeListDir(stream, lists, count);
	datalakeGopherDestroyHandle(stream);
	datalakeFreeGopherConfig(conf);

	return fragment;
}

bool
ignore_hidden_file(char* name)
{
	if (external_table_ignore_hidden_file)
	{
		if (name == NULL)
		{
			return false;
		}
		bool ignore_file = false;
		if (name[0] == '.')
		{
			ignore_file = true;
		}
		else if (strstr(name, "/.") != NULL)
		{
			ignore_file = true;
		}
		return ignore_file;
	}
	return false;
}

static List *
get_partition_values(Relation relation, dataLakeOptions *options)
{
	List *locations = NIL;
	ListCell   *cell;
	StringInfoData partitionkey;
	int index = 1;
	int count = list_length(options->hiveOption->hivePartitionKey);

	initStringInfo(&partitionkey);
	foreach(cell, options->hiveOption->hivePartitionKey)
	{
		char *def = (char *) lfirst(cell);
		appendStringInfo(&partitionkey, "%s%s",
							def,
							(index == count) ? "" : ",");
		index++;
	}

	StringInfoData buf;
	initStringInfo(&buf);
	if (options->hdfs_cluster_name == NULL)
	{
		appendStringInfo(&buf, "s3:/%s hive_cluster_name=%s datasource=%s " \
					"cache=%s transactional=%s partition_keys=%s",
			options->filePath,
			options->hive_cluster_name,
			options->hiveOption->datasource,
			(options->gopher->enableCache) ? "true" : "false",
			(options->hiveOption->transactional) ? "true" : "false",
			partitionkey.data);	
	}
	else
	{
		appendStringInfo(&buf, "gphdfs:/%s hive_cluster_name=%s datasource=%s hdfs_cluster_name=%s" \
					"cache=%s transactional=%s partition_keys=%s",
			options->filePath,
			options->hive_cluster_name,
			options->hiveOption->datasource,
			options->hdfs_cluster_name,
			(options->gopher->enableCache) ? "true" : "false",
			(options->hiveOption->transactional) ? "true" : "false",
			partitionkey.data);
	}
	locations = lappend(locations, makeString(pstrdup(buf.data)));

	return datalake_get_external_fragments(RelationGetRelid(relation), 0, NIL, NIL,
								  locations, options->format, false);
}

List *
datalakeGetNextPartitionFragmentList(dataLakeOptions *options, int64_t *totalSize)
{
	ListCell *lckey;
	ListCell *lcvalue;
	List *serializedFragment = NIL;
	List *partitionKeys;
	List *partitionValues;
	datalakePartitionConstraint *pc;

	pc = (datalakePartitionConstraint*) list_nth(options->hiveOption->hivePartitionConstraints,
		options->hiveOption->curPartition);
	partitionValues = pc->partitionValues;
	partitionKeys = options->hiveOption->hivePartitionKey;


	StringInfoData prefix;
	initStringInfo(&prefix);
	appendStringInfoString(&prefix, options->prefix);

	int len = strlen(prefix.data) - 1;
	if (prefix.data[len] != '/')
	{
		appendStringInfoString(&prefix, "/");
	}

	forboth(lckey, partitionKeys, lcvalue, partitionValues)
	{
		char *partKey = (char *) lfirst(lckey);
		char *partValue = (char *) lfirst(lcvalue);
		char *escapedValue = curl_escape(partValue, strlen(partValue));

		appendStringInfo(&prefix, "%s=%s/", partKey, partValue);
		curl_free(escapedValue);
	}

	int count = 0;
	gopherConfig* conf = datalakeCreateGopherConfig((void*) options->gopher);
	ossFileStream stream = datalakeCreateFileSystem(conf);

	gopherFileInfo* lists = datalakeListDir(stream, prefix.data, &count, true);
	for (int i = 0; i < count; i++)
	{
		if (ignore_hidden_file(lists[i].mPath))
		{
			if (external_table_debug)
			{
				elog(LOG, "set guc datalake.external_table_ignore_hidden_file ignore hidden path %s", lists[i].mPath);
			}
			continue;
		}
		if (lists[i].mLength > 0)
		{
			List *fragment = NIL;
			fragment = lappend(fragment, makeString(pstrdup(lists[i].mPath)));
			char buf[64] = {0};
			sprintf(buf, "%ld", lists[i].mLength);
			fragment = lappend(fragment, makeString(pstrdup(buf)));
			serializedFragment = lappend(serializedFragment, fragment);

			if (totalSize)
				totalSize += lists[i].mLength;
		}
	}
	datalakeFreeListDir(stream, lists, count);
	datalakeGopherDestroyHandle(stream);
	datalakeFreeGopherConfig(conf);

	return serializedFragment;
}

static List *
GetPartitionList(Relation relation, List *quals, dataLakeOptions *options)
{
	List *serializedFragment = NIL;
	List *partitionValues;

	partitionValues = get_partition_values(relation, options);
	serializedFragment = lappend(serializedFragment, partitionValues);

	/* set curPartition to zero for datalake foreign table to read from begin */
	options->hiveOption->curPartition = 0;
	return serializedFragment;
}

List *
datalakeGetExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, int64_t *totalSize)
{
	if (FORMAT_IS_ICEBERG(options->format) || FORMAT_IS_HUDI(options->format))
	{
		List *locations = convert_iceberg_hudi_options(options);
		return datalake_get_external_fragments(RelationGetRelid(relation), 0, NIL, NIL,
									  locations, options->format, false);
	}

	if (options->hiveOption->partitiontable)
	{
		return GetPartitionList(relation, quals, options);
	}
	else
	{
		return NIL;
	}
}

static List *
convert_iceberg_hudi_options(dataLakeOptions *options)
{
	List			*result = NIL;
	StringInfoData	buf;

	initStringInfo(&buf);
	appendStringInfo(&buf, "datalake://%s catalog_type=%s server_name=%s",
					 options->filePath, options->catalog_type,
					 options->server_name);
	if (options->table_identifier)
		appendStringInfo(&buf, " table_identifier=%s", options->table_identifier);
	if (options->split_size > 0)
		appendStringInfo(&buf, " split_size=%d", options->split_size);
	if (options->cache_enabled)
		appendStringInfo(&buf, " cache_enabled=%s", options->cache_enabled);
	if (options->query_type)
		appendStringInfo(&buf, " query_type=%s", options->query_type);
	if (options->metadata_table_enable)
		appendStringInfo(&buf, " metadata_table_enable=%s", options->metadata_table_enable);

	result = lappend(result, makeString(pstrdup(buf.data)));
	return result;
}

List *
datalakeDeserializeExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, List *fragmentInfo)
{
	List *fragmentData = NIL;

	if (FORMAT_IS_ICEBERG(options->format) || FORMAT_IS_HUDI(options->format))
	{
		fragmentData = list_delete_first_n(fragmentInfo, FdwScanPrivateFragmentList);
		return fragmentData;
	}

	if (options->hiveOption->partitiontable)
	{
		List *partitionValues = list_nth(fragmentInfo, PrivatePartitionData);

		/* transfrom partition values */
		options->hiveOption->hivePartitionValues = datalakeTransfromHMSPartitions(partitionValues, options->hiveOption->specifyMaxPartitonValue);
		datalakeInitializeConstraints(options, quals, relation->rd_att);
	}

	return fragmentData;
}

void
datalakeFreeFragmentLists(List *fragments)
{
	ListCell *cell;
	foreach(cell, fragments)
	{
		List *fragment = (List*)lfirst(cell);
		char* filePath = strVal(list_nth(fragment, 0));
		if (filePath)
		{
			pfree(filePath);
		}
		char* length = strVal(list_nth(fragment, 1));
		if (length)
		{
			pfree(length);
		}
	}
	list_free_deep(fragments);
}