#ifndef DATALAKE_UTIL_H
#define DATALAKE_UTIL_H

#include <ctype.h>  /* for tolower */

#define DATALAKE_GOPHERMETA_FOLDER "gophermeta"

char *strConvertLow(char *str);

void DatalakeGetGopherSocketPath(char *dest);

void DatalakeGetGopherPlasmaSocketPath(char *dest);

void DatalakeGetGopherMetaPath(char *dest);

#endif