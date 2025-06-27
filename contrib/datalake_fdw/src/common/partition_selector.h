#ifndef DATALAKE_PARTITION_SELECTOR_H
#define DATALAKE_PARTITION_SELECTOR_H


#include "../datalake_def.h"
#include "nodes/pg_list.h"



typedef struct datalakePartitionConstraint
{
	List *partitionValues;
	List *constraints;
} datalakePartitionConstraint;


extern int datalakeGetAttnumber(TupleDesc tupleDesc, const char *attName);

extern List *datalakeSplitString2(const char *value, char deli, char escape);

extern List *datalakeSelectPartitions(List *quals, TupleDesc tupleDesc, List *keys, List *partitions, bool allParts);


void datalakeInitializeConstraints(dataLakeOptions *options, List *quals, TupleDesc tupleDesc);

bool datalakeIsLastPartition(void* scanstate);

int datalakeInitializeDefaultMap(List *attNums,
					 List *constraints,
					 bool *proj,
					 int *defMap,
					 ExprState **defExprs);

bool
equalHMSSpecifyMaxPartitonValue(List *partitionValue, char* specifyMaxPartitonValue);

List *datalakeTransfromHMSPartitions(List *partitions, char* specifyMaxPartitonValue);

Datum
datalakeExecEvalConst(ExprState *exprstate, ExprContext *econtext,
			  bool *isNull, ExprDoneCond *isDone);

Datum
datalakeExecEvalConst2(ExprState *exprstate, ExprContext *econtext,
			  ExprDoneCond *isDone);
#endif
