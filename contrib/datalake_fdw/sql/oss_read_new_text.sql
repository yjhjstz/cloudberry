set datalake.external_table_new_text = true;
DROP FOREIGN TABLE IF EXISTS read_one_file;
CREATE FOREIGN TABLE read_one_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile/', enableCache 'false', format 'text');
select * from read_one_file order by a;
DROP FOREIGN TABLE IF EXISTS read_one_file;

DROP FOREIGN TABLE IF EXISTS read_one_file2;
CREATE FOREIGN TABLE read_one_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
select * from read_one_file2 order by a;
DROP FOREIGN TABLE IF EXISTS read_one_file2;

DROP FOREIGN TABLE IF EXISTS read_two_file;
CREATE FOREIGN TABLE read_two_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/twofile/', enableCache 'false', format 'text');
SELECT * FROM read_two_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_two_file;

DROP FOREIGN TABLE IF EXISTS read_more_file;
CREATE FOREIGN TABLE read_more_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file/', enableCache 'false', format 'text');
SELECT * FROM read_more_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file;

DROP FOREIGN TABLE IF EXISTS read_more_file2;
CREATE FOREIGN TABLE read_more_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file2/', enableCache 'false', format 'text');
SELECT * FROM read_more_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file2;

DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;
CREATE FOREIGN TABLE read_invalid_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/invalid_path/', enableCache 'false', format 'text');
SELECT * FROM read_invalid_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;

DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
CREATE FOREIGN TABLE read_empty_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/empty_path/', enableCache 'false', format 'text');
SELECT * FROM read_empty_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_empty_file_path;

SET datalake.external_table_limit_segment_num = 1;
set datalake.external_table_new_text = true;
DROP FOREIGN TABLE IF EXISTS read_one_file;
CREATE FOREIGN TABLE read_one_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile/', enableCache 'false', format 'text');
SELECT * FROM read_one_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file;

DROP FOREIGN TABLE IF EXISTS read_one_file2;
CREATE FOREIGN TABLE read_one_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file2;

DROP FOREIGN TABLE IF EXISTS read_one_file3;
CREATE FOREIGN TABLE read_one_file3 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file3 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file3;

DROP FOREIGN TABLE IF EXISTS read_two_file;
CREATE FOREIGN TABLE read_two_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/twofile/', enableCache 'false', format 'text');
SELECT * FROM read_two_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_two_file;

DROP FOREIGN TABLE IF EXISTS read_more_file;
CREATE FOREIGN TABLE read_more_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file/', enableCache 'false', format 'text');
SELECT * FROM read_more_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file;

DROP FOREIGN TABLE IF EXISTS read_more_file2;
CREATE FOREIGN TABLE read_more_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file2/', enableCache 'false', format 'text');
SELECT * FROM read_more_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file2;

DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;
CREATE FOREIGN TABLE read_invalid_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/invalid_path/', enableCache 'false', format 'text');
SELECT * FROM read_invalid_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;

DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
CREATE FOREIGN TABLE read_empty_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/empty_path/', enableCache 'false', format 'text');
SELECT * FROM read_empty_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_empty_file_path;

SET datalake.external_table_limit_segment_num = 2;
set datalake.external_table_new_text = true;
DROP FOREIGN TABLE IF EXISTS read_one_file;
CREATE FOREIGN TABLE read_one_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile/', enableCache 'false', format 'text');
SELECT * FROM read_one_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file;

DROP FOREIGN TABLE IF EXISTS read_one_file2;
CREATE FOREIGN TABLE read_one_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file2;

DROP FOREIGN TABLE IF EXISTS read_one_file3;
CREATE FOREIGN TABLE read_one_file3 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file3 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file3;

DROP FOREIGN TABLE IF EXISTS read_two_file;
CREATE FOREIGN TABLE read_two_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/twofile/', enableCache 'false', format 'text');
SELECT * FROM read_two_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_two_file;

DROP FOREIGN TABLE IF EXISTS read_more_file;
CREATE FOREIGN TABLE read_more_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file/', enableCache 'false', format 'text');
SELECT * FROM read_more_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file;

