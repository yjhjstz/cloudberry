package cn.cbdb.hivesync.kafka.consumer;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.jasypt.util.text.StrongTextEncryptor;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;

import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_USER;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CATALOG;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_GPNAME;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_HDFSNAME;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_PASSWD;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_DB;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_HOST;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_PORT;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_SECRET;

public class CbdbConnector {
    private static final Logger log = LoggerFactory.getLogger(CbdbConnector.class);

    private static final String createServerSQL = "select create_foreign_server('__hive_auto_sync_server', ?, 'datalake_fdw', ?)";
    private static final String createTableSQL = "select sync_hive_table(?, ?, ?, ?, ?, '__hive_auto_sync_server', true)";
    private static final String alterTableSQL = "select sync_hive_table(?, ?, ?, ?, ?, '__hive_auto_sync_server', true)";
    private static final String dropTableSQL = "drop external table if exists %s";
    private static final String createSchemaSQL = "create schema if not exists %s";
    private static final String dropSchemaSQL = "drop schema if exists %s cascade";
    private static final String alterPartitionSQL = "select gp_toolkit.__gopher_cache_free_relation_name(?)";
    private static final String urlFormat = "jdbc:postgresql://%s:%s/%s";

    public static final String CREATE_TABLE_METHOD = "createTable";
    public static final String DROP_TABLE_METHOD = "dropTable";
    public static final String ALTER_TABLE_METHOD = "alterTable";
    public static final String ALTER_PARTITION_METHOD = "alterPartition";
    public static final String DROP_SCHEMA_METHOD = "dropSchema";
    public static final String CREATE_DB_METHOD = "createDBIfNeed";
    public static final String INIT_DB_METHOD = "initDB";

    final private int RETRY_TIMES = 3;
    final private int QUERY_TIMEOUT = 30;
    final private int IDLE_TIMEOUT = 30000;
    final private int CONNECT_TIMEOUT = 5000;

    private String hdwUser;
    private String hdwPasswd;
    private String hdwDatabase;
    private String hdwSecretKey;
    private String hdwHost;
    private String hdwPort;
    private String catName;
    private String hiveName;
    private String hdfsName;

    private HikariDataSource ds;

    private interface SQLExecutor {
        public void execute(Connection conn) throws SQLException;
        public String getMessage();
    }

    private void buildDataSource() {
        HikariConfig config = new HikariConfig();
        config.setJdbcUrl(String.format(urlFormat, hdwHost, hdwPort, hdwDatabase));
        config.setUsername(hdwUser);
        if (hdwSecretKey != null && hdwSecretKey.length() > 0) {
            StrongTextEncryptor encryptor = new StrongTextEncryptor();
            encryptor.setPassword(hdwSecretKey);
            String passwd = encryptor.decrypt(hdwPasswd);
            config.setPassword(passwd);
        } else {
            config.setPassword(hdwPasswd);
        }
        config.setMaximumPoolSize(1);
        config.setMinimumIdle(0);
        config.setIdleTimeout(IDLE_TIMEOUT);
        config.setConnectionTimeout(CONNECT_TIMEOUT);
        ds = new HikariDataSource(config);
    }

    private void initDatasource() {
        try (Connection conn = ds.getConnection()) {
            try (PreparedStatement st = conn.prepareStatement(createServerSQL)) {
                st.setString(1, hdwUser);
                st.setString(2, hdfsName);
                st.executeQuery();
            }
        } catch (SQLException e) {
            log.error("Init cbdb datasource failed!", e);
        }
    }

    public CbdbConnector(Properties props) throws SQLException {
        hdwUser = props.getProperty(HDW_USER.key());
        hdwPasswd = props.getProperty(HDW_PASSWD.key());
        hdwDatabase = props.getProperty(HDW_DB.key());
        hdwHost = props.getProperty(HDW_HOST.key());
        hdwPort = props.getProperty(HDW_PORT.key());
        catName = props.getProperty(HIVE_CATALOG.key());
        hiveName = props.getProperty(HIVE_GPNAME.key());
        hdfsName = props.getProperty(HIVE_HDFSNAME.key());
        hdwSecretKey = props.getProperty(HDW_SECRET.key());
        log.debug("Build CbdbConnector: " + hdwUser + ":" + hdwPasswd + "@" + hdwHost + ":" + hdwPort + "/" + hdwDatabase + " with " + hdwSecretKey);
        buildDataSource();
        initDatasource();
    }

    private String getMethodMessage(String methodName, String tableName) {
        String op = null;
        if (methodName.equals(CREATE_TABLE_METHOD)) {
            op = "Create table";
        } else if (methodName.equals(DROP_TABLE_METHOD)) {
            op = "Drop table";
        } else if (methodName.equals(ALTER_TABLE_METHOD)) {
            op = "Alter table";
        } else if (methodName.equals(ALTER_PARTITION_METHOD)) {
            op = "Alter partition";
        } else if (methodName.equals(INIT_DB_METHOD)) {
            op = "Init database";
        } else if (methodName.equals(CREATE_DB_METHOD)) {
            op = "Create database if not exists";
        } else if (methodName.equals(DROP_SCHEMA_METHOD)){
            op = "Drop database if exists";
        } else {
            log.error("Unrecognized method: " + methodName);
            throw new IllegalArgumentException("Unrecognized method name: " + methodName);
        }
        return op + " " + tableName;
    }

