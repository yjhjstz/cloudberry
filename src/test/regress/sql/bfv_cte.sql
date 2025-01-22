--
-- Test queries mixes window functions with aggregate functions or grouping.
--
DROP TABLE IF EXISTS test_group_window;

CREATE TABLE test_group_window(c1 int, c2 int);

WITH tt AS (SELECT * FROM test_group_window)
SELECT tt.c1, COUNT() over () as fraction
FROM tt
GROUP BY tt.c1
ORDER BY tt.c1;

DROP TABLE test_group_window;

--
-- Set up
--
CREATE TABLE bfv_cte_foo AS SELECT i as a, i+1 as b from generate_series(1,10)i;
CREATE TABLE bfv_cte_bar AS SELECT i as c, i+1 as d from generate_series(1,10)i;

--
-- Test with CTE inlining disabled
--
set optimizer_cte_inlining = off;

--
-- With clause select test 1
--
WITH t AS
(
 SELECT e.*,f.*
 FROM
    (
      SELECT * FROM bfv_cte_foo WHERE a < 10
    ) e
 LEFT OUTER JOIN
    (
       SELECT * FROM bfv_cte_bar WHERE c < 10
    ) f

  ON e.a = f.d )
SELECT t.a,t.d, count(*) over () AS window
FROM t
GROUP BY t.a,t.d ORDER BY t.a,t.d LIMIT 2;

--
-- With clause select test 2
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT t.b,avg(t.a), rank() OVER (PARTITION BY t.a ORDER BY t.a) FROM bfv_cte_foo,t GROUP BY bfv_cte_foo.a,bfv_cte_foo.b,t.b,t.a ORDER BY 1,2,3 LIMIT 5;

--
-- With clause select test 3
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) OVER(PARTITION BY t.b) FROM
  (
    SELECT bfv_cte_bar.*, AVG(t.b) OVER(PARTITION BY t.a ORDER BY t.b desc) AS e FROM t,bfv_cte_bar
  ) AS cup,
t WHERE cup.e < 10
GROUP BY cup.c,cup.d, cup.e ,t.d, t.b
ORDER BY 1,2,3,4
LIMIT 10;

--
-- With clause select test 4
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) FROM
  (
    SELECT bfv_cte_bar.*, count(*) OVER() AS e FROM t,bfv_cte_bar WHERE t.a = bfv_cte_bar.c
  ) AS cup,
t GROUP BY cup.c,cup.d, cup.e,t.a
HAVING AVG(t.d) < 10 ORDER BY 1,2,3,4 LIMIT 10;

--
-- With clause select test 5
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) OVER(PARTITION BY t.b) FROM
  (
    SELECT bfv_cte_bar.c as e,r.d FROM
		(
			SELECT t.d, avg(t.a) over() FROM t
		) r,bfv_cte_bar
  ) AS cup,
t WHERE cup.e < 10
GROUP BY cup.d, cup.e, t.d, t.b
ORDER BY 1,2,3
LIMIT 10;

--
-- Test with CTE inlining enabled
--
set optimizer_cte_inlining = on;
set optimizer_cte_inlining_bound = 1000;

--
-- With clause select test 1
--
WITH t AS
(
 SELECT e.*,f.*
 FROM
    (
      SELECT * FROM bfv_cte_foo WHERE a < 10
    ) e
 LEFT OUTER JOIN
    (
       SELECT * FROM bfv_cte_bar WHERE c < 10
    ) f

  ON e.a = f.d )
SELECT t.a,t.d, count(*) over () AS window
FROM t
GROUP BY t.a,t.d ORDER BY t.a,t.d LIMIT 2;

--
-- With clause select test 2
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT t.b,avg(t.a), rank() OVER (PARTITION BY t.a ORDER BY t.a) FROM bfv_cte_foo,t GROUP BY bfv_cte_foo.a,bfv_cte_foo.b,t.b,t.a ORDER BY 1,2,3 LIMIT 5;

--
-- With clause select test 3
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) OVER(PARTITION BY t.b) FROM
  (
    SELECT bfv_cte_bar.*, AVG(t.b) OVER(PARTITION BY t.a ORDER BY t.b desc) AS e FROM t,bfv_cte_bar
  ) AS cup,
t WHERE cup.e < 10
GROUP BY cup.c,cup.d, cup.e ,t.d, t.b
ORDER BY 1,2,3,4
LIMIT 10;

