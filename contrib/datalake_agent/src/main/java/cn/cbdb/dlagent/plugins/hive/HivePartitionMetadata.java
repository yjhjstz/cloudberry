package cn.cbdb.dlagent.plugins.hive;

import lombok.Getter;
import lombok.NoArgsConstructor;
import cn.cbdb.dlagent.api.utilities.PartitionMetadata;

import java.util.ArrayList;
import java.util.List;

@NoArgsConstructor
public class HivePartitionMetadata implements PartitionMetadata  {
    @Getter
    private List<String> values = new ArrayList<>();

    public HivePartitionMetadata(List<String> values) {
        this.values = values;
    }
}
