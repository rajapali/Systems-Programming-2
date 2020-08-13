#include "sifs-helper.h"
#include <string.h>


// remove an existing file from an existing volume
int SIFS_rmfile(const char *volumename, const char *pathname) {
    char **subpath;                             // Holds the subpath names
    char *filename;                             // Filename to remove
    char *parentdir;                            // Holds parent directory name
    char oneblock[blocksize];                   // Size of one block
    uint32_t size;                              // Holds size of the path
    uint32_t fileblockID;                       // Holds block index of file to be removed
    uint32_t fileindex = 0;                     // Holds index of current file we wish to remove incase of duplicates
    uint32_t nfiles = 0;                        // Number of files of wanted file
    uint32_t noblocks = 0;                      // For determining number of blocks to occupy
    uint32_t firstblock;                        // Records first file block
    uint32_t length;                            // Holds file length
    uint32_t iscopy = 0;                        // Checks if file is a copy of other file
    uint32_t file_pos;                          // File offset
    uint32_t dataoffset;                        // Data offset
    uint32_t bitmap_pos;                        // Bitmap offset
    uint32_t dirposition;                       // Directory offset

    SIFS_FILEBLOCK file;                        // Holds file block information
    SIFS_DIRBLOCK dir;                          // Holds parent directory information

    if (open_validate_vol(volumename, "r+") != 0) {
        return 1;
    }

    if (checkPathname(volumename, pathname, &subpath, &size, 0) != 0) {
        fclose(fp);
        free(subpath);
        free(bitmap);
        return 1;
    }

    //Find file and parent directory if they exist
    filename = subpath[size - 1];
    if(strcmp(filename, "") == 0){
        fclose(fp);
        SIFS_errno = SIFS_ENOTFILE;
        free(subpath);
        free(bitmap);
        return 1;
    }
    if (size > 1) {
        parentdir = subpath[size - 2];
    } else {
        parentdir = "\0";
    }

    //Find file in the volume
    for (int i = 0; i < nblocks; ++i) {

        if (bitmap[i] == SIFS_FILE) {
            file_pos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + i * blocksize;           // Find position of file
            fseek(fp, file_pos, SEEK_SET);
            fread(&file, sizeof(file), 1, fp);                                           // Read the file found

            for (int j = 0; j < file.nfiles; ++j) {
                if (strcmp(filename, file.filenames[j]) == 0) {                          // Found the file we're looking for
                    fileindex = j;
                    fileblockID = i;                                                     //Record the blockID of fileblock
                    firstblock = file.firstblockID;
                    length = file.length;
                    nfiles = file.nfiles;

                    if (fileindex < nfiles - 1) {                                        // If that wasn't the last entry
                        for (int k = j; k < nfiles - 1; k++) {                           // Shift entries up
                            strcpy(file.filenames[k], file.filenames[k + 1]);
                        }
                        file.filenames[nfiles - 1][0] = '\0';                       
                    }

                    file.nfiles--;                                                       // Decrement number of files with the same data
                    nfiles = file.nfiles;
                    break;
                }
            }
        }
    }

    //-------------------------------------IF FILE COPIES STILL EXIST-------------------------------------------------

    if (nfiles > 0) {                                                                   // If there are still some file copies left
        iscopy = 1;
        file_pos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + fileblockID * blocksize;
        fseek(fp, file_pos, SEEK_SET);
        fwrite(&file, sizeof(file), 1, fp);                                             // Write updated file back into volume

    }
        //----------------------------------IF THAT WAS THE ONLY FILE-----------------------------------------------
    else if(nfiles == 0 && bitmap[fileblockID] == SIFS_FILE) {

        // Prepare to remove file data
        if (length % blocksize != 0) {                                                  //Determine the number of blocks the data occupies
            noblocks = (int) (length / blocksize + 1);
        } else {
            noblocks = (int) length / blocksize;
        }

        // Update bitmap
        bitmap[fileblockID] = SIFS_UNUSED;
        for (int i = 0; i < noblocks; i++) {
            bitmap[firstblock + i] = SIFS_UNUSED;
        }

        // Find data and clear it
        dataoffset = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + firstblock * blocksize;
        fseek(fp, dataoffset, SEEK_SET);
        memset(oneblock, 0, sizeof(oneblock));
        fwrite(oneblock, blocksize, 3, fp);

        // Find file block and clear it
        file_pos = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + fileblockID * blocksize;
        fseek(fp, file_pos, SEEK_SET);
        fwrite(oneblock, sizeof(file), 1, fp);

        // Write new bitmap into file
        bitmap_pos = sizeof(SIFS_VOLUME_HEADER);
        fseek(fp, bitmap_pos, SEEK_SET);
        fwrite(bitmap, bitmapsize, 1, fp);

    }

    //-------------------------------------REMOVE ENTRY FROM PARENT DIRECTORY------------------------------------------

    for(int l = 0; l < nblocks; ++l){

        if (bitmap[l] == SIFS_DIR) {
            dirposition = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + l * blocksize;
            fseek(fp, dirposition, SEEK_SET);
            fread(&dir, sizeof(dir), 1, fp);

            if (strcmp(parentdir, dir.name) == 0) { // Found parent directory

                for (int i = 0; i < dir.nentries; ++i) {
                    
                    if ((dir.entries[i].blockID == fileblockID && iscopy && dir.entries[i].fileindex == nfiles) ||
                    ( !iscopy && dir.entries[i].blockID == fileblockID && dir.entries[i].fileindex == fileindex)) { //Found the entry

                        if (i < dir.nentries - 1) {  //If that wasn't the last entry
                            for (int k = i; k < dir.nentries-1; k++) {  //Shift entries up
                                dir.entries[k].fileindex = dir.entries[k+1].fileindex;
                                dir.entries[k].blockID = dir.entries[k+1].blockID;
                            }
                        }

                        dir.entries[dir.nentries - 1].blockID = 0;
                        dir.entries[dir.nentries - 1].fileindex = 0;
                        dir.nentries--;
                        //Write updated directory back into volume
                        fseek(fp, dirposition, SEEK_SET);
                        fwrite(&dir, sizeof(dir), 1, fp);
                        fclose(fp);
                        free(subpath);
                        free(bitmap);
                        return 0;
                    }
                }
            }
        }
    }

    fclose(fp);
    SIFS_errno = SIFS_ENOTFILE;
    free(subpath);
    free(bitmap);
    return 1;
}
