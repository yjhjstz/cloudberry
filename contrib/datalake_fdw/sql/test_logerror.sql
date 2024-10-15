DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw;

CREATE EXTENSION IF NOT EXISTS datalake_fdw;

SELECT pg_sleep(5);

CREATE EXTENSION IF NOT EXISTS hive_connector;

SELECT pg_sleep(5);

set vector.enable_vectorization=off;
set datalake.external_table_new_text=false;

CREATE FOREIGN DATA WRAPPER datalake_fdw
HANDLER datalake_fdw_handler
VALIDATOR datalake_fdw_validator
OPTIONS (mpp_execute 'all segments');

SELECT public.create_foreign_server('foreign_server', 'gpadmin', 'datalake_fdw', 'paa_cluster');

-- sync logerror table
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'rows');
\d hive_test_1
select * from hive_test_1 order by id, name;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', false, 20, 'rows');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'percent');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'ROWS');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'PERCENT');
\d hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'rows');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', false, 20, 'rows');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'percent');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'ROWS');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'PERCENT');

-- forceSync
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'rows', true);
\d hive_test_1
select * from hive_test_1 order by id, name;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', false, 20, 'rows', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'percent', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'ROWS', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', true, 20, 'PERCENT', true);
\d hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'rows', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', false, 20, 'rows', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'percent', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'ROWS', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', true, 20, 'PERCENT', true);

-- sync database
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_2;
DROP SCHEMA IF EXISTS hive_database;
create schema hive_database;
select sync_hive_database('hive_cluster', 'hive_logerror_data_test', 'paa_cluster', 'hive_database', false, 20, 'rows', true);
\d hive_database.hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_2;

DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_2;
DROP SCHEMA IF EXISTS hive_database2;
create schema hive_database2;
select sync_hive_database('hive_cluster', 'hive_logerror_data_test', 'paa_cluster', 'hive_database2', true, 20, 'rows', false);
\d hive_database2.hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_2;

--lighting
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'rows');
\d hive_test_1
select * from hive_test_1 order by id, name;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', false, 20, 'rows');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'percent');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'ROWS');
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'PERCENT');
\d hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'rows');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', false, 20, 'rows');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'percent');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'ROWS');

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'PERCENT');

-- forceSync
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'rows', true);
\d hive_test_1
select * from hive_test_1 order by id, name;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', false, 20, 'rows', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'percent', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'ROWS', true);
\d hive_test_1

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1', 'foreign_server', true, 20, 'PERCENT', true);
\d hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'rows', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', false, 20, 'rows', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'percent', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'ROWS', true);

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_logerror_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2', 'foreign_server', true, 20, 'PERCENT', true);

-- sync database
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_2;
DROP SCHEMA IF EXISTS hive_database;
create schema hive_database;
select sync_hive_database('hive_cluster', 'hive_logerror_data_test', 'paa_cluster', 'hive_database', 'foreign_server', false, 20, 'rows', true);
\d hive_database.hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database.hive_test_2;

DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_2;
DROP SCHEMA IF EXISTS hive_database2;
create schema hive_database2;
select sync_hive_database('hive_cluster', 'hive_logerror_data_test', 'paa_cluster', 'hive_database2', 'foreign_server', true, 20, 'rows', false);
\d hive_database2.hive_test_1
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_database2.hive_test_2;