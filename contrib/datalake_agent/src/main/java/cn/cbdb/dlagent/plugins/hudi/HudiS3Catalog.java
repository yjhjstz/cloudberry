package cn.cbdb.dlagent.plugins.hudi;

import cn.cbdb.dlagent.api.model.Metadata;
import cn.cbdb.dlagent.api.model.RequestContext;
import cn.cbdb.dlagent.api.security.SecureLogin;
import org.apache.hudi.common.table.HoodieTableMetaClient;
import org.apache.hudi.common.table.TableSchemaResolver;
import org.apache.hudi.common.util.collection.Pair;
import org.apache.hudi.internal.schema.InternalSchema;
import org.apache.hudi.internal.schema.convert.AvroInternalSchemaConverter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Implementation of HudiCatalog for tables handled by Hudi's Catalogs API.
 */
public class HudiS3Catalog extends HudiBaseCatalog implements HudiCatalog {

    private static final Logger LOG = LoggerFactory.getLogger(HudiS3Catalog.class);

    public HudiS3Catalog(HoodieTableMetaClient metaClient, SecureLogin secureLogin) {
        super(metaClient, secureLogin);
    }

    @Override
    public Pair<HudiTableOptions, List<CombineHudiSplit>> getSplits(Metadata.Item tableName, RequestContext context) throws Exception {
        return buildInputSplits(tableName, context);
    }

    @Override
    public InternalSchema getSchema(Metadata.Item tableName) throws Exception {
        return getTableSchema();
    }

    @Override
    public HudiFileIndex createFileIndex(HudiPartitionPruner.PartitionPruner partitionPruner,
                                         DataPruner dataPruner,
                                         RequestContext context,
                                         Metadata.Item tableName) throws Exception {
        return HudiFileIndex.builder()
                    .path(metaClient.getBasePathV2())
                    .context(context)
                    .dataPruner(dataPruner)
                    .partitionPruner(partitionPruner)
                    .secureLogin(secureLogin)
                    .build();
    }
}

