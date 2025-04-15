package cn.cbdb.hivesync.kafka.prometheus;

import io.micrometer.core.instrument.Counter;
import io.micrometer.prometheus.PrometheusMeterRegistry;

public class HiveSyncMetrics {
    private static final PrometheusMeterRegistry prometheusRegistry = new PrometheusMeterRegistry(io.micrometer.prometheus.PrometheusConfig.DEFAULT);

    private static final Counter connectorCounts;
    private static final Counter syncTableCounts;
    private static final Counter syncTableFailCounts;
    private static final Counter syncPartitionCounts;
    private static final Counter syncPartitionFailCounts;
    private static final Counter syncDatabaseCounts;
    private static final Counter syncDatabaseFailCounts;

    static {
        connectorCounts = Counter.builder("hivesync.connector.counts")
                            .description("The number of connectors")
                            .register(prometheusRegistry);
        syncTableCounts = Counter.builder("hivesync.event.table.counts")
                            .tag("state", "success")
                            .description("The number of tables synced")
                            .register(prometheusRegistry);
        syncTableFailCounts = Counter.builder("hivesync.event.table.counts")
                            .tag("state", "fail")
                            .description("The number of tables failed to sync")
                            .register(prometheusRegistry);
        syncPartitionCounts = Counter.builder("hivesync.event.partition.counts")
                            .tag("state", "success")
                            .description("The number of partitions synced")
                            .register(prometheusRegistry);
        syncPartitionFailCounts = Counter.builder("hivesync.event.partition.counts")
                            .tag("state", "fail")
                            .description("The number of partitions failed to sync")
                            .register(prometheusRegistry);
        syncDatabaseCounts = Counter.builder("hivesync.event.database.counts")
                            .tag("state", "success")
                            .description("The number of databases synced")
                            .register(prometheusRegistry);
        syncDatabaseFailCounts = Counter.builder("hivesync.event.database.counts")
                            .tag("state", "fail")
                            .description("The number of databases failed to sync")
                            .register(prometheusRegistry);
    }

    public static PrometheusMeterRegistry getPrometheusRegistry() {
        return prometheusRegistry;
    }

    public static void incrementConnectorCounts() {
        connectorCounts.increment();
    }

    public static void incrementSyncTableCounts(Boolean flag){
        if (flag)
            syncTableCounts.increment();
        else
            syncTableFailCounts.increment();
    }

    public static void incrementSyncPartitionCounts(Boolean flag) {
        if (flag)
            syncPartitionCounts.increment();
        else
            syncPartitionFailCounts.increment();
    }

    public static void incrementSyncDatabaseCounts(Boolean flag) {
        if (flag)
            syncDatabaseCounts.increment();
        else
            syncDatabaseFailCounts.increment();
    }
}
