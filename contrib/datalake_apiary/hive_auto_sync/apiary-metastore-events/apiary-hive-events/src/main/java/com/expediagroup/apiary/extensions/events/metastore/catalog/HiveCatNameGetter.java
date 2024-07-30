package com.expediagroup.apiary.extensions.events.metastore.catalog;

import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.api.Table;

public interface HiveCatNameGetter {
    public String getTableCatName(Table table);
    public String getDbCatName(Database db);
}