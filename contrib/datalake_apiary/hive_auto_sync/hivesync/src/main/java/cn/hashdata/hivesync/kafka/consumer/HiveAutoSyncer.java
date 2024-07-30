package cn.hashdata.hivesync.kafka.consumer;

import java.io.FileInputStream;
import java.util.Properties;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;

import cn.hashdata.hivesync.kafka.prometheus.PrometheusHTTPServer;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.JsonNodeType;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.APPLICATION_NAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.BOOTSTRAP_SERVERS;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_CATNAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_DB;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_HOST;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_PORT;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_SECRET;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_USER;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CATLIST;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CATNAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CLUSTERS;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_CLUSTER_NAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_GPNAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HIVE_HDFSNAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_MECHANISM;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_SASL_JAAS_CONFIG;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.KAFKA_SECURITY;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.PROMETHEUS_PORT;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.TOPIC_NAME;
import static cn.hashdata.hivesync.kafka.consumer.HiveAutoSyncProperty.HDW_PASSWD;

public class HiveAutoSyncer implements Runnable {
    private static final Logger log = LoggerFactory.getLogger(HiveAutoSyncer.class);

    private static final String CONFIG_PATH = "hivesync.conf";

    private Properties props = new Properties();
    private List<Thread> syncerList;

    private PrometheusHTTPServer prometheusServer;

    class HiveConfig {
        private String clusterName;
        private String gpName;
        private CatalogConfig[] catList;
        private String hdfsGpName;
    }

    class CatalogConfig {
        private String catName;
        private String dbName;
    }

    private List<HiveConfig> hiveList = new LinkedList<HiveConfig>();

    private Boolean checkNodeIsNull(JsonNode node) {
        if (node == null || node.getNodeType() == JsonNodeType.NULL)
            return true;
        return false;
    }

    private String getPropertyOrNull(JsonNode root, HiveAutoSyncProperty property) {
        JsonNode node = root.get(property.key());
        if (checkNodeIsNull(node)) {
            return null;
        }
        return node.asText().trim();
    }

    private String getProperty(JsonNode root, HiveAutoSyncProperty property) {
        JsonNode node = root.get(property.key());
        if (checkNodeIsNull(node)) {
            throw new IllegalArgumentException(property.key() + " is not specified!");
        }
        return node.asText().trim();
    }

    private String[] getPropertyStringArr(JsonNode root, HiveAutoSyncProperty property) {
        JsonNode node = root.get(property.key());
        if (checkNodeIsNull(node)) {
            throw new IllegalArgumentException(property.key() + " is not specified!");
        }
        if (!node.isArray()) {
            throw new IllegalArgumentException(property.key() + " is not an array!");
        }
        String[] list = new String[node.size()];
        for (int i = 0; i < node.size(); i++) {
            list[i] = node.get(i).asText().trim();
        }
        return list;
    }

    private JsonNode[] getPropertyNodeArr(JsonNode root, HiveAutoSyncProperty property) {
        JsonNode node = root.get(property.key());
        if (checkNodeIsNull(node)) {
            throw new IllegalArgumentException(property.key() + " is not specified!");
        }
        if (!node.isArray()) {
            throw new IllegalArgumentException(property.key() + " is not an array!");
        }
        JsonNode[] list = new JsonNode[node.size()];
        for (int i = 0; i < node.size(); i++) {
            list[i] = node.get(i);
        }
        return list;
    }

