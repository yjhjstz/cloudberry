package cn.cbdb.apiary.kafka.listener;

import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.SYNC_CATALOG;
import static com.expediagroup.apiary.extensions.events.metastore.kafka.messaging.KafkaProducerProperty.SYNC_DB;

import java.util.LinkedHashSet;
import java.util.Set;
import org.apache.hadoop.conf.Configuration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class HiveMetaStoreEventFilter {
    private static final Logger log = LoggerFactory.getLogger(HiveMetaStoreEventFilter.class);

    private Set<String> databaseSet = new LinkedHashSet<String>();
    private Set<String> catalogSet = new LinkedHashSet<String>();
    private Boolean syncAllFlag = false;

    public HiveMetaStoreEventFilter(Configuration conf) {
        String catString = conf.get(SYNC_CATALOG.key());
        if (catString == null || catString.trim().isEmpty()) {
            log.error("No catalog is set in configuration!");
            return;
        }
        if ("*".equals(catString)) {
            syncAllFlag = true;
            log.info("syncAllFlag is set to true!");
        } else {
            String[] catList = catString.split(",");
            for (String catalog : catList) {
                String catName = catalog.trim();
                if (!catName.isEmpty()) {
                    String dbString = conf.get(SYNC_DB.key() + "." + catName, (String)SYNC_DB.defaultValue());
                    if (dbString != null && !"*".equals(dbString)) {
                        String[] dbList = dbString.split(",");
                        for (String db : dbList) {
                            databaseSet.add(catName + '.' + db.trim());
                        }
                    }
                    else {
                        dbString = "*";
                        catalogSet.add(catName);
                    }
                    log.info("add filter for catalog: " + catName + " with db: " + dbString);
                }
            }
        }
    }

    public HiveMetaStoreEventFilter() {}

    public Boolean checkDatabase(String catalogName, String databaseName) {
        if (syncAllFlag)
            return true;
        if (catalogSet.contains(catalogName))
            return true;
        if (databaseSet.contains(catalogName + '.' + databaseName))
            return true;
        return false;
    }
}
