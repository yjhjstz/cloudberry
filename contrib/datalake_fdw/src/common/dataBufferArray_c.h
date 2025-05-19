#ifndef DATALAKE_DATABUFFERARRAY_C_H
#define DATALAKE_DATABUFFERARRAY_C_H

#include "postgres.h"

#define DATALAKE_BUFFER_LENGTH (32 * 1024)
#ifdef __cplusplus
extern "C"
{
#endif

void *datalake_buffer_arr_create(uint64 columns);
void datalake_buffer_arr_destroy(void *buffer);


#ifdef __cplusplus
}
#endif

#endif // DATALAKE_DATABUFFERARRAY_C_H