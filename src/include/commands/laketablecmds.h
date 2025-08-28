/*-------------------------------------------------------------------------
 *
 * laketablecmds.h
 *	  prototypes for laketablecmds.c.
 *
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/laketablecmds.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef LAKETABLECMDS_H
#define LAKETABLECMDS_H

#include "catalog/objectaddress.h"
#include "nodes/params.h"
#include "parser/parse_node.h"
#include "catalog/pg_lake_table.h"

extern void CreateLakeTable(CreateLakeTableStmt *stmt, Oid relId);

#endif /* LAKETABLECMDS_H */