#define _POSIX_C_SOURCE 200809L

#include "sifs-helper.h"
#include <stdio.h>
#include <string.h>

// get information about a requested directory
int SIFS_dirinfo(const char *volumename, const char *pathname,
                 char ***entrynames, uint32_t *nentries, time_t *modtime) {

    uint32_t currentblock;                                              //The current directory block we are examining
    uint32_t dirposition;                                               //The position in bytes of the current directory
    uint32_t block;                                                     //The block ID of the current entry
    uint32_t entrypos;                                                  //The position in bytes of the entry
    uint32_t fileindex;                                                 //The file index of the current file entry
    uint32_t nosubpaths;                                                //The number of subpaths given

    char **subpath;                                                     //Holds the subpath names
    char *dirname;                                                      //The directory name wanted
    SIFS_DIRBLOCK directory;                                            //The wanted directory block
    SIFS_DIRBLOCK subdir;                                               //Records information about directory entries
    SIFS_FILEBLOCK subfile;                                             //Records information about file entries

    char **entries = malloc(SIFS_MAX_ENTRIES*sizeof(char*));

   if(open_validate_vol(volumename, "r") != 0){                      //Opens, validates and records values from volume
        return 1;
   }

   if(checkPathname(volumename, pathname, &subpath, &nosubpaths, 0) != 0){      //Checks is pathname given is valid
       fclose(fp);
       free(subpath);
       free(bitmap);
       return 1;
   }

    if(nosubpaths == 1){                                               //Sets directory name based off pathname given
        dirname = "";
    }else if( nosubpaths > 1){
        dirname = subpath[nosubpaths-1];
    }

    for (int i = 0; i < nblocks ; ++i) {                            //Iterate through bitmap and find directory blocks

        if(bitmap[i] == SIFS_DIR){

            currentblock = i;
            dirposition = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + i*blocksize;
            fseek(fp, dirposition, SEEK_SET);
            fread(&directory, sizeof(directory), 1, fp);       //Read directory block and compare with wanted directory

            if(strcmp(directory.name, dirname) == 0){         //If the wanted directory has been found record variables

                *nentries = directory.nentries;
                *modtime = directory.modtime;

                for (int j = 0; j < SIFS_MAX_ENTRIES; ++j) {  //Iterate through and find the wanted directory's entries

                    block = directory.entries[j].blockID;
                    entrypos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + block*blocksize;
                    fseek(fp, entrypos, SEEK_SET);

                    if(block != currentblock){

                        switch(bitmap[block]){

                            case SIFS_DIR:                                  //If entry is a directory, record its name
                                fread(&subdir, sizeof(subdir), 1, fp);
                                entries[j] = strdup(subdir.name);
                                break;
                            case SIFS_FILE:                                //If entry is a file, record its name
                                fread(&subfile, sizeof(subfile), 1, fp);
                                fileindex = directory.entries[j].fileindex;
                                entries[j] = strdup(subfile.filenames[fileindex]);
                                break;
                        }
                    }
                }
                *entrynames = entries;
                fclose(fp);
                free(subpath);
                free(bitmap);
                return 0;
            }
        }
    }


    fclose(fp);
    SIFS_errno = SIFS_ENOENT;   //Entry is not a volume
    free(subpath);
    free(bitmap);
    return 1;
}
