-- convert hashdata 3X datalake grammar to hashdata lightning

-- start_ignore
DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw;

CREATE EXTENSION IF NOT EXISTS hive_connector;
CREATE EXTENSION IF NOT EXISTS datalake_fdw;
-- end_ignore

CREATE FOREIGN DATA WRAPPER datalake_fdw
  HANDLER datalake_fdw_handler
  VALIDATOR datalake_fdw_validator
  OPTIONS ( mpp_execute 'all segments' );

CREATE SERVER foreign_server
        FOREIGN DATA WRAPPER datalake_fdw
        OPTIONS (host 'obs.cn-north-4.myhuaweicloud.com', protocol 'huawei', isvirtual 'false',
        ishttps 'false');

CREATE USER MAPPING FOR gpadmin
        SERVER foreign_server
        OPTIONS (user 'gpadmin', accesskey 'J04WCCF5VQP6BAIQUFHP', secretkey 'jGDwttCct2b9b4rEf0hsLD7CeP9WubZuqqz90iQU');

-- start_ignore
-- oss
CREATE READABLE EXTERNAL TABLE orc_read (a int, b int)
LOCATION('oss://obs.cn-north-4.myhuaweicloud.com/ossext-ci-test/ext_orc/moreSmallFile/ oss_type=QS access_key_id=KGCPPHVCHRMZMFEAWLLC secret_access_key=0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs') FORMAT 'orc';
\d orc_read
select * from orc_read order by a,b;

CREATE READABLE EXTERNAL TABLE readtable_table (a date)
location('oss://obs.cn-north-4.myhuaweicloud.com/ossext-ci-test/ext_parquet/parquet_v1/coldate_table/ oss_type=QS access_key_id=KGCPPHVCHRMZMFEAWLLC secret_access_key=0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs') FORMAT 'parquet';
\d readtable_table
select * from readtable_table order by a;

CREATE READABLE EXTERNAL TABLE read_one_file(a int, b text)
LOCATION('oss://obs.cn-north-4.myhuaweicloud.com/ossext-ci-test/ext_text/onefile/ oss_type=QS access_key_id=KGCPPHVCHRMZMFEAWLLC secret_access_key=0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs') FORMAT 'text';
\d read_one_file
select * from read_one_file;

CREATE READABLE EXTERNAL TABLE avro_read (a int, b int)
LOCATION('oss://obs.cn-north-4.myhuaweicloud.com/ossext-ci-test/avro/avro_uncompress/oneSmallFile/ oss_type=QS access_key_id=KGCPPHVCHRMZMFEAWLLC secret_access_key=0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs') FORMAT 'avro';
\d avro_read
select * from avro_read;

CREATE READABLE EXTERNAL TABLE qsread_empty (a INT, b INT)
location('oss://obs.cn-north-4.myhuaweicloud.com/icg/read/emptyfile oss_type=QS') FORMAT 'csv';
\d qsread_empty

CREATE READABLE EXTERNAL TABLE read_one_file_2(a int, b text)
LOCATION('oss://obs.cn-north-4.myhuaweicloud.com/ossext-ci-test/ext_text/onefile/ oss_type=QS access_key_id=KGCPPHVCHRMZMFEAWLLC secret_access_key=0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs cache=true transactional=true') FORMAT 'text';
\d read_one_file_2
select * from read_one_file_2;

CREATE READABLE EXTERNAL TABLE read_one_file_3 (a INT, b INT)
location('oss://server-does-not-exist/icg/read/emptyfile oss_type=QS') FORMAT 'csv';

-- other options
CREATE READABLE EXTERNAL TABLE read_one_file_4 (a INT, b INT)
location('oss://obs.cn-north-4.myhuaweicloud.com/icg/read/emptyfile oss_type=QS')
FORMAT 'csv'
(DELIMITER ',' NEWLINE 'CRLF' NULL E'\x1b\x4e' ESCAPE E'\x1b') ENCODING 'UTF8';

-- with log errors
CREATE READABLE EXTERNAL TABLE read_one_file_5(a int, b text)
location('oss://obs.cn-north-4.myhuaweicloud.com/icg/read/emptyfile oss_type=QS')
FORMAT 'text' LOG ERRORS SEGMENT REJECT LIMIT 10 ROWS;
-- end_ignore

-- hdfs
DROP SERVER IF EXISTS foreign_server_2;
SELECT public.create_foreign_server('foreign_server_2', 'gpadmin', 'datalake_fdw', 'paa_cluster');

CREATE READABLE EXTERNAL TABLE read_two_file(a int, b text)
LOCATION('gphdfs://ci-test-data/avro/twofile hdfs_cluster_name=paa_cluster') FORMAT 'avro';
\d read_two_file

CREATE READABLE EXTERNAL TABLE read_more_file(a int, b text)
LOCATION('gphdfs://ci-test-data/orc/more_file hdfs_cluster_name=paa_cluster') FORMAT 'orc';
\d read_more_file

-- ftp
CREATE SERVER ftp_foreign_server
FOREIGN DATA WRAPPER datalake_fdw
OPTIONS (host '192.168.198.144', protocol 'ftp');

CREATE SERVER ftp_foreign_server
FOREIGN DATA WRAPPER datalake_fdw
OPTIONS (host '192.168.198.144', protocol 'ftp');

CREATE USER MAPPING FOR gpadmin
SERVER ftp_foreign_server
OPTIONS (user 'ftp', password 'ftp');

create readable external table ftp_test1 (a char(6), b char(6), c char(6))
location('ftp://192.168.198.144/orc/ ftp_username=ftp ftp_password=ftp') format 'orc';
\d ftp_test1

create readable external table ftp_test2 (a char(6), b char(6), c char(6))
location('ftp://192.168.198.144/custom/ ftp_username=ftp ftp_password=ftp') format 'text';
\d ftp_test2

-- clear
-- start_ignore
DROP FOREIGN TABLE IF EXISTS orc_read;
DROP FOREIGN TABLE IF EXISTS readtable_table;
DROP FOREIGN TABLE IF EXISTS read_one_file;
DROP FOREIGN TABLE IF EXISTS avro_read;
DROP FOREIGN TABLE IF EXISTS qsread_empty;
DROP FOREIGN TABLE IF EXISTS read_one_file_2;
DROP FOREIGN TABLE IF EXISTS read_one_file_4;
DROP FOREIGN TABLE IF EXISTS read_one_file_5;
DROP FOREIGN TABLE IF EXISTS read_two_file;
DROP FOREIGN TABLE IF EXISTS read_more_file;
DROP FOREIGN TABLE IF EXISTS ftp_test1;
DROP FOREIGN TABLE IF EXISTS ftp_test2;

DROP USER MAPPING IF EXISTS FOR CURRENT_USER SERVER foreign_server;
DROP SERVER IF EXISTS foreign_server CASCADE;
DROP SERVER IF EXISTS foreign_server_2 CASCADE;
DROP SERVER IF EXISTS ftp_foreign_server CASCADE;
DROP FOREIGN DATA WRAPPER IF EXISTS datalake_fdw CASCADE;
-- end_ignore
