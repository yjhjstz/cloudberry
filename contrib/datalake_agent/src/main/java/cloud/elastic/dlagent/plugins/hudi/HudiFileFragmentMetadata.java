package cloud.elastic.dlagent.plugins.hudi;

import cloud.elastic.dlagent.api.utilities.FragmentMetadata;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@NoArgsConstructor
@Getter
@Setter
public class HudiFileFragmentMetadata implements FragmentMetadata {

    private String fileContent;
    private String fileFormat;
    private Long fileSize;

    public HudiFileFragmentMetadata(String format, String content, Long size) {
        fileFormat = format;
        fileContent = content;
        fileSize = size;
    }
}
