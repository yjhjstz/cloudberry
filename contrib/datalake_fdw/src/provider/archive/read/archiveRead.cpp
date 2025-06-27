#include "archiveRead.h"

extern "C" {
	#include "src/common/random_segment.h"
	#include "src/datalake_fragment.h"
}

namespace Datalake {
namespace Internal {

void archiveRead::createHandler(void *sstate)
{
	initParameter(sstate);
	initializeColumnValue();
	bool exec = createPolicy();
	if (exec)
		initFileStream();
	readNextFile();
}

bool archiveRead::createPolicy()
{
	bool exec = false;
	int dummy_segid = 0;
	int dummy_segnums = 0;
	datalakeExecSegment(selected_segments, segId, segnum, &exec, &dummy_segid, &dummy_segnums);
	if (!exec) {
		if (scanstate->options->hiveOption->hivePartitionKey != NULL)
		{
			scanstate->options->hiveOption->curPartition += 1;
		}
		return false;
	}

	std::vector<ListContainer> lists;
	int64_t totalsize = 0;
	if (scanstate->options->hiveOption->hivePartitionKey != NULL)
	{
		List *fragment = datalakeGetNextPartitionFragmentList(scanstate->options, &totalsize);
		extraFragmentLists(lists, fragment);
		datalakeFreeFragmentLists(fragment);
		scanstate->options->hiveOption->curPartition += 1;
	}
	else
	{
		List *fragments = datalakeGetFragmentList(scanstate->options, &totalsize);
		extraFragmentLists(lists, fragments);
		datalakeFreeFragmentLists(fragments);
	}
	readPolicy.accordingConsistentHash(dummy_segid, dummy_segnums, lists);
	blockSerial = 0;

	return exec;
}

bool archiveRead::readNextFile()
{
	if (blockSerial >= (int)readPolicy.readingLists.size())
	{
		return false;
	}
	metaInfo info = readPolicy.readingLists[blockSerial];
	curFileName = info.fileName;
	fileRead.open(fileStream, info.fileName, options);
	blockSerial++;
	return true;
}

const char* archiveRead::getReadFileName()
{
	return curFileName.c_str();
}

void archiveRead::restart()
{
	readPolicy.reset();
	initializeColumnValue();
	createPolicy();
	readNextFile();
}

int64_t archiveRead::readWithBuffer(void* buffer, int64_t length)
{
nextPartition:
	int64_t size = 0;
	if (getFileState() == FILE_OPEN)
	{
		size = fileRead.read(buffer, length);
		if (size == 0)
		{
			fileRead.close();
			if (readNextFile())
			{
				size = fileRead.read(buffer, length);
			}
		}

		if (size > 0) {
			return size;
		}
	}
	
	if (QueryFinishPending || QueryCancelPending)
	{
		return 0;
	}

	if (!datalakeIsLastPartition(scanstate))
	{
		restart();
		goto nextPartition;
	}

	return size;
}

void archiveRead::setPartitionValue(void* values, void* nulls) {
	if (scanstate->options->hiveOption->hivePartitionKey != NULL)
	{
		AttrNumber numDefaults = this->nDefaults;
		int *defaultMap = this->defMap;
		ExprState **defaultExprs = this->defExprs;

		for (int i = 0; i < numDefaults; i++)
		{
			/* only eval const expr, so we don't need pg_try catch block here */
			Datum* value = (Datum*)values;
			bool* null = (bool*)nulls;
			value[defaultMap[i]] = datalakeExecEvalConst(defaultExprs[i], NULL, &null[defaultMap[i]], NULL);
		}
	}
}

void archiveRead::destroyHandler()
{
	fileRead.close();
	releaseResources();
}

fileState archiveRead::getFileState()
{
	return fileRead.getState();
}

}
}