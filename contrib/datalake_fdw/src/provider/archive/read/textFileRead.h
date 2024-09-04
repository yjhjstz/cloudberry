#ifndef DATALAKE_TEXTFILEREAD_H
#define DATALAKE_TEXTFILEREAD_H


#include "lineRecordReader.h"
// #include "src/Provider/Provider.h"
// #include "src/common/readPolicy.h"
#include "textFileReadPolicy.h"

namespace Datalake {
namespace Internal {

class textFileRead : public Provider, public readLogical {
public:

    virtual void createHandler(void *sstate);

    virtual int64_t readWithBuffer(void* buffer, int64_t length);

    virtual void destroyHandler();

    virtual void setPartitionValue(void* values, void* nulls);

    virtual const char* getReadFileName();

private:

    void restart();

    void initializeDataStructures(dataLakeFdwScanState *ss);

	void createFileStream(dataLakeFdwScanState *ss);

    void allocateDataBuffer();

	bool createPolicy();

    int64_t readWithBufferInternal(void* buffer, int64_t length);

    int64_t readForFile(void* buffer, int64_t length);

    int64_t readForBlock(void* buffer, int64_t length);

    int64_t readForSnappyFile(void* buffer, int64_t length);

    int64_t readForDeflateFile(void* buffer, int64_t length);

    int64_t readDefaultForBlock(void* buffer, int64_t length);

    int64_t readCustomForBlock(void* buffer, int64_t length);

    bool openTextFile(metaInfo info);

    bool readNextGroup();

    bool readNextFile();

    bool getNextGroup();

    void updateMetaInfo(metaInfo info);

    int serial;

    std::shared_ptr<textFileInput> reader;

    textFileReadPolicy readPolicy;

    // FileScanDesc scan;
    ossFileStream fileStream;
    readOption options;
    metaInfo curMetaInfo;
    std::string curFileName;
    int64_t blockOffset;
    bool firstBlock = false;
    bool isAllowSeek = false;
    std::shared_ptr<lineRecordReader> lineReader;
    datalakeMemoryBuffer memory_buffer;

    char* dataBuffer;
    int64_t dataLength;
    bool skip_first_line;
    bool last;
    int64_t index;
    int segid;

    char* recordDelimiterBytes;
    int skipNum;

};


}
}

#endif