#ifndef DATALAKE_ARCHIVEWRITE_H
#define DATALAKE_ARCHIVEWRITE_H

#include "archiveFileWrite.h"
#include "src/provider/provider.h"


namespace Datalake {
namespace Internal {


class archiveWrite : public Provider {
public:
	void createHandler(void *sstate);

	int64_t write(const void *buf, int64_t length);

	void destroyHandler();

private:
	void setOption(dataLakeOptions *options);

	std::string generateArchiveWriteFileName(const std::string &filePath, const std::string &suffix, uint32 sliceIndex);

	uint32 sliceIndex;

	int64_t currentWriteSize;

	std::string prefix;

	std::string suffix;

	ossFileStream fileStream;

	writeOption option;

	std::string fileName;

	archiveFileWrite fileWrite;
};



}
}






#endif