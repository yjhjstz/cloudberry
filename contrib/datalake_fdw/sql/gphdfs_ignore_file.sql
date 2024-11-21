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
SERVER hive_server 
OPTIONS (filepath '/.custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER hive_server 
OPTIONS (filepath '/custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER hive_server 
OPTIONS (filepath '/ignore/text/', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;

DROP EXTERNAL TABLE IF EXISTS readtable;
CREATE FOREIGN TABLE readtable(
    a text
)
SERVER hive_server 
OPTIONS (filepath '/ignore/text/.ignore/custom_small_file_lf.txt', format 'text', "null" E'\\N', escape E'\\', newline 'LF', encoding 'UTF-8');
select count(*) from readtable;
