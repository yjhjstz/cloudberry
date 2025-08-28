/*-------------------------------------------------------------------------
 *
 * laketablecmds.c
 *	  lake table creation/manipulation commands
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  src/backend/commands/laketablecmds.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/htup_details.h"
#include "access/reloptions.h"
#include "access/table.h"
#include "access/xact.h"
#include "catalog/catalog.h"
#include "catalog/dependency.h"
#include "catalog/heap.h"
#include "catalog/indexing.h"
#include "catalog/objectaccess.h"
#include "catalog/pg_foreign_server.h"
#include "catalog/pg_lake_table.h"
#include "catalog/pg_type.h"
#include "cdb/cdbvars.h"
#include "commands/defrem.h"
#include "commands/laketablecmds.h"
#include "foreign/foreign.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/syscache.h"

/*
 * Validate table type
 */
static void
validate_table_type(const char *table_type)
{
	if (!table_type)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("table type cannot be NULL")));

	if (strcmp(table_type, "ICEBERG") != 0 && strcmp(table_type, "HUDI") != 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("unsupported table type \"%s\"", table_type),
				 errhint("Supported table types are: ICEBERG, HUDI")));
}

/*
 * Validate foreign catalog exists
 */
static Oid
validate_foreign_catalog(const char *catalog_name)
{
	Oid			catalog_oid;

	if (!catalog_name)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("foreign catalog name cannot be NULL")));

	/* Look up the catalog in pg_foreign_server for now */
	catalog_oid = get_foreign_catalog_oid(catalog_name, NULL, false);

	return catalog_oid;
}

/*
 * Validate foreign volume exists
 */
static Oid
validate_foreign_volume(const char *volume_name)
{
	Oid			volume_oid;

	if (!volume_name)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("foreign volume name cannot be NULL")));

	/* Look up the volume in pg_foreign_server for now */
	volume_oid = get_foreign_volume_oid(volume_name, NULL, false);

	return volume_oid;
}

/*
 * CreateLakeTable
 *
 * Create a lake table entry in pg_lake_table after the base table has been created
 */
void
CreateLakeTable(CreateLakeTableStmt *stmt, Oid relId)
{
	Relation	lake_rel;
	Datum		values[Natts_pg_lake_table];
	bool		nulls[Natts_pg_lake_table];
	HeapTuple	tuple;
	Oid			catalog_oid;
	Oid			volume_oid;

	/* Validate inputs */
	validate_table_type(stmt->table_type);
	catalog_oid = validate_foreign_catalog(stmt->foreign_catalog);
	volume_oid = validate_foreign_volume(stmt->foreign_volume);
	/*
	 * Advance command counter to ensure the pg_attribute tuple is visible;
	 * the tuple might be updated to add constraints in previous step.
	 */
	CommandCounterIncrement();
	/*
	 * Open pg_lake_table and insert tuple
	 */
	lake_rel = table_open(LakeTableRelationId, RowExclusiveLock);

	/*
	 * Insert tuple into pg_lake_table
	 */
	memset(values, 0, sizeof(values));
	memset(nulls, false, sizeof(nulls));

	values[Anum_pg_lake_table_ltrelid - 1] = ObjectIdGetDatum(relId);
	values[Anum_pg_lake_table_ltforeign_catalog - 1] = ObjectIdGetDatum(catalog_oid);
	values[Anum_pg_lake_table_ltforeign_volume - 1] = ObjectIdGetDatum(volume_oid);
	values[Anum_pg_lake_table_lttable_type - 1] = CStringGetTextDatum(stmt->table_type);

	/* Handle options */
	if (stmt->options)
	{
		Datum		*options_datums;
		int			noptions;
		ArrayType  *options_array;
		ListCell   *lc;
		int			i = 0;

		noptions = list_length(stmt->options);
		options_datums = (Datum *) palloc(noptions * sizeof(Datum));

		foreach(lc, stmt->options)
		{
			DefElem    *def = (DefElem *) lfirst(lc);
			char	   *option_str;

			option_str = psprintf("%s=%s", def->defname,
								  def->arg ? defGetString(def) : "");
			options_datums[i++] = CStringGetTextDatum(option_str);
		}

		options_array = construct_array(options_datums, noptions,
										TEXTOID, -1, false, 'i');
		values[Anum_pg_lake_table_ltoptions - 1] = PointerGetDatum(options_array);
	}
	else
	{
		nulls[Anum_pg_lake_table_ltoptions - 1] = true;
	}

	tuple = heap_form_tuple(lake_rel->rd_att, values, nulls);

	CatalogTupleInsert(lake_rel, tuple);

	/* Add dependencies */
	

	heap_freetuple(tuple);
	table_close(lake_rel, RowExclusiveLock);
	return;
}
