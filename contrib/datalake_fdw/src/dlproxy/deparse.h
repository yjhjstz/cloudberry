#ifndef _DEPARSE_H_
#define _DEPARSE_H_

#include "postgres.h"
#include "nodes/pg_list.h"

extern void
datalakeDeparseTargetList(Relation rel, Bitmapset *attrs_used, List **retrieved_attrs);

extern void
datalakeClassifyConditions(List *input_conds,
				   List **remote_conds,
				   List **local_conds);

#endif   /* // _DEPARSE_H_ */
