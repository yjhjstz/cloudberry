-- read simple table
DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;
CREATE READABLE EXTERNAL TABLE hive_simple_test_1(a int, b text) LOCATION('gphdfs://user/hive/warehouse/hive_specify_partition_load_data_test.db/hive_simple_test_1/ hdfs_cluster_name=paa_cluster partition_value=') FORMAT 'orc';
select * from hive_simple_test_1 order by a;
DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;
CREATE READABLE EXTERNAL TABLE hive_simple_test_1(a int, b text) LOCATION('gphdfs://user/hive/warehouse/hive_specify_partition_load_data_test.db/hive_simple_test_1/ hdfs_cluster_name=paa_cluster partition_value=aa') FORMAT 'orc';
select * from hive_simple_test_1 order by a;
DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;
CREATE READABLE EXTERNAL TABLE hive_simple_test_1(a int, b text) LOCATION('gphdfs://user/hive/warehouse/hive_specify_partition_load_data_test.db/hive_simple_test_1/ hdfs_cluster_name=paa_cluster partition_value') FORMAT 'orc';
select * from hive_simple_test_1 order by a;
DROP EXTERNAL TABLE IF EXISTS hive_simple_test_1;

-- read partition table
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_type_test_1', '1', 'paa_cluster', 'hive_type_test_1', 'false');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_type_test_1', '2', 'paa_cluster', 'hive_type_test_1');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_type_test_1', '3', 'paa_cluster', 'hive_type_test_1');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_type_test_1', '3', 'paa_cluster', 'hive_type_test_1');
select * from hive_type_test_1 order by id, name, m;
select count(*) from hive_type_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_type_test_1;

-- read multi partition key
DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '1', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '2', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '3', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '4', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '5', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '6', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_1;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_1', '7', 'paa_cluster', 'hive_test_1');
select * from hive_test_1 order by id, name, m, n, o, p;
select count(*) from hive_test_1;
DROP EXTERNAL TABLE IF EXISTS hive_test_1;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_2', 'aa', 'paa_cluster', 'hive_test_2');
select * from hive_test_2 order by id, name, m, n, o, p;
select count(*) from hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_2', 'bb', 'paa_cluster', 'hive_test_2');
select * from hive_test_2 order by id, name, m, n, o, p;
select count(*) from hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_2', 'cc', 'paa_cluster', 'hive_test_2');
select * from hive_test_2 order by id, name, m, n, o, p;
select count(*) from hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;

DROP EXTERNAL TABLE IF EXISTS hive_test_2;
select sync_hive_partition_table('hive_cluster', 'hive_specify_partition_load_data_test', 'hive_test_2', '7', 'paa_cluster', 'hive_test_2');
select * from hive_test_2 order by id, name, m, n, o, p;
select count(*) from hive_test_2;
DROP EXTERNAL TABLE IF EXISTS hive_test_2;
