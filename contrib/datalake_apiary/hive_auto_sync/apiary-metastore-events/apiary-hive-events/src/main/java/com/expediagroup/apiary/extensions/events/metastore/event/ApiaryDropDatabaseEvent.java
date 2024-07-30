package com.expediagroup.apiary.extensions.events.metastore.event;

import java.util.Objects;

import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.events.DropDatabaseEvent;

public class ApiaryDropDatabaseEvent extends ApiaryListenerEvent {
    private static final long serialVersionUID = 1L;

  private Database database;

  ApiaryDropDatabaseEvent() {}

  public ApiaryDropDatabaseEvent(DropDatabaseEvent event) {
    super(event);
    database = event.getDatabase();
  }

  @Override
  public String getDatabaseName() {
    return database.getName();
  }

  @Override
  public String getTableName() {
    return "*";
  }

  public Database getDatabase() {
    return database;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj) {
      return true;
    }
    if (!(obj instanceof ApiaryDropDatabaseEvent)) {
      return false;
    }
    ApiaryDropDatabaseEvent other = (ApiaryDropDatabaseEvent) obj;
    return super.equals(other) && Objects.equals(database, other.database);
  }

}
