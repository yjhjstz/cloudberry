/*--------------------------------------------------------------------
 * guc_vec.h
 *	  Define Vectorization GUCs
 *
 *
 * Copyright (c) 2016-Present Hashdata, Inc. 
 *
 * IDENTIFICATION
 *	  src/backend/misc/guc_vec.c
 *
 *--------------------------------------------------------------------
 */

#include "postgres.h"

#include "utils/guc_vec.h"
#include "cdb/cdbvars.h"
#include "miscadmin.h"

/* max vectorization count */
int max_batch_size = 0;

/* deciding whether to enable vectorization */
bool enable_vectorization = false;

bool force_vectorization = false;

bool enable_vector_optimizer = false;

/* deciding whether to merge arrow plan */
bool enable_arrow_plan_merge = false;

int min_concatenate_rows = 0;
int min_redistribute_handle_rows = 0;
int partition_top_k = 0;
int take_thread_num = 0;
bool two_phase_take = false;
bool gather_motion_take = false;
int control_memory_resource = 5;
int control_global_memory_resource = 5;
bool enable_vector_memory_resource = false;
int pool_threads = 0;

void
assign_enable_vectorization(bool newval, void *extra)
{
	if (newval == true)
	{
		Gp_interconnect_queue_depth = 4096;
		Gp_interconnect_snd_queue_depth = 4096;
	}
	else
	{
		Gp_interconnect_queue_depth = 4;
		Gp_interconnect_snd_queue_depth = 2;
	}
}
