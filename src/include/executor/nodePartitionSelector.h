/*-------------------------------------------------------------------------
 *
 * nodePartitionSelector.h
 *	  implement the execution of PartitionSelector for selecting partition
 *	  Oids based on a given set of predicates. It works for both constant
 *	  partition elimination and join partition elimination
 *
 * Copyright (c) 2014-Present VMware, Inc. or its affiliates.
 *
 *
 * IDENTIFICATION
 *	    src/include/executor/nodePartitionSelector.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef NODEPARTITIONSELECTOR_H
#define NODEPARTITIONSELECTOR_H

#include "access/parallel.h"

extern PartitionSelectorState* ExecInitPartitionSelector(PartitionSelector *node, EState *estate, int eflags);
extern void ExecEndPartitionSelector(PartitionSelectorState *node);
extern void ExecReScanPartitionSelector(PartitionSelectorState *node);
extern void ExecPartitionSelectorInitializeWorker(PartitionSelectorState *node, ParallelWorkerContext *pwcxt);
extern void ExecPartitionSelectorEstimate(PartitionSelectorState *node, ParallelContext *pcxt);
extern void ExecPartitionSelectorInitializeDSM(PartitionSelectorState *node, ParallelContext *pcxt);

#endif   /* NODEPARTITIONSELECTOR_H */

