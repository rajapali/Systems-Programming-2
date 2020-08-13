#define _POSIX_C_SOURCE 200809L

#include "sifs-helper.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int checkPathname(const char *volumename, const char *pathname, char ***subpathname, uint32_t *size, bool isNew) {


    uint32_t count = 1;                                                         //Counts the number of subpaths given
    uint32_t subpathamt = 0;                                                    //The existing number of subpaths
    uint32_t blockid = 0;                                                       //The current Block ID of the subpath
    uint32_t prevblockid;                                                       //The Block ID of the previous subpath
    uint32_t position;                                                          //The position in bytes of a subpath
    uint32_t subposition;                                                       //The position of the subpath's directory
    uint32_t blockno;                                                           //The block ID a directory's entries

    char *token;                                                                //Holds subpath name
    char *pathduplicate = strdup(pathname);                                     //Holds full pathname
    char **subpaths = malloc(strlen(pathname) * sizeof(char *));                //Holds all subpaths

    SIFS_DIRBLOCK dir;                                                          //A directory in pathname
    SIFS_DIRBLOCK subdir;                                                       //The directory's subdirectories
    SIFS_FILEBLOCK subfile;                                                     //The directory's file entries

    token = strtok(pathduplicate, "/");                                     //Divides pathname into individual subpaths
    subpaths[0] = "";                                                       //Set first subpath as root directory

    while (token != NULL) {                                                 //Add subpaths to array
        subpaths[count++] = strdup(token);
        token = strtok(NULL, "/");
    }

    //Records the needed variables
    *subpathname = subpaths;
    *size = count;

    //If pathname is just the root directory, the pathname is valid
    if (count == 1) {
        return 0;
    }

    //If the last subpath is new, do not try to find it in the volume
    if (isNew == 1) {
        subpathamt = count - 1;

    } else {
        subpathamt = count;
    }

    //For the number of subpaths determined, find and record their information
    for (int j = 1; j < subpathamt; ++j) {

        prevblockid = blockid;
        position = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + (blockid * blocksize);
        fseek(fp, position, SEEK_SET);
        fread(&dir, sizeof dir, 1, fp);

        //Iterate through the directory's entries and find their position
        for (int i = 0; i < dir.nentries; ++i) {

            blockno = dir.entries[i].blockID;
            subposition = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + blockno * blocksize;

            switch (bitmap[blockno]) {

                //If the entry is another directory, compare it with the subpath we are trying to find
                case SIFS_DIR: {
                    fseek(fp, subposition, SEEK_SET);
                    fread(&subdir, sizeof(subdir), 1, fp);
                    if (strcmp(subdir.name, subpaths[j]) == 0) {

                        //Update the current Block ID of the subpath if we have found the subpath we are looking for
                        blockid = dir.entries[i].blockID;
                    }
                    break;
                }
                case SIFS_FILE: {

                    fseek(fp, subposition, SEEK_SET);
                    fread(&subfile, sizeof(subfile), 1, fp);

                    for (int k = 0; k < subfile.nfiles; ++k) {
                        if (strcmp(subfile.filenames[k], subpaths[j]) == 0) {
                            blockid = dir.entries[i].blockID;
                            break;
                        }
                    }
                }
            }
        }
        //If the subpath cannot be found, the pathname is not valid
        if (prevblockid == blockid) {
            SIFS_errno = SIFS_ENOENT;
            return 1;
        }
    }
    free(pathduplicate);
    return 0;
}
