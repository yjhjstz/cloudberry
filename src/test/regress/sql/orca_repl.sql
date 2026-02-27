-- Test ORCA parallel execution with replicated tables
create schema orca_repl;
set search_path=orca_repl, public;

set parallel_setup_cost=0;
set max_parallel_workers_per_gather=2;
set enable_parallel = on;
set optimizer = on;

-- Replicated-Replicated join
drop table if exists rep_a, rep_b;
create table rep_a (id int, val text) distributed replicated;
create table rep_b (id int, data int) distributed replicated;
insert into rep_a select i, 'v' || i from generate_series(1, 10) i;
insert into rep_b select i, i*10 from generate_series(1, 5000) i;
analyze rep_a;
analyze rep_b;

explain (verbose, costs off) select * from rep_a join rep_b on rep_a.id = rep_b.id;
select * from rep_a join rep_b on rep_a.id = rep_b.id;

-- Distributed-Replicated join
drop table if exists rep_t, dist_t;
create table rep_t (id int, val text) distributed replicated;
create table dist_t (id int, info text) distributed by (id);
insert into rep_t select i, 'v' || i from generate_series(1, 10) i;
insert into dist_t select i, 'info' || i from generate_series(1, 5000) i;
analyze rep_t;
analyze dist_t;

explain (verbose, costs off) select * from dist_t join rep_t on dist_t.id = rep_t.id;
select * from dist_t join rep_t on dist_t.id = rep_t.id;

-- Anti Semi Join (NOT IN) with replicated inner table
drop table if exists notin_outer, notin_inner_repl;
create table notin_outer(a int, b int) with(parallel_workers=2) distributed by (a);
create table notin_inner_repl(c2 int) with(parallel_workers=2) distributed replicated;
insert into notin_outer select i, i+1 from generate_series(1, 1000) i;
insert into notin_inner_repl values (1), (2), (3), (null), (5), (null), (10), (20);
analyze notin_outer;
analyze notin_inner_repl;

-- NOT IN with nullable inner key
explain (costs off) select * from notin_outer where a not in (select c2 from notin_inner_repl);
-- NOT IN with non-null inner key filter
explain (costs off) select * from notin_outer where a not in (select c2 from notin_inner_repl where c2 is not null);

-- Left Outer Join with replicated table
drop table if exists loj_dist, loj_repl;
create table loj_dist(a int, b int) with(parallel_workers=2) distributed by (a);
create table loj_repl(c int, d int) with(parallel_workers=2) distributed replicated;
insert into loj_dist select i, i+1 from generate_series(1, 1000) i;
insert into loj_repl select i, i*10 from generate_series(1, 5000) i;
analyze loj_dist;
analyze loj_repl;

-- Distributed left join replicated (replicated as inner)
explain (costs off) select * from loj_dist left join loj_repl on loj_dist.a = loj_repl.c;
-- Replicated left join distributed (replicated as outer)
explain (costs off) select * from loj_repl left join loj_dist on loj_repl.c = loj_dist.a;

-- Left Semi Join (IN / EXISTS) with replicated inner table
drop table if exists semi_dist, semi_repl;
create table semi_dist(a int, b int) with(parallel_workers=2) distributed by (a);
create table semi_repl(c int, d int) with(parallel_workers=2) distributed replicated;
insert into semi_dist select i, i+1 from generate_series(1, 5000) i;
insert into semi_repl select i, i*10 from generate_series(1, 20) i;
analyze semi_dist;
analyze semi_repl;

-- IN subquery: distributed outer, replicated inner
explain (costs off) select * from semi_dist where a in (select c from semi_repl);
-- EXISTS subquery: distributed outer, replicated inner
explain (costs off) select * from semi_dist where exists (select c from semi_repl where semi_dist.a = semi_repl.c);
-- Replicated outer, distributed inner (should not use parallel semi join)
explain (costs off) select * from semi_repl where c in (select a from semi_dist);

-- Parallel HashAgg with replicated table
-- Should NOT produce single-stage ParallelHashAgg on replicated table
explain (costs off) select c, count(*) from semi_repl group by c;

-- Parallel GroupAgg (StreamAgg) with replicated table
set optimizer_enable_hashagg = off;
set optimizer_enable_parallel_hashagg=off;
explain (costs off) select c, count(*) from semi_repl group by c;
reset optimizer_enable_parallel_hashagg;
reset optimizer_enable_hashagg;

-- cleanup
reset enable_parallel;
reset max_parallel_workers_per_gather;
reset parallel_setup_cost;
reset optimizer;

-- start_ignore
drop schema orca_repl cascade;
-- end_ignore
