/**
 * Copyright (C) 2018-2023 Expedia, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.expediagroup.apiary.extensions.events.metastore.event;


import static com.expediagroup.apiary.extensions.events.metastore.event.CustomEventParameters.HIVE_VERSION;

import org.apache.hadoop.hive.metastore.events.AddPartitionEvent;
import org.apache.hadoop.hive.metastore.events.AlterPartitionEvent;
import org.apache.hadoop.hive.metastore.events.AlterTableEvent;
import org.apache.hadoop.hive.metastore.events.CreateTableEvent;
import org.apache.hadoop.hive.metastore.events.DropDatabaseEvent;
import org.apache.hadoop.hive.metastore.events.DropPartitionEvent;
import org.apache.hadoop.hive.metastore.events.DropTableEvent;
import org.apache.hive.common.util.HiveVersionInfo;

public class ApiaryListenerEventFactory {

  public ApiaryCreateTableEvent create(CreateTableEvent event) {
    return addParams(new ApiaryCreateTableEvent(event));
  }

  public ApiaryAlterTableEvent create(AlterTableEvent event) {
    return addParams(new ApiaryAlterTableEvent(event));
  }

  public ApiaryDropTableEvent create(DropTableEvent event) {
    return addParams(new ApiaryDropTableEvent(event));
  }

  public ApiaryAddPartitionEvent create(AddPartitionEvent event) {
    return addParams(new ApiaryAddPartitionEvent(event));
  }

  public ApiaryAlterPartitionEvent create(AlterPartitionEvent event) {
    return addParams(new ApiaryAlterPartitionEvent(event));
  }

  public ApiaryDropPartitionEvent create(DropPartitionEvent event) {
    return addParams(new ApiaryDropPartitionEvent(event));
  }

  public ApiaryDropDatabaseEvent create(DropDatabaseEvent event) {
    return addParams(new ApiaryDropDatabaseEvent(event));
  }

  private <T extends ApiaryListenerEvent> T addParams(T event) {
    event.putParameter(HIVE_VERSION.varname(), HiveVersionInfo.getVersion());
    return event;
  }

}
