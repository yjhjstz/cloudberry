-- Test 1. extra test of numeric_fixeddecimal() and fixeddecimal_numeric()

create table t_conversion(a text);
insert into t_conversion select i || '.0' from generate_series(1, 12345)i;
insert into t_conversion select (i * 10000) || '.0' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000) || '.0' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000 * 10000) || '.0' from generate_series(1, 12345)i;

insert into t_conversion select i || '.7' from generate_series(1, 12345)i;
insert into t_conversion select (i * 10000) || '.7' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000) || '.7' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000 * 10000) || '.7' from generate_series(1, 12345)i;

insert into t_conversion select i || '.78' from generate_series(1, 12345)i;
insert into t_conversion select (i * 10000) || '.78' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000) || '.78' from generate_series(1, 12345)i;
insert into t_conversion select (i::bigint * 10000 * 10000 * 10000) || '.78' from generate_series(1, 12345)i;

insert into t_conversion values('.0'), ('-.0'), ('-0'), ('-0.0');

with r(f2n, n2f, tof) as (
select a::fixeddecimal::numeric = a::numeric,
        a::fixeddecimal = a::numeric::fixeddecimal,
        a::numeric(19, 2)::text = a::fixeddecimal::text
from t_conversion
)
select f2n, n2f, tof from r group by 1,2,3;

-- Test 2. optimize aggregate function sum/avg on numeric(P, 2)
create table t_opt_agg(a numeric(11, 2), b numeric(13, 2), i1 int2, i2 int4, i3 int8);

set fixeddecimal.enable_optimizer = on;

-- Test 2.1 simple numeric var
explain (verbose) select sum(a) from t_opt_agg;
explain (verbose) select avg(a) from t_opt_agg;

-- Test 2.2 add/substract numeric/integer
explain (verbose) select sum(a + 1) from t_opt_agg;
explain (verbose) select sum(a - 2) from t_opt_agg;
explain (verbose) select sum(a + i1) from t_opt_agg;
explain (verbose) select sum(a + i2) from t_opt_agg;

-- fallback, doesn't define any operator with int8
explain (verbose) select sum(a + i3) from t_opt_agg;

explain (verbose) select sum(a + b) from t_opt_agg;
explain (verbose) select sum(a - b) from t_opt_agg;
explain (verbose) select sum(a - b + a) from t_opt_agg;

-- Test 2.3 multiply integer
explain (verbose) select sum(a * 2) from t_opt_agg;
explain (verbose) select sum(a * i1) from t_opt_agg;
explain (verbose) select sum(a * i2) from t_opt_agg;

-- fallback, doesn't define any operator with int8
explain (verbose) select sum(a * i3) from t_opt_agg;
-- fallback
explain (verbose) select sum(a * b) from t_opt_agg;
explain (verbose) select sum(a / b) from t_opt_agg;
explain (verbose) select sum(a / i2) from t_opt_agg;

-- Test 2.4 complex expression
explain (verbose) select sum(a * i2 + abs(-2)) from t_opt_agg;
explain (verbose) select sum(a * abs(3)) from t_opt_agg;
-- fallback, can't know the scale of abs
explain (verbose) select sum(a * abs(b) + abs(i2)) from t_opt_agg;

-- Test 3 correctness test
insert into t_opt_agg select 
    (i * 217 - 37)/100.0,
    (i * 517 - 37) / 100.0,
    i - 100, i * 7 - 1234, null
from generate_series(11, 31009, 0.3)i;

------ begin test result
-- Test 2.1 simple numeric var
select sum(a) from t_opt_agg;
select avg(a) from t_opt_agg;
select avg(b) from t_opt_agg;
select avg(a + b) from t_opt_agg;

-- Test 2.2 add/substract numeric/integer
select sum(a + 1) from t_opt_agg;
select sum(a - 2) from t_opt_agg;
select sum(a + i1) from t_opt_agg;
select sum(a + i2) from t_opt_agg;
select sum(a + i3) from t_opt_agg;

select sum(a + b) from t_opt_agg;
select sum(a - b) from t_opt_agg;
select sum(a - b + a) from t_opt_agg;

-- Test 2.3 multiply integer
select sum(a * 2) from t_opt_agg;
select sum(a * i1) from t_opt_agg;
select sum(a * i2) from t_opt_agg;
select sum(a * i3) from t_opt_agg;

-- Test 2.4 complex expression
select sum(a * i2 + abs(-2)) from t_opt_agg;
select sum(a * abs(3)) from t_opt_agg;

------ end test result;

-- turn off optimization and run
set fixeddecimal.enable_optimizer = off;

------ begin test result
-- Test 2.1 simple numeric var
select sum(a) from t_opt_agg;
select avg(a) from t_opt_agg;
select avg(b) from t_opt_agg;
select avg(a + b) from t_opt_agg;

-- Test 2.2 add/substract numeric/integer
select sum(a + 1) from t_opt_agg;
select sum(a - 2) from t_opt_agg;
select sum(a + i1) from t_opt_agg;
select sum(a + i2) from t_opt_agg;
select sum(a + i3) from t_opt_agg;

select sum(a + b) from t_opt_agg;
select sum(a - b) from t_opt_agg;
select sum(a - b + a) from t_opt_agg;

-- Test 2.3 multiply integer
select sum(a * 2) from t_opt_agg;
select sum(a * i1) from t_opt_agg;
select sum(a * i2) from t_opt_agg;
select sum(a * i3) from t_opt_agg;

-- Test 2.4 complex expression
select sum(a * i2 + abs(-2)) from t_opt_agg;
select sum(a * abs(3)) from t_opt_agg;

------ end test result;
