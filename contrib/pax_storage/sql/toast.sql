set default_table_access_method = pax;

set pax.enable_toast to true;
CREATE TABLE pax_toasttest_t1(v1 text);
alter TABLE pax_toasttest_t1 alter column v1 set storage plain;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage extended;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage external;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage main;
insert into pax_toasttest_t1 values(repeat('0', 100000000));

select length(v1) from pax_toasttest_t1;

drop table pax_toasttest_t1;

CREATE TABLE pax_toasttest_t2(v1 text, v2 text, v3 text, v4 text, v5 text, v6 text);
alter TABLE pax_toasttest_t2 alter column v1 set storage plain;
alter TABLE pax_toasttest_t2 alter column v2 set storage extended;
alter TABLE pax_toasttest_t2 alter column v3 set storage external;
alter TABLE pax_toasttest_t2 alter column v4 set storage main;
insert into pax_toasttest_t2 values(repeat('0', 51111111), repeat('0', 52222222), repeat('0', 53333333), repeat('0', 54444444), repeat('0', 55555555), repeat('0', 56666666));
alter TABLE pax_toasttest_t2 alter column v4 set storage plain;
alter TABLE pax_toasttest_t2 alter column v3 set storage extended;
alter TABLE pax_toasttest_t2 alter column v2 set storage external;
alter TABLE pax_toasttest_t2 alter column v1 set storage main;
insert into pax_toasttest_t2 values(repeat('0', 51111111), repeat('0', 52222222), repeat('0', 53333333), repeat('0', 54444444), repeat('0', 55555555), repeat('0', 56666666));

-- test the projection
select length(v1), length(v2), length(v3), length(v4), length(v5), length(v6) from pax_toasttest_t2;
select length(v1), length(v2), length(v3), length(v4) from pax_toasttest_t2;
select length(v1) from pax_toasttest_t2;
select length(v2) from pax_toasttest_t2;
select length(v3) from pax_toasttest_t2;
select length(v4) from pax_toasttest_t2;
select length(v5) from pax_toasttest_t2;
select length(v6) from pax_toasttest_t2;

select length(v2), length(v3), length(v4) from pax_toasttest_t2;
select length(v1), length(v2), length(v3) from pax_toasttest_t2;
select length(v1), length(v4) from pax_toasttest_t2;
select length(v2), length(v3) from pax_toasttest_t2;
select length(v1), length(v3) from pax_toasttest_t2;
select length(v2), length(v4) from pax_toasttest_t2;


drop table pax_toasttest_t2;

-----------------------------------------------------------------------
-- a copy test cases that should be sucess when pax.enable_toast is off
-----------------------------------------------------------------------
set pax.enable_toast to false;
CREATE TABLE pax_toasttest_t1(v1 text);
alter TABLE pax_toasttest_t1 alter column v1 set storage plain;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage extended;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage external;
insert into pax_toasttest_t1 values(repeat('0', 100000000));
alter TABLE pax_toasttest_t1 alter column v1 set storage main;
insert into pax_toasttest_t1 values(repeat('0', 100000000));

select length(v1) from pax_toasttest_t1;

drop table pax_toasttest_t1;

CREATE TABLE pax_toasttest_t2(v1 text, v2 text, v3 text, v4 text, v5 text, v6 text);
alter TABLE pax_toasttest_t2 alter column v1 set storage plain;
alter TABLE pax_toasttest_t2 alter column v2 set storage extended;
alter TABLE pax_toasttest_t2 alter column v3 set storage external;
alter TABLE pax_toasttest_t2 alter column v4 set storage main;
insert into pax_toasttest_t2 values(repeat('0', 51111111), repeat('0', 52222222), repeat('0', 53333333), repeat('0', 54444444), repeat('0', 55555555), repeat('0', 56666666));
alter TABLE pax_toasttest_t2 alter column v4 set storage plain;
alter TABLE pax_toasttest_t2 alter column v3 set storage extended;
alter TABLE pax_toasttest_t2 alter column v2 set storage external;
alter TABLE pax_toasttest_t2 alter column v1 set storage main;
insert into pax_toasttest_t2 values(repeat('0', 51111111), repeat('0', 52222222), repeat('0', 53333333), repeat('0', 54444444), repeat('0', 55555555), repeat('0', 56666666));

-- test the projection
select length(v1), length(v2), length(v3), length(v4), length(v5), length(v6) from pax_toasttest_t2;
select length(v1), length(v2), length(v3), length(v4) from pax_toasttest_t2;
select length(v1) from pax_toasttest_t2;
select length(v2) from pax_toasttest_t2;
select length(v3) from pax_toasttest_t2;
select length(v4) from pax_toasttest_t2;
select length(v5) from pax_toasttest_t2;
select length(v6) from pax_toasttest_t2;

select length(v2), length(v3), length(v4) from pax_toasttest_t2;
select length(v1), length(v2), length(v3) from pax_toasttest_t2;
select length(v1), length(v4) from pax_toasttest_t2;
select length(v2), length(v3) from pax_toasttest_t2;
select length(v1), length(v3) from pax_toasttest_t2;
select length(v2), length(v4) from pax_toasttest_t2;


drop table pax_toasttest_t2;

set pax.enable_toast = on;
set optimizer = off;
set enable_seqscan = off;
set enable_indexscan = off;
set enable_indexonlyscan = off;
set enable_bitmapscan = on;
set work_mem="64kB";

create table pax_toasttest_t3(a int, b text);
insert into pax_toasttest_t3 select i, repeat('b_', 5000) from generate_series(1, 506841)i;
create index on pax_toasttest_t3(a);
create index on pax_toasttest_t3(a);

explain select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;
select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;

delete from pax_toasttest_t3 where a % 1000 = 0;
explain select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;
select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;

update pax_toasttest_t3 set b = null where a % 1010 = 0;
explain select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;
select count(*), count(a), count(b) from pax_toasttest_t3 where a >= 0;

