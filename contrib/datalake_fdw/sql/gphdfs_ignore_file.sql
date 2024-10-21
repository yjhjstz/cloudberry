DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw;

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

set datalake.external_table_ignore_hidden_file = true;
DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (a text)
location('gphdfs://.custom_small_file_lf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (a text)
location('gphdfs://custom_small_file_lf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (a text)
location('gphdfs://ignore/text/ hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE READABLE EXTERNAL TABLE readtable (a text)
location('gphdfs://ignore/text/.ignore/custom_small_file_lf.txt hdfs_cluster_name=paa_cluster') FORMAT 'text' (newline 'LF');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER foreign_server 
OPTIONS (filepath '/.custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER foreign_server 
OPTIONS (filepath '/custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER foreign_server 
OPTIONS (filepath '/ignore/text/', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER foreign_server 
OPTIONS (filepath '/ignore/text/.ignore/custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;
