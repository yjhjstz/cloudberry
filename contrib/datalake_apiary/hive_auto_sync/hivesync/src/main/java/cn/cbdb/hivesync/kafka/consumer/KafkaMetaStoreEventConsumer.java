package cn.cbdb.hivesync.kafka.consumer;

import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.APPLICATION_NAME;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.BOOTSTRAP_SERVERS;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_DB;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CATALOG;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CATLIST;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CLUSTER_NAME;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_MECHANISM;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_SASL_JAAS_CONFIG;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_SECURITY;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.MAX_POLL_INTERVAL_MS;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.MAX_POLL_RECORDS;
import static cn.cbdb.hivesync.kafka.consumer.HiveAutoSyncProperty.TOPIC_NAME;
import static com.expediagroup.apiary.extensions.events.metastore.event.CustomEventParameters.HIVE_VERSION;

import java.util.Map;
import java.util.HashMap;
import java.util.Properties;

import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.api.Table;
import org.apache.kafka.common.errors.InterruptException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryAlterPartitionEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryAlterTableEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryCreateTableEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryDropDatabaseEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryDropPartitionEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryDropTableEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.ApiaryListenerEvent;
import com.expediagroup.apiary.extensions.events.metastore.event.EventType;
import com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaMessageReader;
import com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaMessageReader.KafkaMessageReaderBuilder;

import cn.cbdb.hivesync.kafka.prometheus.HiveSyncMetrics;

public class KafkaMetaStoreEventConsumer implements Runnable{
    private static final Logger log = LoggerFactory.getLogger(KafkaMetaStoreEventConsumer.class);

    private KafkaMessageReader kafkaMessageReader;
    private Properties props;
    private Map<String, CbdbConnector> connectorMap = new HashMap<String, CbdbConnector>();

    public KafkaMetaStoreEventConsumer(Properties props) {
        this.props = props;
    }

    private void pullFromKafka() {
        while (kafkaMessageReader.hasNext()) {
            try {
                ApiaryListenerEvent event = kafkaMessageReader.next();
                log.debug("poll success, type: " + event.getEventType().toString());
                syncEvent(event);
            } catch (InterruptException e) {
                for (CbdbConnector connector : connectorMap.values()) {
                    connector.close();
                }
                log.info("Syncer for cluster " + props.getProperty(HIVE_CLUSTER_NAME.key()) + " is stopped!");
                return;
            }
        }
    }

    private void syncEvent(ApiaryListenerEvent event) {
        log.debug(event.toString());
        String catalog = getConnectorName(event);
        CbdbConnector cbdbConnector = connectorMap.get(catalog);
        if (cbdbConnector == null) {
            log.info("No cbdb connector for catalog " + catalog);
            return;
        }
        if (isTableEvent(event) || isPartitionEvent(event)) {
            String tableType = getTableFromEvent(event).getTableType();
            if (!"MANAGED_TABLE".equals(tableType)) {
                log.info("Table type not supported: " + tableType);
                return;
            }
        }

        EventType et = event.getEventType();
        Boolean syncResult = false;
        int majorVer = Integer.parseInt(event.getParameters().get(HIVE_VERSION.varname()).split("\\.")[0]);
        if (et == EventType.ON_CREATE_TABLE) {
            syncResult = cbdbConnector.createTable(event.getDatabaseName(), event.getTableName(), majorVer);
        }
        else if (et == EventType.ON_ALTER_TABLE) {
            if (majorVer == 2) {
                syncResult = cbdbConnector.alterTable(event.getDatabaseName(), event.getTableName(), majorVer);
            } else if (event.getEnvironmentContext() != null && 
                       event.getEnvironmentContext().getProperties() != null && 
                       event.getEnvironmentContext().getProperties().containsKey("alterTableOpType")) {
                syncResult = cbdbConnector.alterTable(event.getDatabaseName(), event.getTableName(), majorVer);
            }
        }
        else if (et == EventType.ON_DROP_TABLE) {
            syncResult = cbdbConnector.dropTable(event.getDatabaseName(), event.getTableName());
        }
        else if (et == EventType.ON_DROP_PARTITION || et == EventType.ON_ALTER_PARTITION) {
            syncResult = cbdbConnector.alterPartition(event.getDatabaseName(), event.getTableName());
        }
        else if (et == EventType.ON_DROP_DATABASE) {
            syncResult = cbdbConnector.dropSchema(event.getDatabaseName());
        }
        metricPublish(syncResult, et);
    }

    private void metricPublish(Boolean flag, EventType et) {
        if (et == EventType.ON_CREATE_TABLE || et == EventType.ON_ALTER_TABLE || et == EventType.ON_DROP_TABLE) {
            HiveSyncMetrics.incrementSyncTableCounts(flag);
        }
        else if (et == EventType.ON_DROP_PARTITION || et == EventType.ON_ALTER_PARTITION) {
            HiveSyncMetrics.incrementSyncPartitionCounts(flag);
        }
        else if (et == EventType.ON_DROP_DATABASE) {
            HiveSyncMetrics.incrementSyncDatabaseCounts(flag);
        }
    }

