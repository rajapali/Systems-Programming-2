#define _POSIX_C_SOURCE 200809L
#include "sifs-helper.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


// This function remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *dirname)
{
	char **subpaths;			// Storing array with subpaths
	char *par_dir;				// Holds name of parent directory
	char *directoryname;		// Holds name of directory to be removed
	uint32_t sizeofpaths;		// Stores size of path length	
	uint32_t deldirblock=0;		// Holds index for directory to remove
	uint32_t offset2;			// For offsetting in volume file
	SIFS_DIRBLOCK checkdir;		// For checking and finding directory in volume
	SIFS_DIRBLOCK parentdir;	// For updating parent directory

	open_validate_vol(volumename,"r+");	//Opens volume and sets required variables

	if(checkPathname(volumename, dirname, &subpaths, &sizeofpaths, 0)==1){	// Calls external function to verify pathname
		SIFS_errno = SIFS_ENOTDIR;	//Pathname does not exist
		fclose(fp);
		free(subpaths);
		free(bitmap);
		return 1; 
	}

	directoryname = subpaths[sizeofpaths - 1]; //Directory name to remove

    if(sizeofpaths > 1) {
        par_dir = subpaths[sizeofpaths - 2];
    }else{
        SIFS_errno = SIFS_EINVAL;
        free(subpaths);
        free(directoryname);
        free(bitmap);
        return 1;
    }

    // This for block finds directory to remove in the volume and checks if it has entries
    for(int i=0; i<nblocks; i++){
    	if(bitmap[i] == SIFS_DIR){
	    	offset2 = blocksize*(i);
			fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
			fread(&checkdir, sizeof(checkdir), 1, fp); 
			if(strcmp(checkdir.name,directoryname)==0){		//Checks if directory names are same to get index position
			   	deldirblock = i;
			   	if(checkdir.nentries > 0){
			   		SIFS_errno	= SIFS_ENOTEMPTY;
			   		free(subpaths);
        			free(directoryname);
        			free(bitmap);
			   		return 1;							//Directory has entries!
			   	}
			   	break;
			}
	    }
    }

    memset(&checkdir, 0, sizeof checkdir); 				//Reset all of directory we removed
    fwrite(&checkdir, sizeof(checkdir),1, fp);

    bitmap[deldirblock] = SIFS_UNUSED;					//Update bitmap
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, nblocks* sizeof(SIFS_BIT),1, fp); 


    // This if-block updates the parent directory if path is greater than size 1.
    if(sizeofpaths>1){
	    for(int j=0; j<nblocks;j++){
	    	if(bitmap[j] == SIFS_DIR){
		    	offset2 = blocksize*(j);
				fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
				fread(&parentdir, sizeof(parentdir), 1, fp); 
				if(strcmp(parentdir.name,par_dir)==0){
					break;
				}
			}
	    }
	}
	
	if(sizeofpaths==2){
		offset2 =0;
	}

	// If no parent directory, assume parent is the root directory
	fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
	fread(&parentdir, sizeof(parentdir), 1, fp); 
	for(int k=0; k<SIFS_MAX_ENTRIES; k++){
		if(parentdir.entries[k].blockID == deldirblock){
			parentdir.modtime = time(NULL);
			if(parentdir.nentries>1){	//Parent directory entries are not empty
				for(int p=k;p<((parentdir.nentries)-1);p++){
					parentdir.entries[p].fileindex = parentdir.entries[p+1].fileindex;
					parentdir.entries[p].blockID = parentdir.entries[p+1].blockID;
				}
			}
			parentdir.entries[parentdir.nentries-1].fileindex = 0;
			parentdir.entries[parentdir.nentries-1].blockID = 0;
			parentdir.nentries--;
			fseek(fp, sizeof(SIFS_VOLUME_HEADER) + bitmapsize + offset2, SEEK_SET);
			fwrite(&parentdir, sizeof(parentdir),1, fp);
			fclose(fp);
			return 0;
		}
	}

    SIFS_errno	= SIFS_ENOMEM;
    free(subpaths);
    free(directoryname);
    free(bitmap);
    return 1;
}
