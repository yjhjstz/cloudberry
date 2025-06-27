#ifndef DATALAKE_RANDOM_SEGMENT_H
#define DATALAKE_RANDOM_SEGMENT_H

#include "../datalake_def.h"

extern List* datalakeSelectRandomSegments(int num_nodes, int random_num);

extern void datalakeExecSegment(List* selectSegments, int cursegid, int cursegnum,
						 bool *exec, int *segindex, int *segnum);

#endif
