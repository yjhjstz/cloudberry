package cn.cbdb.dlagent.plugins.hudi;

import cn.cbdb.dlagent.api.utilities.FragmentMetadata;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@NoArgsConstructor
@Getter
@Setter
public class HudiFileFragmentMetadata implements FragmentMetadata {

    private String fileContent;
    private String fileFormat;

    public HudiFileFragmentMetadata(String format, String content) {
        fileFormat = format;
        fileContent = content;
    }
}
