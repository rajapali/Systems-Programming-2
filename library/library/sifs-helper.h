
#include "../sifs.h"
#include "md5.h"
#include "sifs-internal.h"
#include <stdio.h>
#include <stdbool.h>

extern SIFS_BIT *bitmap;

extern size_t blocksize;

extern uint32_t nblocks;

extern FILE* fp;

extern size_t bitmapsize;

extern int openVolume(const char *volname, const char* mode);

extern int checkPathname(const char *volumename, const char *pathname, char ***subpathname, uint32_t *size, bool isNew);

extern int open_validate_vol(const char *volumename, const char *mode);

