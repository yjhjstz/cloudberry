
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
set datalake.external_table_limit_segment_num = 1;
-- simple read function
DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/onefile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_one_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_one_file;

DROP EXTERNAL TABLE IF EXISTS read_one_file2;
CREATE READABLE EXTERNAL TABLE read_one_file2(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/onefile/ hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_one_file2 order by a;
DROP EXTERNAL TABLE IF EXISTS read_one_file2;

DROP EXTERNAL TABLE IF EXISTS read_two_file;
CREATE READABLE EXTERNAL TABLE read_two_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/twofile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_two_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_two_file;

DROP EXTERNAL TABLE IF EXISTS read_more_file;
CREATE READABLE EXTERNAL TABLE read_more_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/more_file hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_more_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_more_file;

DROP EXTERNAL TABLE IF EXISTS read_more_file2;
CREATE READABLE EXTERNAL TABLE read_more_file2(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/more_file2 hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_more_file2 order by a;
DROP EXTERNAL TABLE IF EXISTS read_more_file2;

DROP EXTERNAL TABLE IF EXISTS read_invalid_file_path;
CREATE READABLE EXTERNAL TABLE read_invalid_file_path(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/invalid_path hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_invalid_file_path order by a;
DROP EXTERNAL TABLE IF EXISTS read_invalid_file_path;

DROP EXTERNAL TABLE IF EXISTS read_empty_file_path;
CREATE READABLE EXTERNAL TABLE read_empty_file_path(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/empty_path hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_empty_file_path order by a;
DROP EXTERNAL TABLE IF EXISTS read_empty_file_path;

-- sync table
-- hive partition tinyint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_1', 'paa_cluster', 'hive_type_test_1');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;

-- hive partition smallint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_2;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_2', 'paa_cluster', 'hive_type_test_2');
select * from hive_type_test_2 order by id, name, m;
select count(*) from hive_type_test_2;

-- hive partition int
DROP EXTERNAL TABLE IF EXISTS hive_type_test_3;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_3', 'paa_cluster', 'hive_type_test_3');
select * from hive_type_test_3 order by id, name, m;
select count(*) from hive_type_test_3;

-- hive partition bigint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_4;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_4', 'paa_cluster', 'hive_type_test_4');
select * from hive_type_test_4 order by id, name, m;
select count(*) from hive_type_test_4;

-- hive partition float
DROP EXTERNAL TABLE IF EXISTS hive_type_test_5;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_5', 'paa_cluster', 'hive_type_test_5');
select * from hive_type_test_5 order by id, name, m;
select count(*) from hive_type_test_5;

-- hive partition double
DROP EXTERNAL TABLE IF EXISTS hive_type_test_6;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_6', 'paa_cluster', 'hive_type_test_6');
select * from hive_type_test_6 order by id, name, m;
select count(*) from hive_type_test_6;

-- hive partition string
DROP EXTERNAL TABLE IF EXISTS hive_type_test_7;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_7', 'paa_cluster', 'hive_type_test_7');
select * from hive_type_test_7 order by id, name, m;
select count(*) from hive_type_test_7;

-- hive partition date
DROP EXTERNAL TABLE IF EXISTS hive_type_test_8;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_8', 'paa_cluster', 'hive_type_test_8');
select * from hive_type_test_8 order by id, name, m;
select count(*) from hive_type_test_8;

-- hive partition varchar
DROP EXTERNAL TABLE IF EXISTS hive_type_test_9;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_9', 'paa_cluster', 'hive_type_test_9');
select * from hive_type_test_9 order by id, name, m;
select count(*) from hive_type_test_9;

-- hive partition decimal
DROP EXTERNAL TABLE IF EXISTS hive_type_test_10;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_10', 'paa_cluster', 'hive_type_test_10');
select * from hive_type_test_10 order by id, name, m;
select count(*) from hive_type_test_10;

-- hive simple table
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id;
select * from hive_test_1 where id=1;
select count(*) from hive_test_1 where id=1;
select count(*) from hive_test_1;

-- hive partition and transaction table
DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2');

select * from hive_test_2 order by id, name, name2, name3;
select * from hive_test_2 where name3='a' order by id, name, name2, name3;
select count(*) from hive_test_2;
select count(*) from hive_test_2 where name3='b';
select count(*) from hive_test_2 where name3='c';

-- hive partiton key is int type
DROP EXTERNAL TABLE IF EXISTS hive_test_3;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_3', 'paa_cluster', 'hive_test_3');
select * from hive_test_3 order by id, name, m;
select * from hive_test_3 where m=1 order by id;
select count(*) from hive_test_3;
select count(*) from hive_test_3 where m=2;
select count(*) from hive_test_3 where m=3;

-- hive partiton key is int and char type
DROP EXTERNAL TABLE IF EXISTS hive_test_4;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_4', 'paa_cluster', 'hive_test_4');
select * from hive_test_4 order by id, name, m, n;
select count(*) from hive_test_4;
select * from hive_test_4 where m=2 order by id, name, m, n;
select * from hive_test_4 where n='cc' order by id, name, m, n;

-- hive partiton key is int and char and date type
DROP EXTERNAL TABLE IF EXISTS hive_test_5;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_5', 'paa_cluster', 'hive_test_5');
select * from hive_test_5 order by id, name, m, n, o;
select count(*) from hive_test_5;

-- hive partiton key is int and string and float type
DROP EXTERNAL TABLE IF EXISTS hive_test_6;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_6', 'paa_cluster', 'hive_test_6');
select * from hive_test_6 order by id, name, m, n, o;
select count(*) from hive_test_6;

-- hive partiton key is int and string and decimal and tinyint type
DROP EXTERNAL TABLE IF EXISTS hive_test_7;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_7', 'paa_cluster', 'hive_test_7');
select * from hive_test_7 order by id, name, m, n, o, p;
select count(*) from hive_test_7;

-- hive partiton key is int and string and decimal and tinyint type
DROP EXTERNAL TABLE IF EXISTS hive_test_8;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_8', 'paa_cluster', 'hive_test_8');
select * from hive_test_8 order by id, name, m, n, o, p, q, s;
select count(*) from hive_test_8;

DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_3;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_5;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_7;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_8;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_9;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_10;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_3;
DROP EXTERNAL TABLE IF EXISTS hive_test_4;
DROP EXTERNAL TABLE IF EXISTS hive_test_5;
DROP EXTERNAL TABLE IF EXISTS hive_test_6;
DROP EXTERNAL TABLE IF EXISTS hive_test_7;
DROP EXTERNAL TABLE IF EXISTS hive_test_8;

set datalake.external_table_limit_segment_num = 2;
CREATE EXTENSION IF NOT EXISTS hive_connector;
-- simple read function
DROP EXTERNAL TABLE IF EXISTS read_one_file;
CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/onefile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_one_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_one_file;

DROP EXTERNAL TABLE IF EXISTS read_one_file2;
CREATE READABLE EXTERNAL TABLE read_one_file2(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/onefile/ hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_one_file2 order by a;
DROP EXTERNAL TABLE IF EXISTS read_one_file2;

DROP EXTERNAL TABLE IF EXISTS read_two_file;
CREATE READABLE EXTERNAL TABLE read_two_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/twofile hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_two_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_two_file;

DROP EXTERNAL TABLE IF EXISTS read_more_file;
CREATE READABLE EXTERNAL TABLE read_more_file(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/more_file hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_more_file order by a;
DROP EXTERNAL TABLE IF EXISTS read_more_file;

DROP EXTERNAL TABLE IF EXISTS read_more_file2;
CREATE READABLE EXTERNAL TABLE read_more_file2(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/more_file2 hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_more_file2 order by a;
DROP EXTERNAL TABLE IF EXISTS read_more_file2;

DROP EXTERNAL TABLE IF EXISTS read_invalid_file_path;
CREATE READABLE EXTERNAL TABLE read_invalid_file_path(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/invalid_path hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_invalid_file_path order by a;
DROP EXTERNAL TABLE IF EXISTS read_invalid_file_path;

DROP EXTERNAL TABLE IF EXISTS read_empty_file_path;
CREATE READABLE EXTERNAL TABLE read_empty_file_path(a int, b text) LOCATION('gphdfs://ci-test-data/parquet/empty_path hdfs_cluster_name=paa_cluster') FORMAT 'parquet';
select * from read_empty_file_path order by a;
DROP EXTERNAL TABLE IF EXISTS read_empty_file_path;

-- sync table
-- hive partition tinyint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_1', 'paa_cluster', 'hive_type_test_1');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;

-- hive partition smallint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_2;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_2', 'paa_cluster', 'hive_type_test_2');
select * from hive_type_test_2 order by id, name, m;
select count(*) from hive_type_test_2;

-- hive partition int
DROP EXTERNAL TABLE IF EXISTS hive_type_test_3;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_3', 'paa_cluster', 'hive_type_test_3');
select * from hive_type_test_3 order by id, name, m;
select count(*) from hive_type_test_3;

-- hive partition bigint
DROP EXTERNAL TABLE IF EXISTS hive_type_test_4;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_4', 'paa_cluster', 'hive_type_test_4');
select * from hive_type_test_4 order by id, name, m;
select count(*) from hive_type_test_4;

-- hive partition float
DROP EXTERNAL TABLE IF EXISTS hive_type_test_5;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_5', 'paa_cluster', 'hive_type_test_5');
select * from hive_type_test_5 order by id, name, m;
select count(*) from hive_type_test_5;

-- hive partition double
DROP EXTERNAL TABLE IF EXISTS hive_type_test_6;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_6', 'paa_cluster', 'hive_type_test_6');
select * from hive_type_test_6 order by id, name, m;
select count(*) from hive_type_test_6;

-- hive partition string
DROP EXTERNAL TABLE IF EXISTS hive_type_test_7;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_7', 'paa_cluster', 'hive_type_test_7');
select * from hive_type_test_7 order by id, name, m;
select count(*) from hive_type_test_7;

-- hive partition date
DROP EXTERNAL TABLE IF EXISTS hive_type_test_8;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_8', 'paa_cluster', 'hive_type_test_8');
select * from hive_type_test_8 order by id, name, m;
select count(*) from hive_type_test_8;

-- hive partition varchar
DROP EXTERNAL TABLE IF EXISTS hive_type_test_9;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_9', 'paa_cluster', 'hive_type_test_9');
select * from hive_type_test_9 order by id, name, m;
select count(*) from hive_type_test_9;

-- hive partition decimal
DROP EXTERNAL TABLE IF EXISTS hive_type_test_10;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_type_test_10', 'paa_cluster', 'hive_type_test_10');
select * from hive_type_test_10 order by id, name, m;
select count(*) from hive_type_test_10;

-- hive simple table
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_1', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id;
select * from hive_test_1 where id=1;
select count(*) from hive_test_1 where id=1;
select count(*) from hive_test_1;

-- hive partition and transaction table
DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_2', 'paa_cluster', 'hive_test_2');

select * from hive_test_2 order by id, name, name2, name3;
select * from hive_test_2 where name3='a' order by id, name, name2, name3;
select count(*) from hive_test_2;
select count(*) from hive_test_2 where name3='b';
select count(*) from hive_test_2 where name3='c';

-- hive partiton key is int type
DROP EXTERNAL TABLE IF EXISTS hive_test_3;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_3', 'paa_cluster', 'hive_test_3');
select * from hive_test_3 order by id, name, m;
select * from hive_test_3 where m=1 order by id;
select count(*) from hive_test_3;
select count(*) from hive_test_3 where m=2;
select count(*) from hive_test_3 where m=3;

-- hive partiton key is int and char type
DROP EXTERNAL TABLE IF EXISTS hive_test_4;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_4', 'paa_cluster', 'hive_test_4');
select * from hive_test_4 order by id, name, m, n;
select count(*) from hive_test_4;
select * from hive_test_4 where m=2 order by id, name, m, n;
select * from hive_test_4 where n='cc' order by id, name, m, n;

-- hive partiton key is int and char and date type
DROP EXTERNAL TABLE IF EXISTS hive_test_5;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_5', 'paa_cluster', 'hive_test_5');
select * from hive_test_5 order by id, name, m, n, o;
select count(*) from hive_test_5;

-- hive partiton key is int and string and float type
DROP EXTERNAL TABLE IF EXISTS hive_test_6;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_6', 'paa_cluster', 'hive_test_6');
select * from hive_test_6 order by id, name, m, n, o;
select count(*) from hive_test_6;

-- hive partiton key is int and string and decimal and tinyint type
DROP EXTERNAL TABLE IF EXISTS hive_test_7;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_7', 'paa_cluster', 'hive_test_7');
select * from hive_test_7 order by id, name, m, n, o, p;
select count(*) from hive_test_7;

-- hive partiton key is int and string and decimal and tinyint type
DROP EXTERNAL TABLE IF EXISTS hive_test_8;
select sync_hive_table('hive_cluster', 'hive_parquet_load_data_test', 'hive_test_8', 'paa_cluster', 'hive_test_8');
select * from hive_test_8 order by id, name, m, n, o, p, q, s;
select count(*) from hive_test_8;

DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_3;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_5;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_7;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_8;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_9;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_10;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_3;
DROP EXTERNAL TABLE IF EXISTS hive_test_4;
DROP EXTERNAL TABLE IF EXISTS hive_test_5;
DROP EXTERNAL TABLE IF EXISTS hive_test_6;
DROP EXTERNAL TABLE IF EXISTS hive_test_7;
DROP EXTERNAL TABLE IF EXISTS hive_test_8;
