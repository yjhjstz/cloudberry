#ifndef DELETE_BITMAP_C_H
#define DELETE_BITMAP_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"

void *datalakeCreateBitmap(void);
void datalakeBitmapAdd(void *bitmap, uint64 value);
void datalakeDestroyBitmap(void *bitmap);
bool datalakeBitmapContains(void *bitmap, uint64 value);

#ifdef __cplusplus
}
#endif

#endif // DELETE_BITMAP_C_H