    private Boolean readConfig() {
        ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
        try (FileInputStream fis = new FileInputStream(CONFIG_PATH)) {
            JsonParser parser = mapper.getFactory().createParser(fis);
            JsonNode root = mapper.readTree(parser);

            String[] serverArr = getPropertyStringArr(root, BOOTSTRAP_SERVERS);
            props.setProperty(BOOTSTRAP_SERVERS.key(), String.join(",", serverArr));

            props.setProperty(HDW_CATNAME.key(), getProperty(root, HDW_CATNAME));
            props.setProperty(HDW_HOST.key(), getProperty(root, HDW_HOST));
            props.setProperty(HDW_PORT.key(), getProperty(root, HDW_PORT));
            props.setProperty(HDW_USER.key(), getProperty(root, HDW_USER));
            props.setProperty(HDW_PASSWD.key(), getProperty(root, HDW_PASSWD));

            JsonNode[] hiveClusters = getPropertyNodeArr(root, HIVE_CLUSTERS);
            for (JsonNode hiveCluster: hiveClusters) {
                HiveConfig hiveConfig = new HiveConfig();
                hiveConfig.clusterName = getProperty(hiveCluster, HIVE_CLUSTER_NAME);
                hiveConfig.gpName = getProperty(hiveCluster, HIVE_GPNAME);
                JsonNode[] catNodeList = getPropertyNodeArr(hiveCluster, HIVE_CATLIST);
                hiveConfig.catList = new CatalogConfig[catNodeList.length];
                for (int i = 0; i < catNodeList.length; i++) {
                    CatalogConfig catConfig = new CatalogConfig();
                    catConfig.catName = getProperty(catNodeList[i], HIVE_CATNAME);
                    catConfig.dbName = getPropertyOrNull(catNodeList[i], HDW_DB);
                    hiveConfig.catList[i] = catConfig;
                }
                hiveConfig.hdfsGpName = getProperty(hiveCluster, HIVE_HDFSNAME);
                hiveList.add(hiveConfig);
            }

            String security = getPropertyOrNull(root, KAFKA_SECURITY);
            if (security != null) {
                props.setProperty(KAFKA_SECURITY.key(), security);
                if (security.equals("SASL_PLAINTEXT")) {
                    props.setProperty(KAFKA_MECHANISM.key(), getProperty(root, KAFKA_MECHANISM));
                    props.setProperty(KAFKA_SASL_JAAS_CONFIG.key(), getProperty(root, KAFKA_SASL_JAAS_CONFIG));
                }
                else if (!security.equals("PLAINTEXT")) {
                    throw new IllegalArgumentException("Invalid kafka security protocol: " + security);
                }
            }
            String hdwSecret = getPropertyOrNull(root, HDW_SECRET);
            if (hdwSecret != null) {
                props.setProperty(HDW_SECRET.key(), hdwSecret);
            }
            String prometheusPort = getPropertyOrNull(root, PROMETHEUS_PORT);
            if (prometheusPort != null) {
                props.setProperty(PROMETHEUS_PORT.key(), prometheusPort);
            }
            return true;
        } catch (Exception e) {
            log.error("Hive Auto Sync read config failed!", e);
            return false;
        }
    }

    private void work() {
        // stop thread if read config failed
        Boolean configFlag = readConfig();

        // start prometheus server
        prometheusServer = new PrometheusHTTPServer();
        prometheusServer.startHttpServer(Integer.valueOf(props.getProperty(PROMETHEUS_PORT.key(), ((Integer)PROMETHEUS_PORT.defaultValue()).toString())));

        // abort after start server
        if (!configFlag) {
            return;
        }

        syncerList = new ArrayList<Thread>(hiveList.size());
        for (int i = 0; i < hiveList.size(); i++) {
            HiveConfig config = hiveList.get(i);
            Properties syncerProps = new Properties(props);
            syncerProps.setProperty(HIVE_CLUSTER_NAME.key(), config.clusterName);
            syncerProps.setProperty(HIVE_GPNAME.key(), config.gpName);
            String[] catalogList = new String[config.catList.length];
            for (int j = 0; j < config.catList.length; j++) {
                catalogList[j] = config.catList[j].catName;
                if (config.catList[j].dbName != null) {
                    syncerProps.setProperty(HDW_DB.key() + "." + config.catList[j].catName, config.catList[j].dbName);
                }
            }
            syncerProps.setProperty(HIVE_CATLIST.key(), String.join(",", catalogList));
            syncerProps.setProperty(HIVE_HDFSNAME.key(), config.hdfsGpName);
            syncerProps.setProperty(TOPIC_NAME.key(), String.format("%s_fdb-%s_hms", props.getProperty(HDW_CATNAME.key()), config.clusterName));
            syncerProps.setProperty(APPLICATION_NAME.key(), syncerProps.getProperty(TOPIC_NAME.key()));
            syncerList.add(new Thread(new KafkaMetaStoreEventConsumer(syncerProps)));
            syncerList.get(i).start();
            log.info("Start syncer for " + config.clusterName + " ! Catalog list: " + syncerProps.getProperty(HIVE_CATLIST.key()));
        }
    }

    @Override
    public void run() {
        work();
    }

    static public void main(String[] args) {
        HiveAutoSyncer syncer = new HiveAutoSyncer();
        syncer.work();
    }
}


