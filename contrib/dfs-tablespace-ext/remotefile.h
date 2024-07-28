#include "storage/ufile.h"
#include "utils/guc.h"

#define REMOTE_FILE_BLOCK_SIZE (1024 * 1024 * gopher_local_blocksize_mb)

extern FileAm* remote_file_handler(void);