--
-- With clause select test 4
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) FROM
  (
    SELECT bfv_cte_bar.*, count(*) OVER() AS e FROM t,bfv_cte_bar WHERE t.a = bfv_cte_bar.c
  ) AS cup,
t GROUP BY cup.c,cup.d, cup.e,t.a
HAVING AVG(t.d) < 10 ORDER BY 1,2,3,4 LIMIT 10;

--
-- With clause select test 5
--
WITH t(a,b,d) AS
(
  SELECT bfv_cte_foo.a,bfv_cte_foo.b,bfv_cte_bar.d FROM bfv_cte_foo,bfv_cte_bar WHERE bfv_cte_foo.a = bfv_cte_bar.d
)
SELECT cup.*, SUM(t.d) OVER(PARTITION BY t.b) FROM
  (
    SELECT bfv_cte_bar.c as e,r.d FROM
		(
			SELECT t.d, avg(t.a) over() FROM t
		) r,bfv_cte_bar
  ) AS cup,
t WHERE cup.e < 10
GROUP BY cup.d, cup.e, t.d, t.b
ORDER BY 1,2,3
LIMIT 10;

DROP TABLE IF EXISTS bfv_cte_foo;
DROP TABLE IF EXISTS bfv_cte_bar;
reset optimizer_cte_inlining;
reset optimizer_cte_inlining_bound;

--
-- Test for an old bug with rescanning window functions. This needs to use
-- catalog tables, otherwise the plan will contain a Motion node that
-- materializes the result, and masks the problem.
--
with t (a,b,d) as (select 1,2,1 from pg_class limit 1)
SELECT cup.* FROM
 t, (
SELECT sum(t.b) OVER(PARTITION BY t.a ) AS e FROM (select 1 as a, 2 as b from pg_class limit 1)foo,t
) as cup
GROUP BY cup.e;

--
-- Test for a bug in translating a CTE's locus to the outer query.
--
-- This crashed at one point, when the code to translate the locus of
-- CTE subquery to the outer query's equivalence classes was broken.
-- The UNION ALL is a simple "pulled up" UNION ALL, creating an append
-- rel.
--
create temp table bfv_cte_tab (t text, i int4, j int4) distributed randomly;

insert into bfv_cte_tab values ('foo', 1, -1);
insert into bfv_cte_tab values ('bar', 2, -2);

with
  foo as (select t, sum(i) as n from bfv_cte_tab group by t),
  bar as (select t, sum(j) as n from bfv_cte_tab group by t)

select const_a, const_b, sum(n)
 from
 (select 'foo_a' as const_a, 'foo_b' as const_b, n from foo
  union all
  select 'bar_a' as const_a, 'bar_b' as const_b, n from bar
 ) x
 group by const_a, const_b
;

-- test cte can not be the param for gp_dist_random
-- so in set_cte_pathlist we do not neet to check forceDistRandom
create table ttt(tc1 int,tc2 int) ;
insert into ttt values(1,1);
insert into ttt  values(2,2);

WITH cte AS (
  SELECT oid, relname
  FROM pg_class
  WHERE oid <2000
)
SELECT *
FROM gp_dist_random('cte')
JOIN ttt ON cte.oid = ttt.tc2;

--
-- Test bug fix for reader-writer communication in Share Input Scan (SISC)
-- https://github.com/greenplum-db/gpdb/issues/16429
--
-- Helper function
CREATE OR REPLACE FUNCTION wait_until_query_output_to_file(file_path text) RETURNS VOID AS $$
DECLARE
	content TEXT;
	line TEXT;
	match TEXT;
BEGIN
	LOOP
		content := pg_read_file(file_path);
		FOR line IN (SELECT unnest(string_to_array(content, E'\n'))) LOOP
			match := substring(line FROM '\(([0-9]+) row[s]?\)');
			IF match IS NOT NULL THEN
				RETURN;
			END IF;
		END LOOP;
		PERFORM pg_sleep(0.1);  -- Sleep for a short period before checking again
	END LOOP;
END;
$$ LANGUAGE plpgsql;

-- Setup tables
CREATE TABLE sisc_t1(a int, b int) DISTRIBUTED BY (a);
INSERT INTO sisc_t1 VALUES (1, 1), (2, 2), (5, 5);
ANALYZE sisc_t1;
CREATE TABLE sisc_t2(c int, d int) DISTRIBUTED BY (c);
-- Ensure that sisc_t2 has 0 tuple on seg2
INSERT INTO sisc_t2 VALUES (1, 1), (2, 2);
ANALYZE sisc_t2;

