#ifndef SORTED_MERGE_C_H
#define SORTED_MERGE_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"

struct List;

void *datalakeCreateSortedMerge(char *filename, List *readers);
bool datalakeSortedMergeNext(void *sortedMerge, int64 *value);
void datalakeSortedMergeClose(void *sortedMerge);

#ifdef __cplusplus
}
#endif

#endif // SORTED_MERGE_C_H
