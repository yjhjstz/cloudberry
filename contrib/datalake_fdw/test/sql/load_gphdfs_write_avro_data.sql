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

-- create simple function test
-- write simple file
DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/avro/onefile hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS one_file2;
CREATE WRITABLE EXTERNAL TABLE one_file2(a int, b text) LOCATION('gphdfs://ci-test-data/avro/onefile2 hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into one_file2 values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS two_file;
CREATE WRITABLE EXTERNAL TABLE two_file(a int, b text) LOCATION('gphdfs://ci-test-data/avro/twofile hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into two_file values(1, 'aaaaabbbb');
insert into two_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS more_file;
CREATE WRITABLE EXTERNAL TABLE more_file(a int, b text) LOCATION('gphdfs://ci-test-data/avro/more_file hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into more_file values(1, 'aaaaabbbb');
insert into more_file values(2, 'aaaaabbbb');
insert into more_file values(3, 'aaaaabbbb');
insert into more_file values(4, 'aaaaabbbb');
insert into more_file values(5, 'aaaaabbbb');
insert into more_file values(6, 'aaaaabbbb');
insert into more_file values(7, 'aaaaabbbb');
insert into more_file values(8, 'aaaaabbbb');
insert into more_file values(9, 'aaaaabbbb');
insert into more_file values(10, 'aaaaabbbb');
insert into more_file values(11, 'aaaaabbbb');
insert into more_file values(12, 'aaaaabbbb');
insert into more_file values(13, 'aaaaabbbb');
insert into more_file values(14, 'aaaaabbbb');
insert into more_file values(15, 'aaaaabbbb');
insert into more_file values(16, 'aaaaabbbb');
insert into more_file values(17, 'aaaaabbbb');
insert into more_file values(18, 'aaaaabbbb');
insert into more_file values(19, 'aaaaabbbb');
insert into more_file values(20, 'aaaaabbbb');

-- value have NULL
DROP EXTERNAL TABLE IF EXISTS more_file2;
CREATE WRITABLE EXTERNAL TABLE more_file2(a int, b text) LOCATION('gphdfs://ci-test-data/avro/more_file2 hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into more_file2 values(1, 'aaaaabbbb');
insert into more_file2 values(2, 'aaaaabbbb');
insert into more_file2 values(3, 'aaaaabbbb');
insert into more_file2 values(4, 'aaaaabbbb');
insert into more_file2 values(5, 'NULL');
insert into more_file2 values(6, 'aaaaabbbb');
insert into more_file2 values(7, 'aaaaabbbb');
insert into more_file2 values(8, 'aaaaabbbb');
insert into more_file2 values(9, 'NULL');
insert into more_file2 values(10, 'aaaaabbbb');
insert into more_file2 values(11, 'aaaaabbbb');
insert into more_file2 values(12, 'aaaaabbbb');
insert into more_file2 values(13, 'NULL');
insert into more_file2 values(14, 'NULL');
insert into more_file2 values(15, 'aaaaabbbb');
insert into more_file2 values(NULL, 'aaaaabbbb');
insert into more_file2 values(17, 'aaaaabbbb');
insert into more_file2 values(NULL, 'aaaaabbbb');
insert into more_file2 values(19, 'aaaaabbbb');
insert into more_file2 values(20, 'aaaaabbbb');
