CREATE DATABASE IF NOT EXISTS hive_logerror_data_test;

USE hive_logerror_data_test;
SET hive.exec.max.dynamic.partitions.pernode = 2000;
SET hive.support.concurrency = true;
SET hive.enforce.bucketing = true;
SET hive.exec.dynamic.partition.mode = nonstrict;
SET hive.txn.manager = org.apache.hadoop.hive.ql.lockmgr.DbTxnManager;
SET hive.exec.compress.output=true;
SET mapred.output.compress=true;
SET mapred.output.compression.codec=org.apache.hadoop.io.compress.SnappyCodec;
SET hive.stats.autogather=false;
SET hive.exec.mode.local.auto=true;

drop table if exists hive_test_1;
CREATE TABLE hive_test_1
(
    id  int,
    name string
)
ROW FORMAT DELIMITED FIELDS TERMINATED BY '|';

INSERT INTO TABLE hive_test_1 values(1, "\\."), (2, '\0'), (3, 'aabbccdd\\.\0'), (4, '12\03\\.aabbccdd');
INSERT INTO TABLE hive_test_1 values(5, "\\.\0"), (6, '\\\0'), (7, '\0\\.'), (8, '\\aabbcc\0\\.'), (9, '\\0');

drop table if exists hive_test_2;
CREATE TABLE hive_test_2
(
    id  int,
    name string
)
PARTITIONED BY
(
    m string
)
STORED AS TEXTFILE;
INSERT INTO TABLE hive_test_2 PARTITION(m=1) values(1, "\\."), (2, '\0'), (3, 'aabbccdd\\.\0'), (4, '12\03\\.aabbccdd');
INSERT INTO TABLE hive_test_2 PARTITION(m=2) values(5, "\\.\0"), (6, '\\\0'), (7, '\0\\.'), (8, '\\aabbcc\0\\.'), (9, '\\0');
INSERT INTO TABLE hive_test_2 PARTITION(m=3) values(10, "\\.\0"), (11, '\\\0'), (12, '\0\\.'), (13, '\\aabbcc\0\\.'), (14, '\\0');



