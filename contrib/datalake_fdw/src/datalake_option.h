#ifndef DATALAKE_OPTION_H
#define DATALAKE_OPTION_H

#include "datalake_def.h"

dataLakeOptions *getOptions(Oid foreigntableid);

List* getCopyOptions(Oid foreigntableid);

List* getCustomOption(Oid foreigntableid);

void getCopyLogErrorOptions(Oid foreigntableid, int *rejectlimit,
			   bool *islimitinrows, char *logerrors);

void getURIFromOptions(Oid foreigntableid, char** uri);

void freeDataLakeOptions(dataLakeOptions *options);

void checkValidRecordBatchOpt(dataLakeOptions *options);
#endif							/* DATALAKE_OPTION_H */