    private Boolean execSQL(SQLExecutor executor, int retryTimes) {
        log.info(executor.getMessage() + " start......");
        int retry = retryTimes;
        while (retry > 0) {
            try (Connection conn = ds.getConnection()) {                
                executor.execute(conn);
                retry = 0;
                log.info(executor.getMessage() + " success!");
                return true;
            } catch (SQLException e) {
                retry -= 1;
                if (retry > 0) {
                    log.error(String.format(executor.getMessage() + " failed %d, retry......", retryTimes - retry), e);
                } else {
                    log.error(executor.getMessage() + " failed!", e);
                }
            }
        }
        return false;
    }

    public void createSchemaIfNeed(Connection conn, String schemaName) throws SQLException {
        if (schemaName.equals("default")) {
            return;
        }
        try (PreparedStatement st = conn.prepareStatement(String.format(createSchemaSQL, schemaName))) {
            st.setQueryTimeout(QUERY_TIMEOUT);
            st.executeUpdate();
            st.close();
        }
    }

    public Boolean createTable(String schemaName, String tableName, int hiveVer) {
        SQLExecutor executor = new SQLExecutor() {
            public void execute(Connection conn) throws SQLException {
                createSchemaIfNeed(conn, schemaName);
                try (PreparedStatement st = conn.prepareStatement(createTableSQL)) {
                    st.setString(1, hiveName);
                    if (hiveVer == 3) {
                        st.setString(2, catName + '.' + schemaName);
                    } else {
                        st.setString(2, schemaName);
                    }
                    st.setString(3, tableName);
                    st.setString(4, hdfsName);
                    st.setString(5, getTargetSchemaName(schemaName) + '.' + tableName);
                    st.setQueryTimeout(QUERY_TIMEOUT);
                    st.executeQuery();
                }
            }
            public String getMessage() {
                return getMethodMessage(CREATE_TABLE_METHOD, hdwDatabase + '.' + getTargetSchemaName(schemaName) + '.' + tableName);
            }
        };
        return execSQL(executor, RETRY_TIMES);
    }

    public Boolean dropTable(String schemaName, String tableName) {
        SQLExecutor executor = new SQLExecutor() {
            public void execute(Connection conn) throws SQLException {
                try (PreparedStatement s = conn.prepareStatement(String.format(dropTableSQL, getTargetSchemaName(schemaName) + "." + tableName))) {
                    s.setQueryTimeout(QUERY_TIMEOUT);
                    s.executeUpdate();
                }
            }
            public String getMessage() {
                return getMethodMessage(DROP_TABLE_METHOD, hdwDatabase + '.' + getTargetSchemaName(schemaName) + '.' + tableName);
            }
        };
        return execSQL(executor, RETRY_TIMES);
    }

    public Boolean alterTable(String schemaName, String tableName, int hiveVer) {
        SQLExecutor executor = new SQLExecutor() {
            @Override
            public void execute(Connection conn) throws SQLException {
                try (PreparedStatement st = conn.prepareStatement(alterTableSQL)) {
                    st.setString(1, hiveName);
                    if (hiveVer == 3) {
                        st.setString(2, catName + '.' + schemaName);
                    } else {
                        st.setString(2, schemaName);
                    }
                    st.setString(3, tableName);
                    st.setString(4, hdfsName);
                    st.setString(5, getTargetSchemaName(schemaName) + '.' + tableName);
                    st.setQueryTimeout(QUERY_TIMEOUT);
                    st.executeQuery();
                }
            }
            @Override
            public String getMessage() {
                return getMethodMessage(ALTER_TABLE_METHOD, hdwDatabase + '.' + getTargetSchemaName(schemaName) + '.' + tableName);
            }
        };
        return execSQL(executor, RETRY_TIMES);
    }

    public Boolean alterPartition(String schemaName, String tableName) {
        SQLExecutor executor = new SQLExecutor() {
            @Override
            public void execute(Connection conn) throws SQLException {
                try (PreparedStatement st = conn.prepareStatement(alterPartitionSQL)) {
                    st.setString(1, getTargetSchemaName(schemaName) + '.' + tableName);
                    st.setQueryTimeout(QUERY_TIMEOUT);
                    st.executeQuery();
                }
            }
            @Override
            public String getMessage() {
                return getMethodMessage(ALTER_PARTITION_METHOD, hdwDatabase + '.' + getTargetSchemaName(schemaName) + '.' + tableName);
            }
        };
        return execSQL(executor, RETRY_TIMES);
    }

    public Boolean dropSchema(String schemaName) {
        if (schemaName.equals("default")) {
            return true;
        }
        SQLExecutor executor = new SQLExecutor() {
            @Override
            public void execute(Connection conn) throws SQLException {
                try (PreparedStatement st = conn.prepareStatement(String.format(dropSchemaSQL, schemaName))) {
                    st.setQueryTimeout(QUERY_TIMEOUT);
                    st.executeUpdate();
                }
            }
            @Override
            public String getMessage() {
                return getMethodMessage(DROP_SCHEMA_METHOD, hdwDatabase + '.' + schemaName + ".*");
            }
        };
        return execSQL(executor, RETRY_TIMES);
    }

    public String getTargetSchemaName(String schemaName) {
        if ("default".equals(schemaName)) {
            return "public";
        }
        return schemaName;
    }

    public void close() {
        if (ds != null) {
            ds.close();
        }
    }
}
