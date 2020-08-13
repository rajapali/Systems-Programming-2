

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sifs.h"


//  Written by Chris.McDonald@uwa.edu.au, September 2019

int obtainData(const char diskpathname[], void **data, size_t *nbytes){
    FILE *file = fopen(diskpathname, "rb");
    if(file == NULL){
       perror(diskpathname);
       return 1;
    }


    fseek(file, 0, SEEK_END);
    *nbytes = ftell(file);

    rewind(file);

    *data = malloc(*nbytes*sizeof(char) + 1);
    fread(*data, *nbytes, 1, file);

    fclose(file);
    return 0;
}

void printDataChar(int pos, void* data)
{
    // Get character
    char curChar = ((char*) data)[pos];

    // Print non-empty chars
    if (curChar != '\0' && curChar != '\n') {
        printf("  %i-%c", pos, curChar);
    }

    // Spaces
    if (pos != 0 && pos % 12 == 0) {
        printf("\n");
    }
}

void printSampleBytes(void* data, size_t nbytes)
{
    printf("\n  Sample bytes:");
    printf("\n  (in the form: <bytepos>-<character>)");
    printf("\n  (skips null bytes and new line characters)");

    printf("\n\n  First 100 bytes: \n");
    for (int i = 0; i < 100; i++) {
        printDataChar(i, data);
    }

    printf("\n\n  Last 100 bytes: \n");
    for (int i = nbytes - 100; i < nbytes; i++) {
        printDataChar(i, data);
    }
}




//int getMD5(const char diskpathname[], char MD5 ){
//;
//}
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
    //GET SIZE OF FILE
    void *contents;
    size_t nbytes;

    if(obtainData(diskpathname, &contents, &nbytes) != 0){
        printf("Obtaining data failed\n");
    }

    printSampleBytes(contents, nbytes);


//  PASS THE ADDRESS OF OUR VARIABLES SO THAT SIFS_dirinfo() MAY MODIFY THEM
    if(SIFS_writefile(volumename, volpathname, contents, nbytes) != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }



    free(contents);

    return EXIT_SUCCESS;
}

