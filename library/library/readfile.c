#define _POSIX_C_SOURCE 200809L
#include "sifs-helper.h"
#include <stdio.h>
#include <string.h>

// read the contents of an existing file from an existing volume
int SIFS_readfile(const char *volumename, const char *pathname, void **data, size_t *nbytes)
{
    char **subpath;                 // Storing array with subpaths
    char* filename;                 // Filename to look at
    uint32_t size;                  // Stores size of path length    
    uint32_t blockid;               // BlockID of first data block
    uint32_t foundfile = 0;         // Checks if file is found
    uint32_t offset;                // Offset for when seeking in file
    SIFS_FILEBLOCK file;            // For file information

    if(open_validate_vol(volumename, "r") != 0){
        return 1;
    }

    if(checkPathname(volumename, pathname, &subpath, &size, 0) != 0){
        fclose(fp);
        free(subpath);
        free(bitmap);
        return 1;
    }

    filename = subpath[size - 1];

    //Look for file
    for (int i = 0; i < nblocks; ++i) {
        if(bitmap[i] == SIFS_FILE && !foundfile){
            offset = i*blocksize + sizeof(SIFS_VOLUME_HEADER) + bitmapsize;
            fseek(fp, offset, SEEK_SET);
            fread(&file, sizeof file, 1, fp);
            for (int j = 0; j < file.nfiles; ++j) {
                if(strcmp(file.filenames[j],filename) == 0){  // Found the file we're after
                    blockid = file.firstblockID;
                    *nbytes = file.length;
                    foundfile = 1;
                    break;
                }
            }
        }
    }

    if(!foundfile){ //File is not found
        fclose(fp);
        SIFS_errno = SIFS_ENOENT;
        free(subpath);
        free(bitmap);
        return 1;
    }

    //Reads data from file
    *data =malloc(*nbytes*sizeof(char) + 1);
    offset = sizeof(SIFS_VOLUME_HEADER) + nblocks* sizeof(SIFS_BIT) + blockid*blocksize;
    fseek(fp, offset, SEEK_SET);
    fread(*data,*nbytes, 1, fp);
    fclose(fp);
    free(subpath);
    return 0;
}
