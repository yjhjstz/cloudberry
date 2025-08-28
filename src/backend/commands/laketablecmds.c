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
	catalog_oid = get_foreign_server_oid(catalog_name, false);

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
	volume_oid = get_foreign_server_oid(volume_name, false);

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
	values[Anum_pg_lake_table_lttable_type - 1] = CStringGetTextDatum(stmt->table_type);
	values[Anum_pg_lake_table_ltforeign_catalog - 1] = CStringGetTextDatum(stmt->foreign_catalog);
	values[Anum_pg_lake_table_ltforeign_volume - 1] = CStringGetTextDatum(stmt->foreign_volume);

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

/*
 * GetLakeTable
 *
 * Fetch lake table information for a given relation OID
 */
LakeTable *
GetLakeTable(Oid relid)
{
	Relation	lake_rel;
	ScanKeyData key[1];
	SysScanDesc scan;
	HeapTuple	tuple;
	LakeTable  *lake_table = NULL;

	lake_rel = table_open(LakeTableRelationId, AccessShareLock);

	ScanKeyInit(&key[0],
				Anum_pg_lake_table_ltrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relid));

	scan = systable_beginscan(lake_rel, LakeTableOidIndexId, true,
							  NULL, 1, key);

	tuple = systable_getnext(scan);
	if (HeapTupleIsValid(tuple))
	{
		Datum		datum;
		bool		isnull;

		lake_table = (LakeTable *) palloc0(sizeof(LakeTable));
		lake_table->relid = relid;

		/* Extract text fields */
		datum = heap_getattr(tuple, Anum_pg_lake_table_lttable_type,
							 RelationGetDescr(lake_rel), &isnull);
		if (!isnull)
			lake_table->table_type = TextDatumGetCString(datum);

		datum = heap_getattr(tuple, Anum_pg_lake_table_ltforeign_catalog,
							 RelationGetDescr(lake_rel), &isnull);
		if (!isnull)
			lake_table->foreign_catalog = TextDatumGetCString(datum);

		datum = heap_getattr(tuple, Anum_pg_lake_table_ltforeign_volume,
							 RelationGetDescr(lake_rel), &isnull);
		if (!isnull)
			lake_table->foreign_volume = TextDatumGetCString(datum);

		/* Extract options array */
		datum = heap_getattr(tuple, Anum_pg_lake_table_ltoptions,
							 RelationGetDescr(lake_rel), &isnull);
		if (!isnull)
		{
			ArrayType  *options_array = DatumGetArrayTypeP(datum);
			Datum	   *options_datums;
			int			noptions;
			int			i;

			deconstruct_array(options_array, TEXTOID, -1, false, 'i',
							  &options_datums, NULL, &noptions);

			lake_table->options = NIL;
			for (i = 0; i < noptions; i++)
			{
				char	   *option_str = TextDatumGetCString(options_datums[i]);
				char	   *eq_pos = strchr(option_str, '=');
				DefElem    *def;

				if (eq_pos)
				{
					*eq_pos = '\0';
					def = makeDefElem(option_str, (Node *) makeString(eq_pos + 1), -1);
				}
				else
				{
					def = makeDefElem(option_str, NULL, -1);
				}
				lake_table->options = lappend(lake_table->options, def);
			}
		}
	}

	systable_endscan(scan);
	table_close(lake_rel, AccessShareLock);

	return lake_table;
}