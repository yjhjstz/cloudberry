#include "parquetWrite.h"
#include "src/common/util.h"
#include "src/common/fileSystemWrapper.h"


#define DATALAKE_EXPORT_NAME ("datalake")


void parquetWrite::createHandler(void *sstate)
{
	/* parquet init */ 
	ss = (dataLakeFdwScanState*)sstate;
	gopherConfig* conf = createGopherConfig((void*)(ss->options->gopher));
	fileStream = createFileSystem(conf);
	freeGopherConfig(conf);
	prefix = (char*)lfirst(list_head(ss->fragments)); 
	setOption(ss->options);
	sliceIdx= 0;
	file_writer.init(sstate, option);
}

void parquetWrite::setOption(dataLakeOptions *options)
{
	option.compression = options->compress;
	option.writeFileSize = options->fileSizeLimit;
}

std::string parquetWrite::generateParquetFileName(const std::string &filePath, uint32 fileSliceIndex)
{
	return generateWriteFileName(filePath, PARQUET_WRITE_SUFFIX, GpIdentity.segindex, fileSliceIndex);
}

int64_t parquetWrite::write(const void *buf, int64_t length)
{
	if (file_writer.isOpen() && option.writeFileSize > 0 && file_writer.getWrittenBytes() + length > option.writeFileSize)
	{
		file_writer.closeParquetWriter();
		sliceIdx += 1;
	}

	if (!file_writer.isOpen())
	{
		fileName = generateParquetFileName(prefix, sliceIdx);
		file_writer.createParquetWriter(fileStream, fileName);
	}
	int64_t len = file_writer.write(buf, length);
	return len;
}

void parquetWrite::destroyHandler()
{
	if (file_writer.isOpen())
	{
		file_writer.closeParquetWriter();
	}
	destroyFileSystem(fileStream);
	fileStream = NULL;
}