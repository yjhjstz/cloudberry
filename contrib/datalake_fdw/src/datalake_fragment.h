#ifndef DATALAKE_FRAGMENT_H
#define DATALAKE_FRAGMENT_H


#include "datalake_def.h"

List *GetExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, int64_t *totalSize);

List *deserializeExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, List *fragmentInfo);

List *GetNextPartitionFragmentList(dataLakeOptions *options, int64_t *totalSize);

List *GetFragmentList(dataLakeOptions *options, int64_t *totalSize);

void freeFragmentLists(List *fragments);

#endif
