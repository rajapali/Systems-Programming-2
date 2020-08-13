
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sifs.h"

//  REPORT HOW THIS PROGRAM SHOULD BE INVOKED
void usage(char *progname)
{
    fprintf(stderr, "Usage: %s volumename pathname mode\n", progname);
    fprintf(stderr, "or     %s pathname\n", progname);
    exit(EXIT_FAILURE);
}


int main(int argcount, char *argvalue[])
{
    char	*volumename;    // filename storing the SIFS volume
    char	*pathname; // the required directory name on the volume
    char *mode;

//  ATTEMPT TO OBTAIN THE volumename FROM AN ENVIRONMENT VARIABLE

//  ... OR FROM A COMMAND-LINE PARAMETER
     if(argcount == 4) {
        volumename	= argvalue[1];
        pathname	= argvalue[2];
        mode        = argvalue[3];
    }
    else {
        usage(argvalue[0]);
        exit(EXIT_FAILURE);
    }

    char **subpaths;
    uint32_t size;
    if(open_validate_vol(volumename, mode) != 0){
        exit(EXIT_FAILURE);
    }

//  PASS THE ADDRESS OF OUR VARIABLES SO THAT SIFS_dirinfo() MAY MODIFY THEM
    if(checkPathname(volumename, pathname, &subpaths, &size, 0) != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }else{
        printf("%s is a valid pathname!\n", pathname);
    }



    return EXIT_SUCCESS;
}
