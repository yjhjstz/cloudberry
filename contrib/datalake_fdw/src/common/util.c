#include "postgres.h"
#include "postmaster/postmaster.h"
#include "miscadmin.h"
#include "util.h"

char *strConvertLow(char *str) {
	if (str == NULL)
	{
		return NULL;
	}
	char *orign = str;
	for (; *str != '\0'; str++)
	{
		*str = tolower(*str);
	}
	return orign;
}

void DatalakeGetGopherSocketPath(char *dest)
{
	snprintf(dest, 1024, "/tmp/.s.gopher.%d", PostPortNumber);
}

void DatalakeGetGopherPlasmaSocketPath(char *dest)
{
	snprintf(dest, 1024, "/tmp/.s.gopher.plasma.%d", PostPortNumber);
}

void DatalakeGetGopherMetaPath(char *dest)
{
	sprintf(dest, "%s/%s", DataDir, DATALAKE_GOPHERMETA_FOLDER);
}