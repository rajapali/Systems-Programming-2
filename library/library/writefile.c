#define _POSIX_C_SOURCE 200809L

#include "sifs-helper.h"
#include <stdbool.h>
#include <string.h>


// add a copy of a new file to an existing volume
int SIFS_writefile(const char *volumename, const char *pathname,
                   void *data, size_t nbytes) {

    uint32_t nosubpaths;                                              //The number of subpaths given
    uint32_t noblocks = 0;                                            //The number of block the data occupies
    uint32_t firstBlockID = 0;                                        //The first block ID of the data
    uint32_t memspace = 0;                                            //A number of contiguous unused blocks in volume
    uint32_t fileIndex = 0;                                           //The file index of the file to be written
    uint32_t fileID = 0;                                              //The block ID of the file to be written
    uint32_t offset = 0;                                              //The position in bytes of a file block
    uint32_t bitmapPos = 0;                                           //The position in bytes of the bitmap
    uint32_t dirPos = 0;                                              //Position of parent directory
    uint32_t newfilePos = 0;                                          //The position of the new file block to be added
    bool fileExists = 0;                                              //Wether the file already exists or not

    char *subdir;                                                     //Holds parent directory
    char **subpath;                                                   //Holds all subpaths for pathname given
    char *MD5digest;                                                  //MD5 of data needed to be written
    char *filename;                                                   //Name of file
    unsigned char MD5result[MD5_BYTELEN];                             //MD5 result of the data

    SIFS_FILEBLOCK file;                                              //Holds information of file block with the same MD5
    SIFS_FILEBLOCK newfile;                                           //To create a new file block if the file doesn't already exist
    SIFS_DIRBLOCK directory;                                          //Hold parent directory information

    //Opens, validates and records variables from volume
    if(open_validate_vol(volumename, "r+") != 0){
        return 1;
    }

    //Checks if pathname already exists
    if(checkPathname(volumename, pathname, &subpath, &nosubpaths, 0) == 0){
        SIFS_errno = SIFS_EEXIST;
        fclose(fp);
        free(subpath);
        return 1;
    }

    //Checks if there is a file with the same name already
    if (checkPathname(volumename, pathname, &subpath, &nosubpaths, 1) != 0){
        fclose(fp);
        free(subpath);
        return 1;
    }

    //Determine filename and parent directory
    if( nosubpaths > 2){
        filename = subpath[nosubpaths-1];
        subdir = subpath[nosubpaths - 2];
    }else if (nosubpaths > 1 ) {
        filename = subpath[nosubpaths - 1];
        subdir = "";
    }

    //Find MD5 of contents
    MD5_buffer(data, nbytes, MD5result);
    MD5digest = MD5_format(MD5result);

    //Look at MD5 of all files and see if it matches any other file
    for (int i = 0; i < nblocks; ++i) {
        if (bitmap[i] == SIFS_FILE) {

            offset = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + i * blocksize;
            fseek(fp, offset, SEEK_SET);
            fread(&file, sizeof(file), 1, fp);

            if (memcmp(file.md5, MD5result, MD5_BYTELEN) == 0) {            //File already exists
                fileExists = 1;
                strcpy(file.filenames[file.nfiles], filename);              //Add to file names of the file block
                fileIndex = file.nfiles;                                    //Record and update file block variables
                fileID = i;
                file.nfiles++;

                fseek(fp, offset, SEEK_SET);
                fwrite(&file, sizeof file, 1, fp);                          //Write updated file back to volume
                break;
            }
        }
    }

    //Add file in the case that it is not a copy
    if (!fileExists) {

        //Find the number of blocks the data needs to occupy
        if (nbytes % blocksize != 0) {
            noblocks = (int) (nbytes / blocksize + 1);
        } else {
            noblocks = (int) nbytes / blocksize;
        }

        //Iterate through bitmap to see if there is enough space for a new file block
        for (int j = 0; j < nblocks; ++j) {

            if (bitmap[j] == SIFS_UNUSED) {
                bitmap[j] = SIFS_FILE;
                fileID = j;
                break;
            }
        }

        //Iterate through bitmap to see if there is enough space for the data
        for (int i = 0; i < nblocks; ++i) {
            if (memspace == noblocks) {                                 //If there is sufficient space
                for (int j = 0; j < noblocks; ++j) {
                    bitmap[firstBlockID + j] = SIFS_DATABLOCK;          //Assign the needed unused blocks to data blocks
                }
                break;

            } else if (bitmap[i] == SIFS_UNUSED && memspace == 0) {     //If the first contiguous unused block is found
                firstBlockID = i;                                       //Record the block
                memspace++;

            } else if (bitmap[i] == SIFS_UNUSED && memspace != noblocks) { //If more contiguous unused blocks are found
                memspace++;                                                //Increase the amount of space found

            } else if (bitmap[i] != SIFS_UNUSED) {                         //Blocks are no longer contiguous

                memspace = 0;
            }
        }


        // In the case that memory allocation fails
        if (memspace != noblocks) {
            SIFS_errno = SIFS_ENOMEM;
            return 1;
        }

        //If memory allocation is successful, write new bitmap into file
        bitmapPos = sizeof(SIFS_VOLUME_HEADER);
        fseek(fp, bitmapPos, SEEK_SET);
        fwrite(bitmap, bitmapsize, 1, fp);

        //Initialise new file block
        strcpy(newfile.filenames[0], filename);
        memcpy(newfile.md5, MD5result, MD5_BYTELEN);
        newfile.nfiles = 1;
        newfile.firstblockID =firstBlockID;
        newfile.length = nbytes;
        newfile.modtime = time(NULL);

        //Add file block to volume if it does not exist
        newfilePos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + fileID*blocksize;
        fseek(fp, newfilePos, SEEK_SET);
        fwrite(&newfile, sizeof(newfile), 1, fp);

        //Add data to volume if file does not already exist
        int dataOffset = sizeof(SIFS_VOLUME_HEADER) + nblocks* sizeof(SIFS_BIT) + firstBlockID*blocksize;
        fseek(fp, dataOffset, SEEK_SET);
        fwrite(data, nbytes, 1, fp);
    }

    //Add file entry into parent directory
    //Iterate through bitmap to find parent directory
    for (int k = 0; k < nblocks; ++k) {
        if (bitmap[k] == SIFS_DIR) {

            dirPos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + k * blocksize;
            fseek(fp, dirPos, SEEK_SET);
            fread(&directory, sizeof(directory), 1, fp);
            int nentries = directory.nentries;

            if (strcmp(directory.name, subdir) == 0) {                                      //Found parent directory

                //Modify parent directory variables
                directory.entries[nentries].blockID = fileID;
                directory.entries[nentries].fileindex = fileIndex;
                directory.modtime = time(NULL);
                directory.nentries++;

                //Write updated directory back into volume
                fseek(fp, dirPos, SEEK_SET);
                fwrite(&directory, sizeof(directory), 1, fp);
                break;
            }
        }

    }
    free(subpath);
    fclose(fp);

    return 0;
}
