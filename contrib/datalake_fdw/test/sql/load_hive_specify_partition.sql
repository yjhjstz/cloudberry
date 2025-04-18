CREATE DATABASE IF NOT EXISTS hive_specify_partition_load_data_test;

USE hive_specify_partition_load_data_test;
SET hive.exec.max.dynamic.partitions.pernode = 1000;
SET hive.support.concurrency = true;
SET hive.enforce.bucketing = true;
SET hive.exec.dynamic.partition.mode = nonstrict;
SET hive.txn.manager = org.apache.hadoop.hive.ql.lockmgr.DbTxnManager;
SET hive.stats.autogather=false;
SET hive.exec.mode.local.auto=true;

drop table if exists hive_simple_test_1;
CREATE TABLE hive_simple_test_1
(
    id  int,
    name string
)
STORED AS ORC;
INSERT INTO TABLE hive_simple_test_1 values(1, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(2, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(3, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(4, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(5, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(NULL, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(7, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(8, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(9, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(10, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(6, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(7, NULL);
INSERT INTO TABLE hive_simple_test_1 values(8, NULL);
INSERT INTO TABLE hive_simple_test_1 values(9, NULL);
INSERT INTO TABLE hive_simple_test_1 values(NULL, NULL);
INSERT INTO TABLE hive_simple_test_1 values(6, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(7, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(8, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(9, "aaaaabbbb");
INSERT INTO TABLE hive_simple_test_1 values(10, "aaaaabbbb");

-- hive partition tinyint
drop table if exists hive_type_test_1;
CREATE TABLE hive_type_test_1
(
    id  int,
    name string
)
PARTITIONED BY
(
    m tinyint
)
STORED AS ORC;
INSERT INTO TABLE hive_type_test_1 PARTITION(m=1) values(1, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=1) values(2, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=1) values(3, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=1) values(4, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=1) values(5, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=2) values(NULL, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=2) values(7, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=2) values(8, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=2) values(9, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=2) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(6, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(7, NULL);
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(8, NULL);
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(9, NULL);
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(NULL, NULL);
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(6, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(7, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(8, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(9, "aaaaabbbb");
INSERT INTO TABLE hive_type_test_1 PARTITION(m=3) values(10, "aaaaabbbb");

-- hive partiton key is int and string and decimal and tinyint type
drop table if exists hive_test_1;
CREATE TABLE hive_test_1
(
    id  int,
    name string
)
PARTITIONED BY
(
    m int, n string, o decimal(20, 10), p tinyint
)
STORED AS ORC;
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=1, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=2, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=3, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=4, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=5, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=1, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=2, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="aa", o=1.1, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="bb", o=2.99, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=6, n="bb", o=9999.999, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=1.1000000000, p=1) values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=200.999999, p=0) values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="aa", o=1.99999, p=1) values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=1234567890.1234567891, p=1) values(6, "rewrwr3r2");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=999999999.9999999, p=0) values(7, "fdsfsf");
INSERT INTO TABLE hive_test_1 PARTITION(m=7, n="bb", o=1234567890.01, p=1) values(6, "rewrwr3r2");


drop table if exists hive_test_2;
CREATE TABLE hive_test_2
(
    id  int,
    name string
)
PARTITIONED BY
(
    m string, n string, o string, p string, q string, s string
)
STORED AS ORC;
INSERT INTO TABLE hive_test_2 PARTITION(m="7", n="aa", o="aa", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="aa", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="aa", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="aa", o="bb", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="cc", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="dd", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="7", n="aa", o="aa", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="aa", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="aa", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="aa", o="bb", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="cc", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="1", n="dd", o="dd", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="7", n="aa", o="aa", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="bb", p="aa", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="aa", p="cc", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="aa", o="bb", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="cc", q="dd", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="dd", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="7", n="aa", o="aa", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="aa", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="cc", n="dd", o="aa", p="aa", q="aa", s="aa") values(7, "fdsfsf");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="aa", o="bb", p="aa", q="aa", s="aa") values(10, "aaaaabbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="aa", n="bb", o="aa", p="aa", q="aa", s="aa") values(9, "bbbbcccc");
INSERT INTO TABLE hive_test_2 PARTITION(m="bb", n="cc", o="aa", p="cc", q="aa", s="aa") values(8, "ddddbbbb");
INSERT INTO TABLE hive_test_2 PARTITION(m="1", n="dd", o="dd", p="aa", q="aa", s="aa") values(7, "fdsfsf");
