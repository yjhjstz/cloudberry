#include "postgres.h"
#include "nodes/parsenodes.h"

extern CreateForeignTableStmt* ConvertExternalTableStmt(CreateExternalStmt *createExtStmt);
