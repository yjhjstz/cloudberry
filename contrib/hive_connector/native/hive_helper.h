#ifndef HIVE_HELPER_H
#define HIVE_HELPER_H

#include "config_parser.h"
#include "libhms/src/hmsclient.h"

extern bool hiveEnableCacheFile;

extern bool hiveEnableDebug;

extern bool
serverHiveConfExists(List *serverConf, const char *serverName);

extern bool
serverHdfsConfExists(List *serverConf, const char *serverName);

extern ConfigItem *
getHiveServerConf(List *serverConf, const char *serverName);

extern HdfsConfigItem*
getHdfsServerConf(List *serverConf, const char *serverName);

extern void
spiExecQuery(const char *query, int expected_result);

extern void
spiExecUtility(const char *query);

extern void
spiExecSelectUDF(const char *query);

extern char *
extractPathFromLocation(const char *location);

extern char *
extractPathFromLocation3x(const char *location);

extern const char *
extractServiceName(const HdfsConfigItem *hdfsConf);

extern const char *
extractNameNodes(const HdfsConfigItem *hdfsConf, const char *serviceName);

extern char *
extractRpcAddr(const HdfsConfigItem *hdfsConf, const char *service, const char *nodeNameStr);

extern char *
formServerCreateStmt(const char *serverName, const char *dataWrapperName, const HdfsConfigItem *hdfsConf);

extern char *
formUserStmt(const char *userName, const char *serverName);

extern char *
formCreateStmt(HmsHandle *hms,
				const char *destTableName,
				const char *serverName,
				const char *location,
				const char *fields,
				const char *hdfsClusterName,
				const char *format);

extern bool
validateMetaData(HmsHandle *hms,
				 const char *hiveDbName,
				 const char *hiveTableName,
				 char ***partKeys,
				 char **field);

extern void
freeArray(char **strArray);

extern const char *
tableFormatConversion(HmsHandle *hms);

extern void
dropTable(const char *table);

extern char *
formCreateStmt2(HmsHandle *hms,
				const char *destTableName,
				const char *serverName,
				const char *location,
				const char *field,
				const char *hdfsClusterName,
				const char *format,
				const char *hiveClusterName,
				const char *hiveDbName,
				const char *hiveTableName,
				char **partKeys);


//Compatibility3X
extern void
spiExec(const char *query);


extern char *
formCreateStmtCompatibility3X(HmsHandle *hms,
			   const char *destTableName,
			   const char *location,
			   const char *fields,
			   const char *hdfsClusterName,
			   const char *format);

extern char *
formCreateStmt2Compatibility3X(HmsHandle *hms,
				const char *destTableName,
				const char *location,
				const char *field,
				const char *hdfsClusterName,
				const char *format,
				const char *hiveDbName,
				const char *hiveTableName,
				const char *hiveClusterName,
				char **partKeys);

extern char *
formCreateStmt3Compatibility3X(HmsHandle *hms,
				const char *destTableName,
				const char *location,
				const char *field,
				const char *hdfsClusterName,
				const char *format,
				const char *hiveDbName,
				const char *hiveTableName,
				const char *hiveClusterName,
				char **partKeys,
				const char* specifyMaxPartitionValue);
extern bool
validateMetaDataCompatibility3X(HmsHandle *hms,
				 const char *hiveDbName,
				 const char *hiveTableName,
				 char ***partKeys,
				 char **field);

void
dropTableCompatibility3X(const char *table, bool isExternal);

#endif // HIVE_HELPER_H
