#define _POSIX_C_SOURCE 200809L
#include "sifs-helper.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// This function creates a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
	char **subpaths;			// Storing array with subpaths
	char *toadd; 				// Path to add to volume
	char *parentdir;			// Holds name of parent directory	
	uint32_t sizeofpaths;		// Stores size of path length
	uint32_t currentblock;		// Holds index location of parent
	uint32_t newcreated;		// Holds index location of new directory to be created
	uint32_t spacefound;		// Checks if space in bitmap
	uint32_t offset2;			// For offsetting in volume file
	SIFS_DIRBLOCK dircheck;		// Used to check parent directory and fields
	SIFS_DIRBLOCK newblock;		// Hold new directory to be created


	open_validate_vol(volumename,"r+");

	if(checkPathname(volumename, dirname, &subpaths, &sizeofpaths, 1)==1){	// Calls external function to verify pathname
		SIFS_errno	= SIFS_ENOTDIR;
		fclose(fp);
		free(subpaths);
		free(bitmap);
		return 1;
	}

	toadd = subpaths[sizeofpaths - 1];	//Directory to add to parent directory

    // This for-block checks if directory already exists
    spacefound = 0;
	for(int i=0;i<nblocks;i++){
	   	if(bitmap[i] == SIFS_DIR){
	    	offset2 = blocksize*(i);
			fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
			fread(&dircheck, sizeof(dircheck), 1, fp); 
			if(strcmp(dircheck.name,toadd)==0){ 		// Checks if directory already exists in bitmap
			   	SIFS_errno	= SIFS_EEXIST;
			   	fclose(fp);
			   	free(subpaths);
			   	free(toadd);
			   	free(bitmap);
			   	return 1; 								// Directory already exists!
			}
	    }
	    if(bitmap[i]==SIFS_UNUSED && spacefound==0){	// Check if space available in bitmap	  
			newcreated = i;
			spacefound = 1;			
		}	
		if(i==(nblocks-1) && bitmap[i]!=SIFS_UNUSED && spacefound!=1){
			SIFS_errno	= SIFS_ENOSPC;
			fclose(fp);
			free(subpaths);
			free(toadd);
			free(bitmap);
			return 1; 									//Bitmap full
		}
	}

	if(sizeofpaths > 2) {
        parentdir = subpaths[sizeofpaths - 2];
    }else{
        parentdir = "";			// Parent is the root directory
    }

    // This for-block finds the parent directory index
	for(int i=0;i<nblocks;i++){ 
	   	if(bitmap[i] == SIFS_DIR){
	    	offset2 = blocksize*(i);
			fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
			fread(&dircheck, sizeof(dircheck), 1, fp); 
			if(strcmp(dircheck.name,parentdir) == 0){ 	//Checks if directory already exists in bitmap
			   	currentblock = i;
			   	break;
			}
	    }
	}

    if(sizeofpaths==2){		//Adds to root if there is no parent directory
    	currentblock = 0;
	}

    offset2 = blocksize*(currentblock);		//Get parent directory
	fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
	fread(&dircheck, sizeof(dircheck), 1, fp);

	if(dircheck.nentries==SIFS_MAX_ENTRIES){
		SIFS_errno	= SIFS_EMAXENTRY;
		fclose(fp);
		free(subpaths);
		free(toadd);
		free(bitmap);
		return 1;	//No entries space available in parent directory
	}

	// This for-block finds the position to enter entry in directory block
	for(int j=0; j<SIFS_MAX_ENTRIES;j++){
		if(dircheck.entries[j].blockID==0){
			dircheck.entries[j].blockID = newcreated;
			dircheck.nentries++;
			break;
		}
   	}

   	dircheck.modtime = time(NULL);
   	fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
   	fwrite(&dircheck, sizeof(dircheck),1, fp);	//Write back parent directory to volume



   	// Initialise new directory to add to volume
    newblock.nentries = 0;
    for (int k = 0; k < SIFS_MAX_ENTRIES; ++k) {
        newblock.entries[k].fileindex = 0;
        newblock.entries[k].blockID = 0;
    }

   	bitmap[newcreated]=SIFS_DIR; 	// Update bitmap
   	offset2 = blocksize*(newcreated);	
	fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, nblocks* sizeof(SIFS_BIT),1, fp); 
    fseek(fp,offset2, SEEK_CUR);
    strcpy(newblock.name, toadd);
    newblock.modtime = time(NULL);
    fwrite(&newblock, sizeof(newblock),1, fp);	//Creates new block for directory

    fclose(fp);
    free(subpaths);
	free(toadd);
	free(bitmap);

    return 0;
}
