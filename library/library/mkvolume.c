#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sifs-internal.h"

// make a new volume
int SIFS_mkvolume(const char *volumename, size_t blocksize, uint32_t nblocks)
{
//  ENSURE THAT RECEIVED PARAMETERS ARE VALID
    if(volumename == NULL || nblocks == 0 || blocksize < SIFS_MIN_BLOCKSIZE) {
	SIFS_errno	= SIFS_EINVAL; //invalid argument
	return 1;
    }

//  ENSURE THAT THE REQUESTED VOLUME DOES NOT ALREADY EXIST
    if(access(volumename, F_OK) == 0) { //F_OK tests for the files existence
	SIFS_errno	= SIFS_EEXIST;
	return 1;
    }

//  ATTEMPT TO CREATE THE NEW VOLUME - OPEN FOR WRITING
    FILE *vol	= fopen(volumename, "w"); //open for writing

//  VOLUME CREATION FAILED
    if(vol == NULL) {
	SIFS_errno	= SIFS_ECREATE;
	return 1;
    }

//  DEFINE AND INITIALISE VARIABLES FOR header, bitmap, and blocks
    SIFS_VOLUME_HEADER	header = {
        .blocksize	= blocksize,
        .nblocks	= nblocks,
    };

    SIFS_BIT	bitmap[nblocks]; //can hold u, d, f, b

    bitmap[SIFS_ROOTDIR_BLOCKID] = SIFS_DIR;	// the root directory = 0 --> d
    for(int b=1 ; b<nblocks ; ++b) { //sets rest to unused
        bitmap[b]	= SIFS_UNUSED;
    }

    char		oneblock[blocksize]; //initialise whats in oneblock

    SIFS_DIRBLOCK	rootdir_block; //creates new directory block
    memset(&rootdir_block, 0, sizeof rootdir_block);	// cleared to all zeroes

    rootdir_block.name[0]       = '\0';
    rootdir_block.modtime	= time(NULL);
    rootdir_block.nentries	= 0;
    memset(oneblock, 0, sizeof oneblock);        // cleared to all zeroes
    memcpy(oneblock, &rootdir_block, sizeof rootdir_block); //copy rootdir_block into oneblock?

//  WRITE ALL OF THE INITIALISED SECTIONS TO THE VOLUME
    fwrite(&header, sizeof header, 1, vol); // writes 1 header element to vol sizeof header bytes long
    fwrite(bitmap,  sizeof bitmap, 1, vol);

    fwrite(oneblock, sizeof oneblock, 1, vol);	// write rootdir to vol and then reset oneblock
    memset(oneblock, 0, sizeof oneblock);	// reset to all zeroes after being copied to vol?
    for(int b=1 ; b<nblocks ; ++b) {
        fwrite(oneblock, sizeof oneblock, 1, vol); //fill rest of vol with 0s
    }

//  FINISHED, CLOSE THE VOLUME
    fclose(vol);

//  AND RETURN INDICATING SUCCESS
    return 0;
}
