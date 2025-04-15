package cn.cbdb.apiary.kafka.listener;

import static com.expediagroup.apiary.extensions.events.metastore.common.Preconditions.checkNotEmpty;
import static com.expediagroup.apiary.extensions.events.metastore.common.PropertyUtils.stringProperty;
import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.CLIENT_ID;
import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.HDW_CATNAME;
import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.HIVE_CLUSTER_NAME;
import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.TOPIC_NAME;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hive.metastore.MetaStoreEventListener;
import org.apache.hadoop.hive.metastore.events.AddPartitionEvent;
import org.apache.hadoop.hive.metastore.events.AlterPartitionEvent;
import org.apache.hadoop.hive.metastore.events.AlterTableEvent;
import org.apache.hadoop.hive.metastore.events.ConfigChangeEvent;
import org.apache.hadoop.hive.metastore.events.CreateDatabaseEvent;
import org.apache.hadoop.hive.metastore.events.CreateTableEvent;
import org.apache.hadoop.hive.metastore.events.DropDatabaseEvent;
import org.apache.hadoop.hive.metastore.events.DropPartitionEvent;
import org.apache.hadoop.hive.metastore.events.DropTableEvent;
import org.apache.hive.common.util.HiveVersionInfo;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.expediagroup.apiary.extensions.events.metastore.catalog.HiveCatNameGetter;
import com.expediagroup.apiary.extensions.events.metastore.kafka.listener.KafkaMetaStoreEventListener;


public class HiveMetaStoreEventListener extends MetaStoreEventListener{
    private static final Logger log = LoggerFactory.getLogger(HiveMetaStoreEventListener.class);

    private final KafkaMetaStoreEventListener listener;
    private HiveMetaStoreEventFilter filter;
    private HiveCatNameGetter catNameGetter;
    private int majorVer;
    private String catGetterName;

    public HiveMetaStoreEventListener(Configuration config){
        super(config);
        log.info("HiveMetaStoreEventListener Initialize!");
        config.set(TOPIC_NAME.key(), getTopicName(config));
        config.set(CLIENT_ID.key(), config.get(TOPIC_NAME.key()));
        listener = new KafkaMetaStoreEventListener(config);
        filter = new HiveMetaStoreEventFilter(config);
        log.info("Hive version: " + HiveVersionInfo.getVersion());
        majorVer = Integer.parseInt(HiveVersionInfo.getVersion().split("\\.")[0]);
        if (majorVer >= 3) {
            catGetterName = "com.expediagroup.apiary.extensions.events.metastore.catalog.Hive3xCatNameGetter";
            log.info("Hive 3.x detected! Try to use com.expediagroup.apiary.extensions.events.metastore.catalog.Hive3xCatNameGetter!");
        } else {
            catGetterName = "com.expediagroup.apiary.extensions.events.metastore.catalog.Hive2xCatNameGetter";
            log.info("Hive 2.x detected! Try to use com.expediagroup.apiary.extensions.events.metastore.catalog.Hive2xCatNameGetter!");
        }

        try {
            Class<?> clazz = Class.forName(catGetterName);
            catNameGetter = (HiveCatNameGetter) clazz.newInstance();
        } catch (Exception e) {
            log.error("Failed to load HiveCatNameGetter", e);
            throw new RuntimeException(e);
        }
    }

    private String getTopicName(Configuration config) {
        return checkNotEmpty(stringProperty(config, HDW_CATNAME), "hdw.cataname is empty!") + "_fdb-" + 
                checkNotEmpty(stringProperty(config, HIVE_CLUSTER_NAME), "hive.cluster.name is empty!") + "_hms";
    }

    @Override
    public void onAlterTable(AlterTableEvent tableEvent) {
        log.debug("alter event received!");
        if (tableEvent.getStatus() &&
            filter.checkDatabase(catNameGetter.getTableCatName(tableEvent.getOldTable()), tableEvent.getOldTable().getDbName())) {
                listener.onAlterTable(tableEvent);
                log.info("Alter table " + tableEvent.getOldTable().getTableName() + " success, write to kafka!");
            }
    }

    @Override
    public void onCreateTable(CreateTableEvent tableEvent) {
        log.debug("create event received!");
        if (tableEvent.getStatus() && 
            filter.checkDatabase(catNameGetter.getTableCatName(tableEvent.getTable()), tableEvent.getTable().getDbName())){
            listener.onCreateTable(tableEvent);
            log.info("Create table " + tableEvent.getTable().getTableName() + " success, write to kafka!");
        }
    }

    @Override
    public void onDropTable(DropTableEvent tableEvent) {
        log.debug("drop event received!");
        if (tableEvent.getStatus() && 
            filter.checkDatabase(catNameGetter.getTableCatName(tableEvent.getTable()), tableEvent.getTable().getDbName())) {
            listener.onDropTable(tableEvent);
            log.info("Drop table " + tableEvent.getTable().getTableName() + " success, write to kafka!");
        }
    }

    @Override
    public void onAddPartition(AddPartitionEvent partitionEvent) {}

    @Override
    public void onDropPartition(DropPartitionEvent partitionEvent) {
        log.debug("drop event received!");
        if (partitionEvent.getStatus() && 
            filter.checkDatabase(catNameGetter.getTableCatName(partitionEvent.getTable()), partitionEvent.getTable().getDbName())) {
            listener.onDropPartition(partitionEvent);
            log.info("Drop partition of " + partitionEvent.getTable().getTableName() + " success, write to kafka!");
        }
    }

    @Override
    public void onAlterPartition(AlterPartitionEvent partitionEvent) {
        log.debug("alter partition event received!");
        if (partitionEvent.getStatus() &&
            filter.checkDatabase(catNameGetter.getTableCatName(partitionEvent.getTable()), partitionEvent.getTable().getDbName())) {
            listener.onAlterPartition(partitionEvent);
            log.info("Alter partition of " + partitionEvent.getTable().getTableName() + " success, write to kafka!");
        }
    }

    @Override
    public void onConfigChange(ConfigChangeEvent tableEvent) {}

    @Override
    public void onCreateDatabase(CreateDatabaseEvent dbEvent) {
        if (dbEvent.getStatus() &&
            filter.checkDatabase(catNameGetter.getDbCatName(dbEvent.getDatabase()), dbEvent.getDatabase().getName())) {
                // listener.onCreateDatabase(dbEvent);
                log.info("Create database " + dbEvent.getDatabase().getName() + " success!");
            }
    }

    @Override
    public void onDropDatabase(DropDatabaseEvent dbEvent) {
        log.debug("drop event received!");
        if (dbEvent.getStatus() &&
            filter.checkDatabase(catNameGetter.getDbCatName(dbEvent.getDatabase()), dbEvent.getDatabase().getName())) {
                listener.onDropDatabase(dbEvent);
                log.info("Drop database " + dbEvent.getDatabase().getName() + " success, write to kafka!");
            }
    }

}
