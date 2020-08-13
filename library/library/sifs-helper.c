#define _POSIX_C_SOURCE 200809L

#include "sifs-helper.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

SIFS_BIT *bitmap;
size_t blocksize;
uint32_t nblocks;
FILE *fp;
size_t bitmapsize;

