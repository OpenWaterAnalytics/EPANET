#include <stdio.h>
#include <string.h>
#include "epanet2_dep.h"

#define   MAXMSG         255       /* Max. # characters in message text      */
#define   MAXWARNCODE    99
/* text copied here, no more need of include "text.h" */
#define FMT01  "\nEPANET Version %d.%d.%d\n"
#define FMT03 "\nUsage:\n %s <input_filename> <report_filename> [<binary_filename>]\n"
#define FMT09  "\n\nEPANET completed.\n"
#define FMT10  "\nEPANET completed. There are warnings.\n"
#define FMT11  "\nEPANET completed. There are errors.\n"


void  writeConsole(char *s);


/*
----------------------------------------------------------------
   Entry point used to compile a stand-alone executable.
----------------------------------------------------------------
*/


int   main(int argc, char *argv[])
/*--------------------------------------------------------------
 **  Input:   argc    = number of command line arguments
 **           *argv[] = array of command line arguments
 **  Output:  none
 **  Purpose: main program segment
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
  char errmsg[MAXMSG+1]="";
  int  errcode;
  int  version;
  int major;
  int minor;
  int patch;

  /* get version from DLL and trasform in Major.Minor.Patch format
  instead of hardcoded version */
  ENgetversion(&version);
  major=  version/10000;
  minor=  (version%10000)/100;
  patch=  version%100;
  printf(FMT01, major, minor, patch);

  /* Check for proper number of command line arguments */
  if (argc < 2) {
    printf(FMT03, argv[0]);
    return(1);
  }

  /* set inputfile name */
  f1 = argv[1];
  if (argc > 2) {
    /* set rptfile name */
    f2 = argv[2];
  }
  else {
    /* use stdout for rptfile */
    f2 = blank;
  }
  if (argc > 3) {
    /* set binary output file name */
    f3 = argv[3];
  }
  else {
    /* NO binary output*/
    f3 = blank;
  }

  /* Call the main control function */
  errcode = ENepanet(f1,f2,f3,NULL);

  /* Error/Warning check   */
  if (errcode == 0) {
     /* no errors */
     printf(FMT09);
     return(0);
  }
  else {
     if (errcode > MAXWARNCODE) printf("\n    Fatal Error: ");
     ENgeterror(errcode, errmsg, MAXMSG);
     printf("%s\n", errmsg);
     if (errcode > MAXWARNCODE) {
        // error //
        printf(FMT11);
        return(errcode);
     }
     else {
        // warning //
        printf(FMT10);
        return(0);
     }
  }
}                                       /* End of main */
