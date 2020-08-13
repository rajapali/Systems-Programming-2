
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sifs.h"


//  REPORT HOW THIS PROGRAM SHOULD BE INVOKED
void usage(char *progname)
{
    fprintf(stderr, "Usage: %s volumename pathname\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argcount, char *argvalue[])
{
    char	*volumename;    // filename storing the SIFS volume
    char	*pathname;  //the required directory name on the volume


    if(argcount == 3) {
        volumename	= argvalue[1];
        pathname	= argvalue[2];

    }
    else {
        usage(argvalue[0]);
        exit(EXIT_FAILURE);
    }


//  PASS THE ADDRESS OF OUR VARIABLES SO THAT SIFS_dirinfo() MAY MODIFY THEM
    if(SIFS_rmfile(volumename, pathname) != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }





    //free(contents);

    return EXIT_SUCCESS;
}