    public void run() {
        String[] catalogList = props.getProperty(HIVE_CATLIST.key()).split(",");
        String clusterName = props.getProperty(HIVE_CLUSTER_NAME.key());
        for (String catalog : catalogList) {
            try {
                props.setProperty(HIVE_CATALOG.key(), catalog);
                if (!props.containsKey(HDW_DB.key() + "." + catalog)) {
                    props.setProperty(HDW_DB.key(), clusterName + "." + catalog);
                } else {
                    props.setProperty(HDW_DB.key(), props.getProperty(HDW_DB.key() + "." + catalog));
                }
                connectorMap.put(catalog, new CbdbConnector(props));
                HiveSyncMetrics.incrementConnectorCounts();
                log.info("Build cbdb connector " + props.getProperty(HDW_DB.key()) + " success!");
            } catch (Exception e) {
                log.error("Build cbdb connector " + props.getProperty(HDW_DB.key()) + " failed!", e);
            }
        }
        if (connectorMap.isEmpty()) {
            log.error("No cbdb connector is built, stop syncer for cluster " + props.getProperty(HIVE_CLUSTER_NAME.key()));
            return;
        }
        buildKafkaReader();
        log.info(String.format("Build kafka reader: server: %s, topic: %s, app: %s" , props.getProperty(BOOTSTRAP_SERVERS.key()),
                    props.getProperty(TOPIC_NAME.key()),
                    props.getProperty(APPLICATION_NAME.key())));
        pullFromKafka();
    }

    private Properties setProperties(Properties props) {
        Properties additionalProps = new Properties();
        additionalProps.put(MAX_POLL_INTERVAL_MS.key(),
                props.getOrDefault(MAX_POLL_INTERVAL_MS.key(), MAX_POLL_INTERVAL_MS.defaultValue()));
        additionalProps.put(MAX_POLL_RECORDS.key(),
                props.getOrDefault(MAX_POLL_RECORDS.key(), MAX_POLL_RECORDS.defaultValue()));
        String protocol = props.getProperty(KAFKA_SECURITY.key(), (String)KAFKA_SECURITY.defaultValue());
        if (protocol.equals("SASL_PLAINTEXT") || protocol.equals("SASL_SSL")) {
            additionalProps.put(KAFKA_SECURITY.key(), protocol);
            additionalProps.put(KAFKA_MECHANISM.key(), props.getProperty(KAFKA_MECHANISM.key()));
            additionalProps.put(KAFKA_SASL_JAAS_CONFIG.key(), props.getProperty(KAFKA_SASL_JAAS_CONFIG.key()));
        }
        return additionalProps;
    }

    private void buildKafkaReader() {
        KafkaMessageReaderBuilder builder = KafkaMessageReaderBuilder.builder(
                props.getProperty(BOOTSTRAP_SERVERS.key()),
                props.getProperty(TOPIC_NAME.key()),
                props.getProperty(APPLICATION_NAME.key()));
        builder.withConsumerProperties(setProperties(props));
        kafkaMessageReader = builder.build();
    }

    private Table getTableFromEvent(ApiaryListenerEvent event) {
        EventType et = event.getEventType();
        if (et == EventType.ON_CREATE_TABLE) {
            return ((ApiaryCreateTableEvent)event).getTable();
        }
        if (et == EventType.ON_ALTER_TABLE) {
            return ((ApiaryAlterTableEvent)event).getNewTable();
        }
        if (et == EventType.ON_DROP_TABLE) {
            return ((ApiaryDropTableEvent)event).getTable();
        }
        if (et == EventType.ON_ALTER_PARTITION) {
            return ((ApiaryAlterPartitionEvent)event).getTable();
        }
        if (et == EventType.ON_DROP_PARTITION) {
            return ((ApiaryDropPartitionEvent)event).getTable();
        }
        return null;
    }

    private Database getDBFromEvent(ApiaryListenerEvent event) {
        if (event.getEventType() == EventType.ON_DROP_DATABASE) {
            return ((ApiaryDropDatabaseEvent)event).getDatabase();
        }
        return null;
    }

    private String getConnectorName(ApiaryListenerEvent event) {
        String version = event.getParameters().get(HIVE_VERSION.varname());
        if (version != null && version.startsWith("2")) {
            return "hive";
        }
        if (isTableEvent(event) || isPartitionEvent(event)) {
            return getTableFromEvent(event).getCatName();
        }
        if (isDatabaseEvent(event)) {
            return getDBFromEvent(event).getCatalogName();
        }
        return null;
    }

    private Boolean isTableEvent(ApiaryListenerEvent event) {
        EventType et = event.getEventType();
        return et == EventType.ON_CREATE_TABLE || et == EventType.ON_ALTER_TABLE || et == EventType.ON_DROP_TABLE;
    }

    private Boolean isPartitionEvent(ApiaryListenerEvent event) {
        EventType et = event.getEventType();
        return et == EventType.ON_DROP_PARTITION || et == EventType.ON_ALTER_PARTITION;
    }

    private Boolean isDatabaseEvent(ApiaryListenerEvent event) {
        return event.getEventType() == EventType.ON_DROP_DATABASE;
    }
}
