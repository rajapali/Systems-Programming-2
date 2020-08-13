


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sifs.h"

//  REPORT HOW THIS PROGRAM SHOULD BE INVOKED
void usage(char *progname)
{
    fprintf(stderr, "Usage: %s volumename\n", progname);
    exit(EXIT_FAILURE);
}


int main(int argcount, char *argvalue[])
{
    char	*volumename;    // filename storing the SIFS volume


//  ATTEMPT TO OBTAIN THE volumename FROM AN ENVIRONMENT VARIABLE

//  ... OR FROM A COMMAND-LINE PARAMETER
    if(argcount == 2) {
        volumename	= argvalue[1];
    }
    else {
        usage(argvalue[0]);
        exit(EXIT_FAILURE);
    }



//  PASS THE ADDRESS OF OUR VARIABLES SO THAT SIFS_dirinfo() MAY MODIFY THEM
    if(open_validate_vol(volumename, "r") != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }else{
        printf("%s is a valid volume!\n", volumename);
    }



    return EXIT_SUCCESS;
}
