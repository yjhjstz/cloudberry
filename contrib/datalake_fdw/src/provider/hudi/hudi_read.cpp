#include "hudi_read.h"

namespace Datalake {
namespace Internal {

void hudiRead::createHandler(void *sstate)
{
    initParameter(sstate);
    protocolContext = datalakeCreateContext(scanstate->options);
    datalakeProtocolImportStart(scanstate, protocolContext, includes_columns);
}

int64_t hudiRead::read(void *values, void *nulls)
{
    protocolContext->record = (DatalakeInternalRecord *) palloc0(sizeof(DatalakeInternalRecord));
    protocolContext->record->nulls = (bool *)nulls;
    protocolContext->record->values = (Datum *)values;

    return datalakeRowReaderNext(protocolContext->file->reader, protocolContext->record);
}

void hudiRead::destroyHandler()
{
    releaseResources();
    datalakeCleanupContext(protocolContext);
}

}
}
