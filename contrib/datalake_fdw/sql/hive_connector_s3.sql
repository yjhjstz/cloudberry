DROP SCHEMA IF EXISTS synctab CASCADE;
DROP SCHEMA IF EXISTS syncdb CASCADE;

CREATE SCHEMA synctab;
CREATE SCHEMA syncdb;

SET datestyle = ISO, MDY;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','text_default','synctab.text_default', 'hive_s3_server');

SELECT * FROM synctab.text_default order by a, b, c, d;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','text_custom','synctab.text_custom', 'hive_s3_server');

SELECT * FROM synctab.text_custom order by a, b, c, d;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','csv_default','synctab.csv_default', 'hive_s3_server');

SELECT * FROM synctab.csv_default order by a, b, c, d;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','csv_custom','synctab.csv_custom', 'hive_s3_server');

SELECT * FROM synctab.csv_custom order by a, b, c, d;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_orc_transactional','synctab.empty_orc_transactional', 'hive_s3_server');

SELECT * FROM synctab.empty_orc_transactional;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_orc','synctab.empty_orc', 'hive_s3_server');

SELECT * FROM synctab.empty_orc;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_parquet','synctab.empty_parquet', 'hive_s3_server');

SELECT * FROM synctab.empty_parquet;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_orc_partition','synctab.empty_orc_partition', 'hive_s3_server');

SELECT * FROM synctab.empty_orc_partition;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_parquet_partition','synctab.empty_parquet_partition', 'hive_s3_server');

SELECT * FROM synctab.empty_parquet_partition;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_avro','synctab.empty_avro', 'hive_s3_server');

SELECT * FROM synctab.empty_avro;

SELECT public.sync_hive_table_s3('hive_s3_cluster','mytestdb','empty_avro_partition','synctab.empty_avro_partition', 'hive_s3_server');

SELECT * FROM synctab.empty_avro_partition;

SELECT public.sync_hive_database_s3('hive_s3_cluster', 'mytestdb', 'syncdb', 'hive_s3_server');

SELECT * FROM syncdb.text_default order by a, b, c, d;

SELECT * FROM syncdb.text_custom order by a, b, c, d;

SELECT * FROM syncdb.csv_default order by a, b, c, d;

SELECT * FROM syncdb.csv_custom order by a, b, c, d;

SELECT * FROM syncdb.empty_orc_transactional;

SELECT * FROM syncdb.empty_orc;

SELECT * FROM syncdb.empty_parquet;

SELECT * FROM syncdb.empty_orc_partition;

SELECT * FROM syncdb.empty_parquet_partition;

SELECT * FROM syncdb.empty_avro;

SELECT * FROM syncdb.empty_avro_partition;

-- Analyze empty tables
ANALYZE synctab.text_default;
ANALYZE synctab.text_custom;
ANALYZE synctab.csv_default;
ANALYZE synctab.csv_custom;
ANALYZE synctab.empty_orc_transactional;
ANALYZE synctab.empty_orc;
ANALYZE synctab.empty_parquet;
ANALYZE synctab.empty_orc_partition;
ANALYZE synctab.empty_parquet_partition;
ANALYZE synctab.empty_avro;
ANALYZE synctab.empty_avro_partition;

DROP FOREIGN TABLE IF EXISTS synctab.text_default;
DROP FOREIGN TABLE IF EXISTS synctab.text_custom;
DROP FOREIGN TABLE IF EXISTS synctab.csv_default;
DROP FOREIGN TABLE IF EXISTS synctab.csv_custom;
DROP FOREIGN TABLE IF EXISTS synctab.empty_orc_transactional;
DROP FOREIGN TABLE IF EXISTS synctab.empty_orc;
DROP FOREIGN TABLE IF EXISTS synctab.empty_parquet;
DROP FOREIGN TABLE IF EXISTS synctab.empty_orc_partition;
DROP FOREIGN TABLE IF EXISTS synctab.empty_parquet_partition;
DROP FOREIGN TABLE IF EXISTS synctab.empty_avro;
DROP FOREIGN TABLE IF EXISTS synctab.empty_avro_partition;

DROP FOREIGN TABLE IF EXISTS syncdb.text_default;
DROP FOREIGN TABLE IF EXISTS syncdb.text_custom;
DROP FOREIGN TABLE IF EXISTS syncdb.csv_default;
DROP FOREIGN TABLE IF EXISTS syncdb.csv_custom;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_orc_transactional;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_orc;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_parquet;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_orc_partition;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_parquet_partition;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_avro;
DROP FOREIGN TABLE IF EXISTS syncdb.empty_avro_partition;

DROP SCHEMA IF EXISTS synctab;
DROP SCHEMA IF EXISTS syncdb;
