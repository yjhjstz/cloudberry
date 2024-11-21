SET datalake.external_table_limit_segment_num = 0;
-- numeric
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  col numeric(2,1),
  col2 numeric(9,6),
  col3 numeric(18,3),
  col4 numeric(38,3),
  col5 numeric(18,17),
  col6 numeric(38,37)
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/numeric/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY col, col2, col3, col4, col5;

-- date
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a date
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coldate_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

-- col null
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a int, 
  b text, 
  c int, 
  d text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/colnull_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a, b, c, d;

-- col timestamp
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a timestamp
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coltimestamp_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

SET datalake.external_table_limit_segment_num = 1;

-- numeric
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  col numeric(2,1),
  col2 numeric(9,6),
  col3 numeric(18,3),
  col4 numeric(38,3),
  col5 numeric(18,17),
  col6 numeric(38,37)
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/numeric/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY col, col2, col3, col4, col5;

-- date
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a date
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coldate_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

-- col null
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a int, 
  b text, 
  c int, 
  d text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/colnull_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a, b, c, d;

-- col timestamp
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a timestamp
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coltimestamp_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

SET datalake.external_table_limit_segment_num = 2;

-- numeric
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  col numeric(2,1),
  col2 numeric(9,6),
  col3 numeric(18,3),
  col4 numeric(38,3),
  col5 numeric(18,17),
  col6 numeric(38,37)
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/numeric/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY col, col2, col3, col4, col5;

-- date
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a date
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coldate_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

-- col null
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a int, 
  b text, 
  c int, 
  d text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/colnull_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a, b, c, d;

-- col timestamp
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a timestamp
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coltimestamp_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

SET datalake.external_table_limit_segment_num = 3;

-- numeric
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  col numeric(2,1),
  col2 numeric(9,6),
  col3 numeric(18,3),
  col4 numeric(38,3),
  col5 numeric(18,17),
  col6 numeric(38,37)
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/numeric/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY col, col2, col3, col4, col5;

-- date
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a date
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coldate_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;

-- col null
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a int, 
  b text, 
  c int, 
  d text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/colnull_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a, b, c, d;

-- col timestamp
SET datestyle = ISO, MDY;
DROP FOREIGN TABLE IF EXISTS readtable_table;
CREATE FOREIGN TABLE readtable_table (
  a timestamp
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_parquet/parquet_v1/coltimestamp_table/', enableCache 'false', format 'parquet');
SELECT * FROM readtable_table ORDER BY a;
