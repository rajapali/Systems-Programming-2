
#define _POSIX_C_SOURCE 200809L

#include "sifs-helper.h"
#include <stdio.h>
#include <string.h>

// get information about a requested file
int SIFS_fileinfo(const char *volumename, const char *pathname,
                  size_t *length, time_t *modtime) {

    uint32_t offset;                                    //The position of a file in the volume in bytes
    uint32_t nosubpaths;                                //The number of subpaths within a pathname

    char **subpath;                                     //Holds the pathnames
    char *filename;                                     //Holds the wanted filename
    SIFS_FILEBLOCK file;                                //To read file information into

    //Opens, validates and records variables from the volume
    if (open_validate_vol(volumename, "r+") == 1) {
        return 1;
    }

    //Checks if the given pathname is valid
    if (checkPathname(volumename, pathname, &subpath, &nosubpaths, 0) != 0) {

        fclose(fp);
        free(bitmap);
        free(subpath);
        return 1;
    }

    //Assigns filename from the given pathname
    if (nosubpaths > 1) {
        filename = subpath[nosubpaths - 1];
    } else {

        SIFS_errno = SIFS_EINVAL;
        fclose(fp);
        free(bitmap);
        free(subpath);
        return 1;
    }

    //Iterates through bitmap and finds file blocks
    for (int i = 0; i < nblocks; ++i) {
        if (bitmap[i] == SIFS_FILE) {

            offset = i * blocksize + sizeof(SIFS_VOLUME_HEADER) + bitmapsize;
            fseek(fp, offset, SEEK_SET);
            fread(&file, sizeof(SIFS_FILEBLOCK), 1, fp);

            for (int j = 0; j < file.nfiles; ++j) {           //Iterate through the number of files within a file block
                if (strcmp(file.filenames[j], filename) == 0) {

                    *length = file.length;                   //Records variables of the wanted file
                    *modtime = file.modtime;

                    fclose(fp);                             //Close and free variables
                    free(bitmap);
                    free(subpath);
                    free(filename);
                    return 0;

                }

            }

        }

    }

    fclose(fp);                                             //Close file free variables
    free(bitmap);
    free(subpath);
    free(filename);
    SIFS_errno = SIFS_ENOENT;                               //No such file
    return 1;
}