-- ORCA plan contains one SISC writer on slice1 and two readers on slice2 and slice4.
-- The Hash Join on slice4 has its hash side being empty on seg2, so the SISC
-- reader on the probe side will be squelched. Same applies to the Hash Join
-- on slice2 for seg2.
-- Similary, planner plan contains one SISC wirter on slice4 and one reader on slice2.
-- The Hash Join on slice2 has its hash side being empty on seg2, so the SISC
-- reader on the probe side will be squelched.
SET optimizer_enable_motion_broadcast TO off;
EXPLAIN (COSTS OFF)
WITH cte AS MATERIALIZED (SELECT * FROM sisc_t1)
SELECT y.a, z.a
FROM (SELECT cte1.a, cte1.b FROM cte cte1 JOIN sisc_t2 ON (cte1.a = sisc_t2.d)) y,
     (SELECT cte2.a, cte2.b FROM cte cte2 JOIN sisc_t2 ON (cte2.a = sisc_t2.d)) z
WHERE y.b = z.b - 1;

-- On seg2, introduce a delay in SISC WRITER (slice1) so that the xslice shared state
-- which is stored in shared memory is initialized by the SISC READER (slice2 or slice4)
select gp_inject_fault('get_shareinput_reference_delay_writer', 'suspend', dbid) from gp_segment_configuration where content = 2 and role = 'p';
select gp_inject_fault('get_shareinput_reference_done', 'skip', dbid) from gp_segment_configuration where content = 2 and role = 'p';

-- Run the above CTE query again with additional debug logging.
-- Use debug_shareinput_xslice to verify that the SISC READER on seg2 is indeed
-- initialized and squelched before the SISC WRITER starts its execution.
--
-- XXX: since this is a blocking query, we need to run it in a separate shell.
-- isolation2 test seems to be a more intuitive option, however, it lacks the
-- ability to redirect the LOGs as sterr into stdout. Hence directly using bash.
\! bash -c 'psql -X regression -c "set client_min_messages to log; set debug_shareinput_xslice to true; set optimizer_enable_motion_broadcast to off; WITH cte AS MATERIALIZED (SELECT * FROM sisc_t1) SELECT y.a, z.a FROM (SELECT cte1.a, cte1.b FROM cte cte1 JOIN sisc_t2 ON (cte1.a = sisc_t2.d)) y, (SELECT cte2.a, cte2.b FROM cte cte2 JOIN sisc_t2 ON (cte2.a = sisc_t2.d)) z WHERE y.b = z.b - 1;" &> /tmp/bfv_cte.out' &

-- Wait for both SISC READERs to be initialized and squelched
select gp_wait_until_triggered_fault('get_shareinput_reference_done', 1, dbid) from gp_segment_configuration where content = 2 and role = 'p';
select gp_inject_fault('get_shareinput_reference_done', 'reset', dbid) from gp_segment_configuration where content = 2 and role = 'p';
select gp_inject_fault('get_shareinput_reference_delay_writer', 'reset', dbid) from gp_segment_configuration where content = 2 and role = 'p';

-- Wait for the query to finish
select wait_until_query_output_to_file('/tmp/bfv_cte.out');
-- start_matchsubs
-- m/SISC READER \(shareid=0, slice=2\)/
-- s/SISC READER \(shareid=0, slice=2\)/SISC READER (shareid=0, slice={2|4})/g
-- m/SISC READER \(shareid=0, slice=4\)/
-- s/SISC READER \(shareid=0, slice=4\)/SISC READER (shareid=0, slice={2|4})/g
-- m/slice=2\): initialized xslice state/
-- s/slice=2\): initialized xslice state/slice={2|4}): initialized xslice state/g
-- m/slice=4\): initialized xslice state/
-- s/slice=4\): initialized xslice state/slice={2|4}): initialized xslice state/g
-- end_matchsubs
-- Filter out irrelevant LOG messages from segments other than seg2.
\! cat /tmp/bfv_cte.out | grep -P '^(?!LOG)|^(LOG.*seg2)' | grep -vP 'LOG.*fault|decreased xslice state refcount'

-- cleanup
select gp_inject_fault_infinite('all', 'reset', dbid) from gp_segment_configuration;
RESET optimizer_enable_motion_broadcast;
DROP TABLE sisc_t1;
DROP TABLE sisc_t2;
\! rm /tmp/bfv_cte.out
