/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       main.c
 Description:  main stub for a command line executable version of EPANET
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/07/2018
 ******************************************************************************
*/

#include <stdio.h>
#include "epanet2.h"

void  writeConsole(char *s)
{
    fprintf(stdout, "\r%s", s);
    fflush(stdout);
}

int  main(int argc, char *argv[])
/*--------------------------------------------------------------
 **  Input:   argc    = number of command line arguments
 **           *argv[] = array of command line arguments
 **  Output:  none
 **  Purpose: main program stub for command line EPANET
 **
 **  Command line for stand-alone operation is:
 **    progname f1  f2  f3
 **  where progname = name of executable this code was compiled to,
 **  f1 = name of input file,
 **  f2 = name of report file
 **  f3 = name of binary output file (optional).
 **--------------------------------------------------------------
 */
{
    char *f1,*f2,*f3;
    char blank[] = "";
    char errmsg[256] = "";
    int  errcode;
    int  version;
    int  major;
    int  minor;
    int  patch;
    
    // Check for proper number of command line arguments
    if (argc < 3)
    {
        printf(
    "\nUsage:\n %s <input_filename> <report_filename> [<binary_filename>]\n",
        argv[0]);
        return 0;
    }

    // Get version number and display in Major.Minor.Patch format
    ENgetversion(&version);
    major = version/10000;
    minor = (version%10000)/100;
    patch = version%100;
    printf("\n... Running EPANET Version %d.%d.%d\n", major, minor, patch);
  
    // Assign pointers to file names
    f1 = argv[1];
    f2 = argv[2];
    if (argc > 3) f3 = argv[3];
    else          f3 = blank;

    // Run EPANET
    errcode = ENepanet(f1, f2, f3, &writeConsole);

    // Blank out the last progress message
    printf("\r                                                               ");

    // Check for errors/warnings and report accordingly
    if (errcode == 0)
    {
        printf("\n... EPANET ran successfully.\n");
        return 0;
    }
    else if (errcode < 100)
    {
        printf("\n... EPANET ran with warnings - check the Status Report.\n");
        return 0;
    }
    else
    {
        ENgeterror(errcode, errmsg, 255);
        printf("\n... EPANET failed with %s.\n", errmsg);
        return 100;
    }
}
