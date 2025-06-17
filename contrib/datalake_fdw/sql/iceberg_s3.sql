DROP FOREIGN TABLE IF EXISTS iceberg_simple;
CREATE FOREIGN TABLE iceberg_simple (
id int,
name text
)
server oss_server
OPTIONS (filePath '/ossext-ci-test/warehouse/iceberg/warehouse/', catalog_type 's3', server_name 's3_cluster', table_identifier 'default.simple_table', format 'iceberg');
select count(*) from iceberg_simple;

DROP FOREIGN TABLE IF EXISTS iceberg_partitioned_table;
CREATE FOREIGN TABLE iceberg_partitioned_table (
id int,
name text,
age int,
department text,
create_date date
)
server oss_server
OPTIONS (filePath '/ossext-ci-test/warehouse/iceberg/warehouse/', catalog_type 's3', server_name 's3_cluster', table_identifier 'default.partitioned_table', format 'iceberg');
select count(*) from iceberg_partitioned_table;


DROP FOREIGN TABLE IF EXISTS iceberg_partitioned_table2;
CREATE FOREIGN TABLE iceberg_partitioned_table2 (
id int,
name text,
age int,
department text,
create_date date
)
server oss_server
OPTIONS (filePath '/ossext-ci-test/warehouse/iceberg/warehouse/', catalog_type 's3', server_name 's3_cluster', table_identifier 'testdb.partitioned_table', format 'iceberg');
select count(*) from iceberg_partitioned_table2;