#include <stdio.h>
#include "epanet2.h"
#include "text.h"

void  writeConsole(char *s);

extern char Warnflag;

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
 **  f1 = name of input file, f2 = name of report file, and
 **  f3 = name of binary output file (optional).
 **--------------------------------------------------------------
 */
{
  char *f1,*f2,*f3;
  char blank[] = "";
  int  errcode;
  
  /* Check for proper number of command line arguments */
  if (argc < 3) {
    writeConsole(FMT03);
  }
  else {
    /* Call the main control function */
    f1 = argv[1];
    f2 = argv[2];
    if (argc > 3) {
      f3 = argv[3];
    }
    else {
      f3 = blank;
    }
    writeConsole(FMT01);
    errcode = ENepanet(f1,f2,f3,NULL);
    if (errcode > 0) {
      writeConsole(FMT11);
    }
    else if (Warnflag > 0) {
      writeConsole(FMT10);
    }
    else {
      writeConsole(FMT09);
    }
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
  fprintf(stdout,"%s",s);
  fflush(stdout);
}
