DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server cascade;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw cascade;

CREATE EXTENSION IF NOT EXISTS datalake_fdw;

SELECT pg_sleep(5);

CREATE EXTENSION IF NOT EXISTS hive_connector;

SELECT pg_sleep(5);

set vector.enable_vectorization=off;

CREATE FOREIGN DATA WRAPPER datalake_fdw
HANDLER datalake_fdw_handler
VALIDATOR datalake_fdw_validator
OPTIONS (mpp_execute 'all segments');

SELECT public.create_foreign_server('foreign_server', 'gpadmin', 'datalake_fdw', 'paa_cluster');

-- multi delimiter used newline \r\n
set datalake.external_table_new_text = true;
DROP SERVER IF EXISTS foreign_server_hdfs;
SELECT public.create_foreign_server('foreign_server_hdfs', 'gpadmin', 'datalake_fdw', 'paa_cluster');
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/crlf/custom_file_crlf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CRLF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/crlf/custom_file_crlf2.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CRLF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/crlf/custom_file_crlf3.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CRLF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/crlf/custom_file_crlf4.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CRLF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

-- used newline \n
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf2.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf3.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf4.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

-- used default delimiter \n
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text';
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf2.txt hdfs_cluster_name=paa_cluster') FORMAT 'text';
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf3.txt hdfs_cluster_name=paa_cluster') FORMAT 'text';
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/custom_file_lf4.txt hdfs_cluster_name=paa_cluster') FORMAT 'text';
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

-- used newline \r
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/cr/custom_file_cr.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CR');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/cr/custom_file_cr2.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CR');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/cr/custom_file_cr3.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CR');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/cr/custom_file_cr4.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CR');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

-- total
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/crlf/ hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CRLF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/cr/ hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'CR');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/ hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (i int, a text)
location('gphdfs://test/lf/ hdfs_cluster_name=paa_cluster') FORMAT 'text';
select count(*) from readtable;
DROP EXTERNAL TABLE IF EXISTS readtable;