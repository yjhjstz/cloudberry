-- create simple function test
-- write simple file
DROP EXTERNAL TABLE IF EXISTS one_file;
CREATE WRITABLE EXTERNAL TABLE one_file(a int, b text) LOCATION('gphdfs://ci-test-data/text/onefile hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into one_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS one_file2;
CREATE WRITABLE EXTERNAL TABLE one_file2(a int, b text) LOCATION('gphdfs://ci-test-data/text/onefile2 hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into one_file2 values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS two_file;
CREATE WRITABLE EXTERNAL TABLE two_file(a int, b text) LOCATION('gphdfs://ci-test-data/text/twofile hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into two_file values(1, 'aaaaabbbb');
insert into two_file values(1, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS more_file;
CREATE WRITABLE EXTERNAL TABLE more_file(a int, b text) LOCATION('gphdfs://ci-test-data/text/more_file hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into more_file values(1, 'aaaaabbbb');
insert into more_file values(2, 'aaaaabbbb');
insert into more_file values(3, 'aaaaabbbb');
insert into more_file values(4, 'aaaaabbbb');
insert into more_file values(5, 'aaaaabbbb');
insert into more_file values(6, 'aaaaabbbb');
insert into more_file values(7, 'aaaaabbbb');
insert into more_file values(8, 'aaaaabbbb');
insert into more_file values(9, 'aaaaabbbb');
insert into more_file values(10, 'aaaaabbbb');
insert into more_file values(11, 'aaaaabbbb');
insert into more_file values(12, 'aaaaabbbb');
insert into more_file values(13, 'aaaaabbbb');
insert into more_file values(14, 'aaaaabbbb');
insert into more_file values(15, 'aaaaabbbb');
insert into more_file values(16, 'aaaaabbbb');
insert into more_file values(17, 'aaaaabbbb');
insert into more_file values(18, 'aaaaabbbb');
insert into more_file values(19, 'aaaaabbbb');
insert into more_file values(20, 'aaaaabbbb');

-- value have NULL
DROP EXTERNAL TABLE IF EXISTS more_file2;
CREATE WRITABLE EXTERNAL TABLE more_file2(a int, b text) LOCATION('gphdfs://ci-test-data/text/more_file2 hdfs_cluster_name=paa_cluster') FORMAT 'text';
insert into more_file2 values(1, 'aaaaabbbb');
insert into more_file2 values(2, 'aaaaabbbb');
insert into more_file2 values(3, 'aaaaabbbb');
insert into more_file2 values(4, 'aaaaabbbb');
insert into more_file2 values(5, 'NULL');
insert into more_file2 values(6, 'aaaaabbbb');
insert into more_file2 values(7, 'aaaaabbbb');
insert into more_file2 values(8, 'aaaaabbbb');
insert into more_file2 values(9, 'NULL');
insert into more_file2 values(10, 'aaaaabbbb');
insert into more_file2 values(11, 'aaaaabbbb');
insert into more_file2 values(12, 'aaaaabbbb');
insert into more_file2 values(13, 'NULL');
insert into more_file2 values(14, 'NULL');
insert into more_file2 values(15, 'aaaaabbbb');
insert into more_file2 values(NULL, 'aaaaabbbb');
insert into more_file2 values(17, 'aaaaabbbb');
insert into more_file2 values(NULL, 'aaaaabbbb');
insert into more_file2 values(19, 'aaaaabbbb');
insert into more_file2 values(20, 'aaaaabbbb');

DROP EXTERNAL TABLE IF EXISTS split_file;
CREATE WRITABLE EXTERNAL TABLE split_file(a text, b text) LOCATION('gphdfs://ci-test-data/text/split_file hdfs_cluster_name=paa_cluster filesizelimit=2') FORMAT 'text';
insert into split_file select md5(random()::text), md5(random()::text) from generate_series(1, 500000);

DROP EXTERNAL TABLE IF EXISTS split_file_gz;
CREATE WRITABLE EXTERNAL TABLE split_file_gz(a text, b text) LOCATION('gphdfs://ci-test-data/text/split_file_gz hdfs_cluster_name=paa_cluster filesizelimit=2 compression=gzip') FORMAT 'text';
insert into split_file_gz select md5(random()::text), md5(random()::text) from generate_series(1, 500000);

DROP EXTERNAL TABLE IF EXISTS split_file_zip;
CREATE WRITABLE EXTERNAL TABLE split_file_zip(a text, b text) LOCATION('gphdfs://ci-test-data/text/split_file_zip hdfs_cluster_name=paa_cluster filesizelimit=2 compression=zip') FORMAT 'text';
insert into split_file_zip select md5(random()::text), md5(random()::text) from generate_series(1, 500000);