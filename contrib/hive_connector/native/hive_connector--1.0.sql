/* contrib/hive_connector/hive_connector--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION hive_connector" to load this file. \quit

SET search_path = public;

CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
serverName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
serverName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
serverName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
serverName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION create_foreign_server(serverName text,
userMapName text,
dataWrapName text,
hdfsClusterName text) RETURNS boolean
AS '$libdir/hive_connector','create_foreign_server'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION hc_drop_table(
    name text,
    miss_ok bool
) RETURNS void AS $$
DECLARE
    schema_name text;
    rel_name text;
    relkind char;
    v_sql text;
BEGIN
    -- Split schema and relation name if a schema is specified
    IF position('.' in name) > 0 THEN
        schema_name := split_part(name, '.', 1);
        rel_name := split_part(name, '.', 2);
    ELSE
        schema_name := 'public'; -- If no schema is specified, assume 'public'
        rel_name := name;
    END IF;

    -- Find the relkind of the relation
    SELECT c.relkind
    INTO relkind
    FROM pg_class c
    JOIN pg_namespace n ON c.relnamespace = n.oid
    WHERE n.nspname = schema_name AND c.relname = rel_name;

    IF NOT FOUND THEN
        IF miss_ok THEN
            RETURN; -- If the relation doesn't exist and miss_ok is true, return without error
        END IF;
        RAISE EXCEPTION 'Relation % in schema % does not exist', rel_name, schema_name; -- Raise an exception if the relation doesn't exist
    ELSE
        -- Build the DROP command based on the relation kind
        CASE relkind
            WHEN 'r' THEN -- Ordinary table or partitioned table
                v_sql := format('DROP TABLE IF EXISTS %I.%I CASCADE', schema_name, rel_name);
            WHEN 'p' THEN -- Ordinary table or partitioned table
                v_sql := format('DROP TABLE IF EXISTS %I.%I CASCADE', schema_name, rel_name);
            WHEN 'v' THEN -- View
                v_sql := format('DROP VIEW IF EXISTS %I.%I CASCADE', schema_name, rel_name);
            WHEN 'm' THEN -- Materialized view
                v_sql := format('DROP MATERIALIZED VIEW IF EXISTS %I.%I CASCADE', schema_name, rel_name);
--            WHEN 'i' THEN -- Index (assuming we have the correct index name)
--                v_sql := format('DROP INDEX IF EXISTS %I.%I CASCADE', schema_name, rel_name);
            WHEN 'f' THEN -- Foreign table
                v_sql := format('DROP FOREIGN TABLE IF EXISTS %I.%I CASCADE', schema_name, rel_name);
            ELSE
                RAISE EXCEPTION 'Unsupported relation kind: %', relkind; -- Raise an exception for unsupported relation kinds
        END CASE;

        -- Execute the DROP command
        EXECUTE v_sql;
    END IF;
EXCEPTION
    WHEN OTHERS THEN
        RAISE; -- Re-raise the exception so the caller can see the detailed error message
END;
$$ LANGUAGE plpgsql;

-- sync with logerror
CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
serverName text,
logerrors boolean,
rejectlimit int,
islimitinrows text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_logerror'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
serverName text,
logerrors boolean,
rejectlimit int,
islimitinrows text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_logerror'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
serverName text,
logerrors boolean,
rejectlimit int,
islimitinrows text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_logerror'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
serverName text,
logerrors boolean,
rejectlimit int,
islimitinrows text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_logerror'
LANGUAGE C STRICT ;


-- 3x
CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_partition_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hivePartitionValue text,
hdfsClusterName text,
destTableName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_partition_table_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_partition_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hivePartitionValue text,
hdfsClusterName text,
destTableName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_partition_table_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_3x'
LANGUAGE C STRICT ;

-- sync hive table support log error
CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
logerrors boolean,
rejectlimit int,
islimitinrows text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_logerror_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
hdfsClusterName text,
destTableName text,
logerrors boolean,
rejectlimit int,
islimitinrows text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_logerror_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
logerrors boolean,
rejectlimit int,
islimitinrows text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_logerror_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database(hiveClusterName text,
hiveDatabaseName text,
hdfsClusterName text,
destSchemaName text,
logerrors boolean,
rejectlimit int,
islimitinrows text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_logerror_3x'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table_s3(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
destTableName text,
serverName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_s3_storage'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_table_s3(hiveClusterName text,
hiveDatabaseName text,
hiveTableName text,
destTableName text,
serverName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_table_with_s3_storage'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database_s3(hiveClusterName text,
hiveDatabaseName text,
destSchemaName text,
serverName text) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_s3_storage'
LANGUAGE C STRICT ;

CREATE OR REPLACE FUNCTION sync_hive_database_s3(hiveClusterName text,
hiveDatabaseName text,
destSchemaName text,
serverName text,
forceSync boolean) RETURNS boolean
AS '$libdir/hive_connector','sync_hive_database_with_s3_storage'
LANGUAGE C STRICT ;
