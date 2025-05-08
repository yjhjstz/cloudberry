CREATE EXTENSION IF NOT EXISTS datalake_fdw;
CREATE EXTENSION IF NOT EXISTS hive_connector;
CREATE FOREIGN DATA WRAPPER datalake_fdw
HANDLER datalake_fdw_handler
VALIDATOR datalake_fdw_validator
OPTIONS (mpp_execute 'all segments');
create user hdfs;
SELECT public.create_foreign_server('hdfs_server', 'hdfs', 'datalake_fdw', 'paa_cluster');
GRANT USAGE ON FOREIGN SERVER hdfs_server TO hdfs;
\c contrib_regression hdfs
DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/avro/onefile hdfs_cluster_name=paa_cluster') FORMAT 'avro';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/orc/onefile hdfs_cluster_name=paa_cluster') FORMAT 'orc';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/parquet/onefile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/text/onefile hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/avro/onefile hdfs_cluster_name=paa_cluster') FORMAT 'avro';
select * from read_one_file order by a, b;

DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/orc/onefile hdfs_cluster_name=paa_cluster') FORMAT 'orc';
select * from read_one_file order by a, b;

DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/parquet/onefile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_one_file order by a, b;

DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/hdfs_user/text/onefile hdfs_cluster_name=paa_cluster') FORMAT 'text';
select * from read_one_file order by a, b;



