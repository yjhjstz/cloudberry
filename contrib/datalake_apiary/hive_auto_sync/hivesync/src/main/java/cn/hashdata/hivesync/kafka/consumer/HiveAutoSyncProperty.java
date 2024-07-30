package cn.hashdata.hivesync.kafka.consumer;

import com.expediagroup.apiary.extensions.events.metastore.common.Property;

public enum HiveAutoSyncProperty implements Property {
  BOOTSTRAP_SERVERS("bootstrap.servers", null),
  TOPIC_NAME("topic.name", null),
  APPLICATION_NAME("application.name", null),
  HDW_CATNAME("hdw.catalog.name", null),
  HDW_USER("hdw.user",null),
  HDW_PASSWD("hdw.password",null),
  HDW_DB("hdw.database", null),
  HDW_SECRET("hdw.encryptor.password", null),
  HDW_HOST("hdw.host", null),
  HDW_PORT("hdw.port", null),
  HIVE_CLUSTER_NAME("hive.cluster.name", null),
  HIVE_GPNAME("hive.gp.name", null),
  HIVE_CATLIST("hive.catalog.list", null),
  HIVE_CATNAME("hive.catalog.name", null),
  HIVE_CATALOG("hive.catalog", null),
  HIVE_HDFSNAME("hdfs.gp.name", null),
  KAFKA_SECURITY("security.protocol", "PLAINTEXT"),
  KAFKA_MECHANISM("sasl.mechanism", null),
  KAFKA_SASL_JAAS_CONFIG("sasl.jaas.config",null),
  PROMETHEUS_PORT("prometheus.port", 15888),
  MAX_POLL_INTERVAL_MS("max.poll.interval.ms",300000),
  MAX_POLL_RECORDS("max.poll.records", 500),
  HIVE_CLUSTERS("hive.clusters", null);

  private final String unprefixedKey;
  private final Object defaultValue;

  HiveAutoSyncProperty(String unprefixedKey, Object defaultValue) {
    this.unprefixedKey = unprefixedKey;
    this.defaultValue = defaultValue;
  }

  @Override
  public String key() {
    return unprefixedKey;
  }

  @Override
  public String unprefixedKey() {
    return unprefixedKey;
  }

  @Override
  public Object defaultValue() {
    return defaultValue;
  }

  @Override
  public String toString() {
    return key();
  }

}
