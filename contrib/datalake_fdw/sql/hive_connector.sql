DROP SCHEMA IF EXISTS synctab CASCADE;
DROP SCHEMA IF EXISTS syncdb CASCADE;

CREATE SCHEMA synctab;
CREATE SCHEMA syncdb;

SET datestyle = ISO, MDY;

SELECT public.sync_hive_table('hive_cluster','mytestdb','text_default','paa_cluster', 'synctab.text_default', 'hive_server');

SELECT * FROM synctab.text_default order by a, b, c, d;

SELECT public.sync_hive_table('hive_cluster','mytestdb','text_custom','paa_cluster', 'synctab.text_custom', 'hive_server');

SELECT * FROM synctab.text_custom order by a, b, c, d;

SELECT public.sync_hive_table('hive_cluster','mytestdb','csv_default','paa_cluster', 'synctab.csv_default', 'hive_server');

SELECT * FROM synctab.csv_default order by a, b, c, d;

SELECT public.sync_hive_table('hive_cluster','mytestdb','csv_custom','paa_cluster', 'synctab.csv_custom', 'hive_server');

SELECT * FROM synctab.csv_custom order by a, b, c, d;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_orc_transactional','paa_cluster', 'synctab.empty_orc_transactional', 'hive_server');

SELECT * FROM synctab.empty_orc_transactional;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_orc','paa_cluster', 'synctab.empty_orc', 'hive_server');

SELECT * FROM synctab.empty_orc;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_parquet','paa_cluster', 'synctab.empty_parquet', 'hive_server');

SELECT * FROM synctab.empty_parquet;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_orc_partition','paa_cluster', 'synctab.empty_orc_partition', 'hive_server');

SELECT * FROM synctab.empty_orc_partition;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_parquet_partition','paa_cluster', 'synctab.empty_parquet_partition', 'hive_server');

SELECT * FROM synctab.empty_parquet_partition;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_avro','paa_cluster', 'synctab.empty_avro', 'hive_server');

SELECT * FROM synctab.empty_avro;

SELECT public.sync_hive_table('hive_cluster','mytestdb','empty_avro_partition','paa_cluster', 'synctab.empty_avro_partition', 'hive_server');

SELECT * FROM synctab.empty_avro_partition;

SELECT public.sync_hive_database('hive_cluster', 'mytestdb', 'paa_cluster', 'syncdb', 'hive_server');

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
