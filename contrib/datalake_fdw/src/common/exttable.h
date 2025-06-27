
#ifndef EXTTABLE_H
#define EXTTABLE_H



#include "access/formatter.h"
#include "access/sdir.h"
#include "access/url.h"
#include "utils/rel.h"

/*
 * ExternalInsertDescData is used for storing state related
 * to inserting data into a writable external table.
 */
typedef struct DatalakeExternalInsertDescData
{
	Relation	ext_rel;
	URL_FILE   *ext_file;
	char	   *ext_uri;		/* "command:<cmd>" or "tablespace:<path>" */
	bool		ext_noop;		/* no op. this segdb needs to do nothing (e.g.
								 * mirror seg) */

	TupleDesc	ext_tupDesc;

	FmgrInfo   *ext_custom_formatter_func; /* function to convert to custom format */
	List	   *ext_custom_formatter_params; /* list of defelems that hold user's format parameters */

	FormatterData *ext_formatter_data;

	struct CopyToStateData *ext_pstate;	/* data parser control chars and state */

} DatalakeExternalInsertDescData;

typedef DatalakeExternalInsertDescData *DatalakeExternalInsertDesc;

/*
 * datalakeExternalSelectDescData is used for storing state related
 * to selecting data from an external table.
 */
typedef struct DatalakeExternalSelectDescData
{
	ProjectionInfo *projInfo;   /* Information for column projection */
	List *filter_quals;         /* Information for filter pushdown */

} DatalakeExternalSelectDescData;

/*
 * used for scan of external relations with the file protocol
 */
typedef struct DatalakeFileScanDescData
{
	/* scan parameters */
	Relation	fs_rd;			/* target relation descriptor */
	struct URL_FILE *fs_file;	/* the file pointer to our URI */
	char	   *fs_uri;			/* the URI string */
	bool		fs_noop;		/* no op. this segdb has no file to scan */
	uint32      fs_scancounter;	/* copied from struct ExternalScan in plan */

	/* current file parse state */
	struct CopyFromStateData *fs_pstate;

	AttrNumber	num_phys_attrs;
	Datum	   *values;
	bool	   *nulls;
	FmgrInfo   *in_functions;
	Oid		   *typioparams;
	Oid			in_func_oid;

	/* current file scan state */
	TupleDesc	fs_tupDesc;
	HeapTupleData fs_ctup;		/* current tuple in scan, if any */

	/* custom data formatter */
	FmgrInfo   *fs_custom_formatter_func; /* function to convert to custom format */
	List	   *fs_custom_formatter_params; /* list of defelems that hold user's format parameters */
	FormatterData *fs_formatter;

	/* CHECK constraints and partition check quals, if any */
	bool		fs_hasConstraints;
	struct ExprState **fs_constraintExprs;
	bool		fs_isPartition;
	struct ExprState *fs_partitionCheckExpr;
}	DatalakeFileScanDescData;

typedef DatalakeFileScanDescData *DatalakeFileScanDesc;

typedef enum DatalakeDataLineStatus
{
	LINE_OK,
	LINE_ERROR,
	NEED_MORE_DATA,
	END_MARKER
} DatalakeDataLineStatus;

typedef struct
{
	DatalakeFileScanDesc ess_ScanDesc;

	ExternalSelectDesc externalSelectDesc;

} datalake_exttable_fdw_state;

/*
 * datalakeExternalSelectDescData is used for storing state related
 * to selecting data from an external table.
 */
typedef struct ExternalSelectDescData
{
	ProjectionInfo *projInfo;   /* Information for column projection */
	List *filter_quals;         /* Information for filter pushdown */

} ExternalSelectDescData;

extern void
datalake_to_exttable_BeginForeignScan(ForeignScanState *node, int eflags, void *datalakeState, bool isMasterOnly, List *uriList, List *extOptions);

extern TupleTableSlot *
datalake_to_exttable_IterateForeignScan(ForeignScanState *node);

extern void
datalake_to_exttable_ReScanForeignScan(ForeignScanState *node);

extern void
datalake_to_exttable_EndForeignScan(ForeignScanState *node);

extern void
datalake_to_exttable_BeginForeignModify(ModifyTableState *mtstate,
							ResultRelInfo *rinfo,
							List *fdw_private,
							List* extOption,
							int subplan_index,
							int eflags);

extern TupleTableSlot *
datalake_to_exttable_ExecForeignInsert(EState *estate,
						   ResultRelInfo *rinfo,
						   TupleTableSlot *slot,
						   TupleTableSlot *planSlot);

extern void
datalake_to_exttable_EndForeignModify(EState *estate, ResultRelInfo *rinfo);
#endif /* EXTTABLE_H */