DROP FOREIGN TABLE IF EXISTS read_more_file2;
CREATE FOREIGN TABLE read_more_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file2/', enableCache 'false', format 'text');
SELECT * FROM read_more_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file2;

DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;
CREATE FOREIGN TABLE read_invalid_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/invalid_path/', enableCache 'false', format 'text');
SELECT * FROM read_invalid_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;

DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
CREATE FOREIGN TABLE read_empty_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/empty_path/', enableCache 'false', format 'text');
SELECT * FROM read_empty_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_empty_file_path;

SET datalake.external_table_limit_segment_num = 3;
set datalake.external_table_new_text = true;
DROP FOREIGN TABLE IF EXISTS read_one_file;
CREATE FOREIGN TABLE read_one_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile/', enableCache 'false', format 'text');
SELECT * FROM read_one_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file;

DROP FOREIGN TABLE IF EXISTS read_one_file2;
CREATE FOREIGN TABLE read_one_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file2;

DROP FOREIGN TABLE IF EXISTS read_one_file3;
CREATE FOREIGN TABLE read_one_file3 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file3 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file3;

DROP FOREIGN TABLE IF EXISTS read_two_file;
CREATE FOREIGN TABLE read_two_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/twofile/', enableCache 'false', format 'text');
SELECT * FROM read_two_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_two_file;

DROP FOREIGN TABLE IF EXISTS read_more_file;
CREATE FOREIGN TABLE read_more_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file/', enableCache 'false', format 'text');
SELECT * FROM read_more_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file;

DROP FOREIGN TABLE IF EXISTS read_more_file2;
CREATE FOREIGN TABLE read_more_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file2/', enableCache 'false', format 'text');
SELECT * FROM read_more_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file2;

DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;
CREATE FOREIGN TABLE read_invalid_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/invalid_path/', enableCache 'false', format 'text');
SELECT * FROM read_invalid_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;

DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
CREATE FOREIGN TABLE read_empty_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/empty_path/', enableCache 'false', format 'text');
SELECT * FROM read_empty_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_empty_file_path;

SET datalake.external_table_limit_segment_num = 500;
set datalake.external_table_new_text = true;
DROP FOREIGN TABLE IF EXISTS read_one_file;
CREATE FOREIGN TABLE read_one_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile/', enableCache 'false', format 'text');
SELECT * FROM read_one_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file;

DROP FOREIGN TABLE IF EXISTS read_one_file2;
CREATE FOREIGN TABLE read_one_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file2;

DROP FOREIGN TABLE IF EXISTS read_one_file3;
CREATE FOREIGN TABLE read_one_file3 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/onefile2/', enableCache 'false', format 'text');
SELECT * FROM read_one_file3 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_one_file3;

DROP FOREIGN TABLE IF EXISTS read_two_file;
CREATE FOREIGN TABLE read_two_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/twofile/', enableCache 'false', format 'text');
SELECT * FROM read_two_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_two_file;

DROP FOREIGN TABLE IF EXISTS read_more_file;
CREATE FOREIGN TABLE read_more_file (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file/', enableCache 'false', format 'text');
SELECT * FROM read_more_file ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file;

DROP FOREIGN TABLE IF EXISTS read_more_file2;
CREATE FOREIGN TABLE read_more_file2 (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/more_file2/', enableCache 'false', format 'text');
SELECT * FROM read_more_file2 ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_more_file2;

DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;
CREATE FOREIGN TABLE read_invalid_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/invalid_path/', enableCache 'false', format 'text');
SELECT * FROM read_invalid_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_invalid_file_path;

DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
CREATE FOREIGN TABLE read_empty_file_path (
  a int,
  b text
)
SERVER oss_server
OPTIONS (filePath '/ossext-ci-test/ext_text/empty_path/', enableCache 'false', format 'text');
SELECT * FROM read_empty_file_path ORDER BY a;
DROP FOREIGN TABLE IF EXISTS read_empty_file_path;
