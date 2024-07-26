#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "postgres.h"

#include "c.h"
#include "utils/guc.h"

#define TRANSFORM_NUM 8
#define TRANSFORM_LEN 18

extern const char* merge_join_partten[];
extern const char* optimizer_enable_motion_broadcast_partten[];
extern const char* partition_top_k_partten;
extern const char* stddev_partten[];
extern const char *seventy_two_control_partten;
extern bool* prev_optimizer;
extern bool* prev_enable_mergejoin;
extern char* prev_vec_stddev;

extern void restore_gucs(void);

extern void try_transform_sql(const char **query_string);

#endif	
