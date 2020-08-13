
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sifs.h"

int writeData(const char diskpathname[], void **data, size_t *nbytes){

    FILE *file = fopen(diskpathname, "w");
    if(file == NULL){
        perror(diskpathname);
        return 1;
    }

    fwrite(*data, *nbytes, 1, file);
    fclose(file);
    return 0;
}

//  REPORT HOW THIS PROGRAM SHOULD BE INVOKED
void usage(char *progname)
{
    fprintf(stderr, "Usage: %s volumename pathname-in-vol pathname-on-disk\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argcount, char *argvalue[])
{
    char	*volumename;    // filename storing the SIFS volume
    char	*volpathname;  //the required directory name on the volume
    char    *diskpathname;

    if(argcount == 4) {
        volumename	= argvalue[1];
        volpathname	= argvalue[2];
        diskpathname = argvalue[3];
    }
    else {
        usage(argvalue[0]);
        exit(EXIT_FAILURE);
    }

    void *contents;
    size_t nbytes;

//  PASS THE ADDRESS OF OUR VARIABLES SO THAT SIFS_dirinfo() MAY MODIFY THEM
    if(SIFS_readfile(volumename, volpathname, &contents, &nbytes) != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }

    writeData(diskpathname, &contents, &nbytes);

   // printf("nbytes = %li\n", nbytes);
    //printf("%s\n", (char *)contents);
   // FILE *file = fopen(diskpathname, "w");
   // fwrite(*contents, nbytes, 1, file);
  //  printf("THIS IS THE DATA BEING WRITTEN\n");


    //free(contents);

    return EXIT_SUCCESS;
}
