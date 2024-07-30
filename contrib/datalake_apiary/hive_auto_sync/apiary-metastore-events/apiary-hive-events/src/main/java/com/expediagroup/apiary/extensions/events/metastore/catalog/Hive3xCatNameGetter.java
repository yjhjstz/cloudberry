package com.expediagroup.apiary.extensions.events.metastore.catalog;

import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.api.Table;

public class Hive3xCatNameGetter implements HiveCatNameGetter{

    @Override
    public String getTableCatName(Table table) {
        return table.getCatName();
    }

    @Override
    public String getDbCatName(Database db) {
        return db.getCatalogName();
    }
    
}
