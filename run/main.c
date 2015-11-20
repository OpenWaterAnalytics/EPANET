#include <stdio.h>
#include <string.h>
#include "epanet2.h"

/* text copied here, no more need of include "text.h" */
#define FMT01  "\n... EPANET Version %d.%d.%d\n"
#define FMT03  "\n Correct syntax is:\n epanet <input file> <output file>\n"
#define FMT09  "\n... EPANET completed.\n"
#define FMT10  "\n... EPANET completed. There are warnings."
#define FMT11  "\n... EPANET completed. There are errors."


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
  char errmsg[40];
  int  errcode;
  int  version;
  char s[25];

  
  /* Check for proper number of command line arguments */
  if (argc < 2) {
    writeConsole(FMT03);
  }
  else {
    /* Call the main control function */
    f1 = argv[1];
    if (argc > 2) {
      f2 = argv[2];
    }
    else {
      f2 = blank;
    }
    if (argc > 3) {
      f3 = argv[3];
    }
    else {
      f3 = blank;
    }
  }

  /* get version from DLL and trasform in Major.Minor.Patch format
  instead of hardcoded version */
  ENgetversion(&version);
  int major=  version/10000;
  int minor=  (version%10000)/100;
  int patch=  version%100;
  sprintf(s,FMT01, major , minor, patch);
  writeConsole(s);

  if (strlen(f2)> 0) {
     /* use stdout for progress messages */
     errcode = ENepanet(f1,f2,f3,writeConsole);
  }
  else {
     /* use stdout for reporting, no progress messages */
     errcode = ENepanet(f1,f2,f3,NULL);
  }

  /* Error/Warning check   */
  if (errcode == 0)   writeConsole(FMT09);
  else {
     if (errcode > 100) {
        writeConsole(FMT11);
     }
     else {
        writeConsole(FMT10);
     }
    ENgeterror(errcode, errmsg, 40);
    writeConsole(errmsg);
  }
  return(0);
}                                       /* End of main */


void  writeConsole(char *s)
/*----------------------------------------------------------------
 **  Input:   text string
 **  Output:  none
 **  Purpose: writes string of characters to console
 **----------------------------------------------------------------
 */
{
  fprintf(stdout,"%s\n",s);
  fflush(stdout);
}


