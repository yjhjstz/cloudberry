#ifndef DATALAKE_FRAGMENT_H
#define DATALAKE_FRAGMENT_H


#include "datalake_def.h"

List *datalakeGetExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, int64_t *totalSize);

List *datalakeDeserializeExternalFragmentList(Relation relation, List *quals, dataLakeOptions *options, List *fragmentInfo);

List *datalakeGetNextPartitionFragmentList(dataLakeOptions *options, int64_t *totalSize);

List *datalakeGetFragmentList(dataLakeOptions *options, int64_t *totalSize);

void datalakeFreeFragmentLists(List *fragments);

#endif
