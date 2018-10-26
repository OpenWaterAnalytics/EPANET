/*
----------------------------------------------------------------
Command line executable for the EPANET water distribution system
analysis program using the EPANET API library.
----------------------------------------------------------------
*/

#include <stdio.h>
#include "epanet2.h"

// Function for writing progress messages to the console
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
 **  f2 = name of report file (optional, stdout if left blank)
 **  f3 = name of binary output file (optional, nullfile if left blank).
 **--------------------------------------------------------------
 */
{
    char *f1,*f2,*f3;
    char blank[] = "";
    char errmsg[EN_MAXMSG+1] = "";
    int  errcode;
    int  version;
    int  major;
    int  minor;
    int  patch;
    EN_ProjectHandle ph;
    
    // Check for proper number of command line arguments
    if (argc < 2)
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
    if (argc > 2) f2 = argv[2];  // set rptfile name
    else          f2 = blank;    // use stdout for rptfile
    if (argc > 3) f3 = argv[3];  // set binary output file name
    else          f3 = blank;    // no binary output file

    // Create a project and run it
    errcode = EN_createproject(&ph);
    errcode = EN_runproject(ph, f1, f2, f3, &writeConsole);

    // Blank out the last progress message
    printf("\r                                                               ");

    // Check for errors/warnings and report accordingly
    if (errcode == 0)
    {
        printf("\n... EPANET ran successfully.\n");
    }
    else if (errcode < 100)
    {
        printf("\n... EPANET ran with warnings - check the Status Report.\n");
    }
    else
    {
        EN_geterror(errcode, errmsg, EN_MAXMSG);
        printf("\n... EPANET failed with ERROR %d: %s.\n", errcode, errmsg);
    }

    // Delete the project
    EN_deleteproject(&ph);
    return errcode;
}
