#include "sifs-helper.h"

// Opens, validates and records variables in volume
int open_validate_vol(const char *volumename, const char *mode){

    size_t volumesize;                      // Size of volume
    size_t expectedvolumesize;              // Should equal volumesize
    uint32_t dirblockno = 0;                // Number of directory blocks in bitmap
    uint32_t fileblockno = 0;               // Number of file blocks in bitmap
    uint32_t datablockno = 0;               // Number of data blocks in bitmap
    uint32_t unusedblockno = 0;             // Number of unused blocks in bitmap
    uint32_t nblock_check;                  // For checking if size requirements are met
    uint32_t dirposition;                   // Directory offset
    uint32_t fileposition;                  // File offset

    SIFS_DIRBLOCK dircheck;                 // Check if directories are valid
    SIFS_FILEBLOCK filecheck;               // Check if files are valid
    SIFS_VOLUME_HEADER head;                // Checks if volume header are valid

    fp = fopen(volumename, mode);

    if(fp == NULL){     //Check if the volume exists
        SIFS_errno = SIFS_ENOVOL;
        return 1;
    }

    //Find volume size
    fseek(fp,0, SEEK_END);
    volumesize = ftell(fp);
    rewind(fp);

    //Check header parameters
    fread(&head, sizeof(head), 1, fp);

    if(head.blocksize < SIFS_MIN_BLOCKSIZE){ //Checks to see if header block size is less than minimum blocksize
        fclose(fp);
        SIFS_errno = SIFS_ENOTVOL;
        return 1;
    }else{
        blocksize = head.blocksize;
    }

    if(head.nblocks <= 0){
        fclose(fp);
        SIFS_errno = SIFS_ENOTVOL;
        return 1;
    }
    else{
        nblocks = head.nblocks;
        bitmapsize = nblocks*sizeof(SIFS_BIT); //Size of bitmap (bytes)
    }

    //Find expected volume size and check against actual volume size
    expectedvolumesize = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + nblocks*blocksize;

    if(expectedvolumesize != volumesize){
        fclose(fp);
        SIFS_errno = SIFS_ENOTVOL;
        return 1;
    }else{
        bitmap = calloc(nblocks, sizeof(SIFS_BIT*));    //Clears memory for bitmap for reading
        fread(bitmap, bitmapsize, 1, fp);
    }

    /*
    Iterate through bitmap to see if the number of directory, file, data and unused blocks sum up
    to the total number of blocks
    */
    for (int i = 0; i < nblocks; ++i) {
        switch (bitmap[i]){
            case SIFS_DIR:
                dirblockno++;
                break;
            case SIFS_FILE:
                fileblockno++;
                break;
            case SIFS_DATABLOCK:
                datablockno++;
                break;
            case SIFS_UNUSED:
                unusedblockno++;
        }
    }

    nblock_check = dirblockno + fileblockno + datablockno + unusedblockno;
    if(nblock_check != nblocks){
        fclose(fp);
        SIFS_errno = SIFS_ENOTVOL;
        return 1;
    }

    // Check if root directory blockId actually holds a directory
    if(bitmap[SIFS_ROOTDIR_BLOCKID] != SIFS_DIR){
        fclose(fp);
        SIFS_errno = SIFS_ENOTVOL;
        return 1;
    }

    // Check if file and directory blocks are valid
    for (int j = 0; j <nblocks ; ++j) {
        switch (bitmap[j]){
            case SIFS_DIR:{ // If we are dealing with a directory block
                dirposition = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + j*blocksize;
                fseek(fp, dirposition, SEEK_SET);
                fread(&dircheck, sizeof(dircheck), 1, fp);

                // Start checking if directory is valid
                if(dircheck.nentries > SIFS_MAX_ENTRIES){
                    fclose(fp);
                    SIFS_errno = SIFS_ENOTDIR;
                    return 1;
                }

                break;
            }
            case SIFS_FILE:{ // If we are dealing with a file block
                fileposition = sizeof(SIFS_VOLUME_HEADER) + bitmapsize + j*blocksize;
                fseek(fp, fileposition, SEEK_SET);
                fread(&filecheck, sizeof(filecheck), 1, fp);

                //Start checking if file is valid
                if(filecheck.length > datablockno*blocksize){
                    fclose(fp);
                    SIFS_errno = SIFS_ENOTFILE;
                    return 1;
                }

                if(filecheck.firstblockID > nblocks){
                    fclose(fp);
                    SIFS_errno = SIFS_ENOTFILE;
                    return 1;

                }

                if(filecheck.nfiles > SIFS_MAX_ENTRIES){
                    fclose(fp);
                    SIFS_errno = SIFS_ENOTFILE;
                    return 1;
                }
            }
        }
    }

    return 0; // File successfully validated
}
