#include "dataBufferArray_c.h"
#include "dataBufferArray.h"

using Datalake::Internal::dataBufferArray;

void *datalake_buffer_arr_create(uint64 columns)
{
	dataBufferArray *buffer = (dataBufferArray *) palloc0(sizeof(dataBufferArray));
	buffer->allocDataBufferArray(columns);
	return buffer;	
}

void datalake_buffer_arr_destroy(void *buffer)
{
	dataBufferArray *buffer_ = (dataBufferArray *) buffer;
	buffer_->freeDataBuffer();
	pfree(buffer_);
}
