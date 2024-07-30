package com.expediagroup.apiary.extensions.events.metastore.catalog;

import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.api.Table;

public class Hive2xCatNameGetter implements HiveCatNameGetter{

    @Override
    public String getTableCatName(Table table) {
        return "hive";
    }

    @Override
    public String getDbCatName(Database db) {
        return "hive";
    }

}
