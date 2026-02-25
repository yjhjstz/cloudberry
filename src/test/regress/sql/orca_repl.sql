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

-- cleanup
reset enable_parallel;
reset max_parallel_workers_per_gather;
reset parallel_setup_cost;
reset optimizer;

-- start_ignore
drop schema orca_repl cascade;
-- end_ignore
