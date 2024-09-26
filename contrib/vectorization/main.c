/*--------------------------------------------------------------------
 * main.c
 *
 * Copyright (c) 2016-Present Hashdata, Inc. 
 *
 * IDENTIFICATION
 *	  main.c
 *
 *--------------------------------------------------------------------
 */
#include "postgres.h"
#include "access/printtup_vec.h"
#include "cdb/cdbvars.h"
#include "utils/guc.h"
#include "optimizer/planner.h"

#include "utils/guc_vec.h"
#include "hook/hook.h"
#include "utils/am_vec.h"
#include "optimizer/planner_vec.h"
#include "tcop/tcopprot.h"
#include "vecnodes/nodes.h"
#include "vecexecutor/executor.h"
#include "vecexecutor/nodeShareInputScan.h"

#include "transform.h"

PG_MODULE_MAGIC;

exec_simple_query_hook_type exec_simple_query_hook_prev = NULL;
static void exec_simple_query_vec(const char *query_string);

void		_PG_init(void);

void
_PG_init(void)
{
    DefineCustomIntVariable("vector.max_batch_size",
                            "max vectorization executor row count handle in one batch",
                            NULL,
                            &max_batch_size,
                            16384,
                            0, 163840,
                            PGC_USERSET,
							GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);
    DefineCustomBoolVariable("vector.enable_vectorization",
                             "Enables the planner's use of vectorized plans."
                             "Notice: gp_interconnect_queue_depth and gp_interconnect_snd_queue_depth"
                             "will be set to 4096 when vectorization is enabled,"
                             "and restore to default value when disabled.",
                             NULL,
                             &enable_vectorization,
                             false,
                             PGC_USERSET,
							 GUC_GPDB_NEED_SYNC,
                             NULL, assign_enable_vectorization, NULL);
    DefineCustomBoolVariable("vector.enable_vector_optimizer",
							 "This guc enables optimizing better plan for vectorization",
							 NULL,
							 &enable_vector_optimizer,
							 false,
							 PGC_USERSET,
							 GUC_GPDB_NO_SYNC,
							 NULL, NULL, NULL);
    DefineCustomBoolVariable("vector.force_vectorization",
                             "Force the planner's use of vectorized plans."
                             "If the plan produced by the current optimizer does not support vectorization, "
                             "it will switch to regenerate the plan using another optimizer. "
                             "If the plans produced by both optimizers cannot be vectorized, "
                             "the plan produced by the preset optimizer is used",
                             NULL,
                             &force_vectorization,
                             false,
                             PGC_USERSET,
							 GUC_GPDB_NEED_SYNC,
                             NULL, NULL, NULL);
    DefineCustomIntVariable("vector.min_concatenate_rows",
                            "Minimum number of rows to motion concatenate",
                            NULL,
                            &min_concatenate_rows,
                            max_batch_size,
                            0, 65536,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);
    DefineCustomIntVariable("vector.min_redistribute_handle_rows",
                            "Minimum number of rows to motion concatenate",
                            NULL,
                            &min_redistribute_handle_rows,
                            0,
                            0, 163840,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);
    DefineCustomBoolVariable("vector.enable_plan_merge",
                             "merge postgres ExecNode into a big plan if true.",
                             NULL,
                             &enable_plan_merge,
                             false,
                             PGC_USERSET,
                             GUC_GPDB_NEED_SYNC,
                             NULL, NULL, NULL);
    DefineCustomIntVariable("vector.partition_top_k",
                            "Partition selecter for WindowAgg sort",
                            NULL,
                            &partition_top_k,
                            0,
                            0, 10000000,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);
    DefineCustomIntVariable("vector.take_thread_num",
                            "take thread for sort",
                            NULL,
                            &take_thread_num,
                            0,
                            0, 10000000,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);
    DefineCustomBoolVariable("vector.two_phase_take",
                             "take phase for take",
                             NULL,
                             &two_phase_take,
                             false,
                             PGC_USERSET,
                             GUC_GPDB_NEED_SYNC,
                             NULL, NULL, NULL);
    DefineCustomBoolVariable("vector.gather_motion_take",
                             "take for gather motion",
                             NULL,
                             &gather_motion_take,
                             false,
                             PGC_USERSET,
                             GUC_GPDB_NEED_SYNC,
                             NULL, NULL, NULL);
    DefineCustomIntVariable("vector.control_memory_resource",
                            "minimal execution resources",
                            NULL,
                            &control_memory_resource,
                            6,
                            1, 8,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);

    DefineCustomIntVariable("vector.control_global_memory_resource",
                            "global max execution resources",
                            NULL,
                            &control_global_memory_resource,
                            1,
                            1, 10,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);

    DefineCustomBoolVariable("vector.enable_vector_memory_resource",
                             "enable execution resources",
                             NULL,
                             &enable_vector_memory_resource,
                             false,
                             PGC_USERSET,
                             GUC_GPDB_NEED_SYNC,
                             NULL, NULL, NULL);

    DefineCustomIntVariable("vector.pool_threads",
                            "num threads in pool",
                            NULL,
                            &pool_threads,
                            0,
                            0, 64,
                            PGC_USERSET,
                            GUC_GPDB_NEED_SYNC,
                            NULL, NULL, NULL);

    exec_simple_query_hook_prev = exec_simple_query_hook;
    exec_simple_query_hook = exec_simple_query_vec;

    planner_prev = planner_hook; 
    planner_hook = planner_hook_wrapper;

    vec_explain_prev = ExplainOneQuery_hook;
    ExplainOneQuery_hook = VecExplainOneQuery;

    vec_exec_start_prev = ExecutorStart_hook;
    ExecutorStart_hook = ExecutorStartWrapper;

    vec_exec_run_prev = ExecutorRun_hook;
    ExecutorRun_hook = ExecutorRunWrapper;

    vec_exec_end_prev = ExecutorEnd_hook;
    ExecutorEnd_hook = ExecutorEndWrapper;

    /* init vectorization am routine */
    InitAOCSVecHandler();

    RegisterXactCallback(dummy_schema_xact_cb, NULL);

    RegisterVectorExtensibleNode();

}

static void
exec_simple_query_vec(const char *query_string)
{
	if (enable_vector_optimizer && enable_vectorization && Gp_role == GP_ROLE_DISPATCH)
	{
		const char **query_string_ref = &query_string;
		try_transform_sql(query_string_ref);
		exec_simple_query(*query_string_ref);
		restore_gucs();
	}
	else if (exec_simple_query_hook_prev)
		exec_simple_query_hook_prev(query_string);
	else
		exec_simple_query(query_string);
}

PG_FUNCTION_INFO_V1(vector_stddev_in);
PG_FUNCTION_INFO_V1(vector_stddev_out);


Datum
vector_stddev_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("cannot accept a value of a vector type")));

	PG_RETURN_VOID();                       /* keep compiler quiet */
}

Datum
vector_stddev_out(PG_FUNCTION_ARGS)
{
	ereport(ERROR,
			(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
			 errmsg("cannot display a value of a vector type")));

	PG_RETURN_VOID();                       /* keep compiler quiet */
}
