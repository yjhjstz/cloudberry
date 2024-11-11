DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server cascade;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw cascade;

CREATE EXTENSION IF NOT EXISTS datalake_fdw;

SELECT pg_sleep(5);

CREATE EXTENSION IF NOT EXISTS hive_connector;

CREATE EXTENSION gp_exttable_delimiter;

SELECT pg_sleep(5);

set vector.enable_vectorization=off;

CREATE FOREIGN DATA WRAPPER datalake_fdw
HANDLER datalake_fdw_handler
VALIDATOR datalake_fdw_validator
OPTIONS (mpp_execute 'all segments');

SELECT public.create_foreign_server('foreign_server', 'gpadmin', 'datalake_fdw', 'paa_cluster');

-- create simple function test
-- write simple file
DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou, entry_delim=',',line_delim=E'\n');
insert into example values('aaaaabbbb', 'cccaaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile2 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou_any, entry_delim=',',line_delim=E'\n');
insert into example values('aaaaabbbb', 'ccaaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile3 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou, entry_delim=',',line_delim=E'\n');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,2000000);

DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile4 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou_any, entry_delim=',',line_delim=E'\n');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,2000000);

DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/more hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou, entry_delim=',',line_delim=E'\n');
insert into example values('aaaaabbbb', 'ccaaaaabbbb');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,2000000);
insert into example values('aaaaabbbb', 'ccaaaaabbbb');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,10);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,100);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,1000);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,10000);


DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/more2 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_ou_any, entry_delim=',',line_delim=E'\n');
insert into example values('aaaaabbbb', 'ccaaaaabbbb');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,2000000);
insert into example values('aaaaabbbb', 'ccaaaaabbbb');
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,10);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,100);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,1000);
insert into example select 'aaaaabbbb', 'ccaaaaabbbb' from generate_series(1,10000);

-- read simple file
DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/onefile2 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/onefile3 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/onefile4 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/invalidpath hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/invalidpath hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');
select count(*) from example;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a varchar(50), b varchar(50)) LOCATION('gphdfs://ci-test-data/custom/more hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in, entry_delim=',',line_delim=E'\n');
select count(*) from example;
select * from example order by a,b limit 10;

DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/more2 hdfs_cluster_name=paa_cluster') FORMAT 'custom' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');
select count(*) from example;
select * from example order by a,b limit 10;

-- invalid test
DROP EXTERNAL TABLE IF EXISTS example;
CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/onefile hdfs_cluster_name=paa_cluster') FORMAT 'csv' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');

DROP EXTERNAL TABLE IF EXISTS example;
CREATE WRITABLE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/invalidinsert hdfs_cluster_name=paa_cluster') FORMAT 'csv' (formatter=delimiter_in_fix, entry_delim=',',line_delim=E'\n');

CREATE EXTERNAL TABLE example(a char(9), b char(9)) LOCATION('gphdfs://ci-test-data/custom/invalidinsert hdfs_cluster_name=paa_cluster') FORMAT 'custom' (entry_delim=',',line_delim=E'\n');