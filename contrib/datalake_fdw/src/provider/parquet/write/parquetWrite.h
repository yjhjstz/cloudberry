#ifndef HDATALAKE_PARQUETWRITE_H
#define DATALAKE__PARQUETWRITE_H


#include "parquetFileWriter.h"

class parquetWrite : public Provider {

public:
    void createHandler(void *sstate);

    int64_t read(void *values, void *nulls) { return 0; };

    int64_t write(const void *buf, int64_t length);

    void destroyHandler();

private:
    void setOption(dataLakeOptions *options);

private:
    std::string generateParquetFileName(std::string filePath, uint32 fileSliceIndex);
    parquetFileWriter file_writer;
    ossFileStream fileStream;
    writeOption option;
    int sliceIdx;
    std::string prefix;
	std::string fileName;
    dataLakeFdwScanState *ss;
};

#endif
