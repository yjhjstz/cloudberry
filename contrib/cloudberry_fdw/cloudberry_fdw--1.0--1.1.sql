/* contrib/cloudberry_fdw/cloudberry_fdw--1.0--1.1.sql */

-- complain if script is sourced in psql, rather than via ALTER EXTENSION
\echo Use "ALTER EXTENSION cloudberry_fdw UPDATE TO '1.1'" to load this file. \quit

CREATE FUNCTION cloudberry_fdw_get_connections (OUT server_name text,
    OUT valid boolean)
RETURNS SETOF record
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT PARALLEL RESTRICTED;

CREATE FUNCTION cloudberry_fdw_disconnect (text)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT PARALLEL RESTRICTED;

CREATE FUNCTION cloudberry_fdw_disconnect_all ()
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT PARALLEL RESTRICTED;

CREATE SCHEMA IF NOT EXISTS cloudberry_fdw;

CREATE FUNCTION cloudberry_fdw.cbdb_fdw_get_helper_ports (
	cmdid cstring,
	OUT cmdID text,
	OUT segID int4,
	OUT port int4)
RETURNS SETOF record
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT EXECUTE ON ALL SEGMENTS;

CREATE FUNCTION cloudberry_fdw.cbdb_fdw_copy_from (
	Oid, cstring,
	int4 DEFAULT 0,
	cstring DEFAULT '',
	cstring DEFAULT '')
RETURNS SETOF record
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT EXECUTE ON ALL SEGMENTS;
