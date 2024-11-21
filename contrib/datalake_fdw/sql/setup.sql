-- Create extension
CREATE EXTENSION IF NOT EXISTS datalake_fdw;
CREATE EXTENSION IF NOT EXISTS hive_connector;
CREATE EXTENSION gp_exttable_delimiter;

-- Create data wrapper
CREATE FOREIGN DATA WRAPPER datalake_fdw
HANDLER datalake_fdw_handler
VALIDATOR datalake_fdw_validator
OPTIONS (mpp_execute 'all segments');

-- Create server and user mapping for hive
SELECT public.create_foreign_server('hive_server', 'gpadmin', 'datalake_fdw', 'paa_cluster');

-- Create server and user mapping for obs
CREATE SERVER oss_server
        FOREIGN DATA WRAPPER datalake_fdw
        OPTIONS (host 'obs.cn-north-4.myhuaweicloud.com', protocol 'huawei', isvirtual 'false',
        ishttps 'false');
CREATE USER MAPPING FOR gpadmin
        SERVER oss_server
        OPTIONS (user 'gpadmin', accesskey 'J04WCCF5VQP6BAIQUFHP', secretkey 'jGDwttCct2b9b4rEf0hsLD7CeP9WubZuqqz90iQU');

-- Create server and user mapping for hive_s3
CREATE SERVER hive_s3_server
        FOREIGN DATA WRAPPER datalake_fdw OPTIONS (host '127.0.0.1:9100', protocol 's3', isvirtual 'false', ishttps 'false');
CREATE USER MAPPING FOR gpadmin
        SERVER hive_s3_server
        OPTIONS (user 'gpadmin', accesskey 'admin', secretkey 'password');

-- Create server and user mapping for ftp
CREATE SERVER ftp_server
        FOREIGN DATA WRAPPER datalake_fdw
        OPTIONS (host '192.168.198.144', protocol 'ftp');
CREATE USER MAPPING FOR gpadmin
        SERVER ftp_server
        OPTIONS (user 'ftp', password 'ftp');
