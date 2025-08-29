-- Test CREATE ICEBERG TABLE functionality
-- This tests the creation and description of iceberg tables with foreign catalogs and volumes

-- Create foreign catalog
CREATE FOREIGN CATALOG basic_catalog SERVER gp_exttable_server;

-- Create foreign volume  
CREATE FOREIGN VOLUME basic_volume SERVER gp_exttable_server;

-- Create iceberg table with foreign catalog and volume
CREATE ICEBERG TABLE sales_data (a int) FOREIGN CATALOG basic_catalog FOREIGN VOLUME basic_volume OPTIONS (database_name 'sales_db');

-- Test table description
\d+ sales_data

-- Test dependency checking - these should fail with dependency errors
\echo 'Testing dependency check for FOREIGN CATALOG (should fail):'
DROP CATALOG basic_catalog;

\echo 'Testing dependency check for FOREIGN VOLUME (should fail):'
DROP VOLUME basic_volume;

-- Clean up
DROP FOREIGN TABLE sales_data;
DROP VOLUME basic_volume;
DROP CATALOG basic_catalog;
