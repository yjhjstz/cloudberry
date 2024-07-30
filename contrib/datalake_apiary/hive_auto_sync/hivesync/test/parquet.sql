

CREATE TABLE parquet_test_1
(
    id  int,
    name string
)
PARTITIONED BY
(
    m tinyint
)
STORED AS parquet;

INSERT INTO TABLE parquet_test_1 PARTITION(m) values
(1, "aaaaabbbb", 1),
(2, "aaaaabbbb", 1),
(3, "bbaaabbbb", 1),
(1, "aaaaabbbb", 2),
(2, "bbaaabbbb", 2),
(3, "aaaaabbbb", 2);

DROP TABLE parquet_test_1;

CREATE TABLE parquet_test_1
(
    c0 string,
    c1 decimal(20, 10),
    c2 int
)
PARTITIONED BY
(
    m date,
    n int
)
STORED AS parquet;

INSERT INTO TABLE parquet_test_1 PARTITION(m='2020-01-01', n=1) values
("aaaaabbbb", 1.1234567890, 1),
("aaaaabbbb", 2.1234567890, 2);

CREATE TABLE parquet_test_8
(
    id  int,
    name string
)
PARTITIONED BY
(
    m date
)
STORED AS parquet;

INSERT INTO TABLE parquet_test_8 PARTITION(m='0009-01-01') values (1, "aaaaabbbb");
INSERT INTO TABLE parquet_test_8 PARTITION(m='0010-01-01') values (2, "aaaaabbbb");
INSERT INTO TABLE parquet_test_8 PARTITION(m='1011-01-01') values (3, "aaaaabbbb");

ALTER TABLE parquet_test_8 ADD PARTITION(m='0030-01-01');
ALTER TABLE parquet_test_8 DROP PARTITION(m='0009-01-01');
INSERT INTO TABLE parquet_test_8 PARTITION(m='0009-01-01') values (4, "aaaaabbbb");
INSERT OVERWRITE TABLE parquet_test_8 PARTITION(m='0009-01-01') values (0, "aaaaabbbb");

ALTER TABLE parquet_test_8 ADD COLUMNS(a decimal(20, 10));
INSERT INTO parquet_test_8 PARTITION(m='0009-01-01') values (4, "aaaaabbbb", 123.12345);

CREATE TABLE parquet_test_bucket (
  id  int,
  name               string,
  name2              string
) CLUSTERED BY (id) INTO 5 BUCKETS
STORED AS parquet
TBLPROPERTIES ("transactional"="false");

INSERT INTO TABLE parquet_test_bucket values(1, "a", "a"), (2, NULL, "a"), (3, "a", "a"), (4, "a", NULL), (5, "a", "a");

CREATE TABLE parquet_test_partition
(
    id  int,
    name string
)
PARTITIONED BY
(
    m int, n string, o decimal(20, 10), p tinyint, q smallint, s bigint, t float, u date, v varchar(10)
)
STORED AS PARQUET;

INSERT INTO TABLE parquet_test_partition PARTITION(m=7, n="aa", o=1.10000000, p=1, q=1, s=1, t=1.1, u='2020-01-01', v="vv") VALUES (10, "aaaaabbbb");
INSERT INTO TABLE parquet_test_partition PARTITION(m=7, n="aa", o=200.999999, p=0, q=2, s=3, t=200.9, u='2021-01-01', v="aa") VALUES (9, "bbbbcccc");
INSERT INTO TABLE parquet_test_partition PARTITION(m=7, n="aa", o=1.99999, p=1, q=4, s=5, t=1.9, u='2022-01-01', v="bb") VALUES (8, "ddddbbbb");
INSERT INTO TABLE parquet_test_partition PARTITION(m=7, n="bb", o=999999999.9999999, p=0, q=22, s=3, t=99.99, u='2023-01-01', v="cc") VALUES (7, "fdsfsf");
INSERT INTO TABLE parquet_test_partition PARTITION(m=7, n="bb", o=1234567890.1234567891, p=1, q=2, s=3, t=12345.123, u='2024-01-01', v="dd") VALUES (6, "rewrwr3r2");


CREATE TABLE normal_parquet
(
a tinyint,
b smallint,
c int,
d bigint,
e float,
f double,
g string,
h timestamp,
i date,
j char(20),
k varchar(20),
l decimal(20, 10)
) stored as parquet;

INSERT INTO normal_parquet VALUES (1, 1, 1, 1, 1, 1, '1', '2020-01-01 01:01:01', '2020-01-01', '1', '1', 10.01),
(2, 2, 2, 2, 2, 2, '2', '2020-02-02 02:02:02', '2020-02-01', '2', '2', 11.01),
(3, 3, 3, 3, 3, 3, '3', '2020-03-03 03:03:03', '2020-03-01', '3', '3', 12.01);
