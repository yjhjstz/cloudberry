

CREATE TABLE text_test_1
(
    id  int,
    name string
)
PARTITIONED BY
(
    m tinyint
)
STORED AS TEXTFILE;

INSERT INTO TABLE text_test_1 PARTITION(m) values
(1, "aaaaabbbb", 1),
(2, "aaaaabbbb", 1),
(3, "bbaaabbbb", 1),
(1, "aaaaabbbb", 2),
(2, "bbaaabbbb", 2),
(3, "aaaaabbbb", 2);

DROP TABLE text_test_1;

CREATE TABLE text_test_1
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
STORED AS text;

INSERT INTO TABLE text_test_1 PARTITION(m='2020-01-01', n=1) values
("aaaaabbbb", 1.1234567890, 1),
("aaaaabbbb", 2.1234567890, 2);


CREATE TABLE text_test_8
(
    id  int,
    name string
)
PARTITIONED BY
(
    m date
)
STORED AS TEXTFILE;

INSERT INTO TABLE text_test_8 PARTITION(m='0009-01-01') values (1, "aaaaabbbb");
INSERT INTO TABLE text_test_8 PARTITION(m='0010-01-01') values (2, "aaaaabbbb");
INSERT INTO TABLE text_test_8 PARTITION(m='1011-01-01') values (3, "aaaaabbbb");

ALTER TABLE text_test_8 ADD PARTITION(m='0030-01-01');
ALTER TABLE text_test_8 DROP PARTITION(m='0009-01-01');
INSERT INTO TABLE text_test_8 PARTITION(m='0009-01-01') values (4, "aaaaabbbb");
INSERT OVERWRITE TABLE text_test_8 PARTITION(m='0009-01-01') values (0, "aaaaabbbb");

ALTER TABLE text_test_8 ADD COLUMNS(a decimal(20, 10));
INSERT INTO text_test_8 PARTITION(m='0009-01-01') values (4, "aaaaabbbb", 123.12345);

CREATE TABLE text_test_bucket (
  id  int,
  name               string,
  name2              string
) CLUSTERED BY (id) INTO 5 BUCKETS
STORED AS TEXTFILE
TBLPROPERTIES ("transactional"="false");

INSERT INTO TABLE text_test_bucket values(1, "a", "a"), (2, NULL, "a"), (3, "a", "a"), (4, "a", NULL), (5, "a", "a");

CREATE TABLE text_test_partition
(
    id  int,
    name string
)
PARTITIONED BY
(
    m int, n string, o decimal(20, 10), p tinyint, q smallint, s bigint
)
STORED AS TEXTFILE;

INSERT INTO TABLE text_test_partition PARTITION(m,n,o,p,q,s) values
(10, "aaaaabbbb", 7, "aa", 1.10000000, 1, 1, 1),
(9, "bbbbcccc", 7, "aa", 200.999999, 0, 2, 3),
(8, "ddddbbbb", 7, "aa", 1.99999, 1, 4, 5),
(7, "fdsfsf", 7, "bb", 999999999.9999999, 0, 22, 3),
(6, "rewrwr3r2", 7, "bb", 1234567890.1234567891, 1, 2, 3);

CREATE TABLE normal_text
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
) STORED AS TEXTFILE;

INSERT INTO normal_text VALUES (1, 1, 1, 1, 1, 1, '1', '2020-01-01 01:01:01', '2020-01-01', '1', '1', 10.01),
(2, 2, 2, 2, 2, 2, '2', '2020-02-02 02:02:02', '2020-02-01', '2', '2', 11.01),
(3, 3, 3, 3, 3, 3, '3', '2020-03-03 03:03:03', '2020-03-01', '3', '3', 12.01);
