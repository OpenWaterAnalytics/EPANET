/*
*******************************************************************************

EPANET.C -- Hydraulic & Water Quality Simulator for Water Distribution Networks

VERSION:    2.2
AUTHORS:    L. Rossman - US EPA - NRMRL
            OpenWaterAnalytics members: see git stats for contributors

EPANET performs extended period hydraulic and water quality analysis of
looped, pressurized piping networks. The program consists of the
following code modules:

    EPANET.C  -- main module providing supervisory control
    INPUT1.C  -- controls processing of input data
    INPUT2.C  -- reads data from input file
    INPUT3.C  -- parses individual lines of input data
    INPFILE.C -- saves modified input data to a text file
    RULES.C   -- implements rule-based control of piping system
    HYDRAUL.C -- computes extended period hydraulic behavior
    QUALITY.C -- tracks transport & fate of water quality
    OUTPUT.C  -- handles transfer of data to and from binary files
    REPORT.C  -- handles reporting of results to text file
    SMATRIX.C -- sparse matrix linear equation solver routines
    MEMPOOL.C -- memory allocation routines
    HASH.C    -- hash table routines

The program can be compiled as either a stand-alone console application
or as a dynamic link library (DLL) of function calls depending on whether
the macro identifier 'DLL' is defined or not.

See EPANET2.H for function prototypes of exported DLL functions
See FUNCS.H for prototypes of all other functions
See TYPES.H for declaration of global constants and data structures
See VARS.H for declaration of global variables
See TEXT.H for declaration of all string constants
See ENUMSTXT.H for assignment of string constants to enumerated types

The following naming conventions are used in all modules of this program:
1. Names of exportable functions in the DLL begin with the "EN" prefix.
2. All other function names are lowercase.
3. Global variable names begin with an uppercase letter.
4. Local variable names are all lowercase.
5. Declared constants and enumerated values defined in TYPES.H are
   all uppercase.
6. String constants defined in TEXT.H begin with a lower case character
   followed by an underscore and then all uppercase characters (e.g.
   t_HEADLOSS)

--------------------------------------------------------------------------

This is the main module of the EPANET program. It uses a series of
functions, all beginning with the letters EN, to control program behavior.
See the main() and ENepanet() functions below for the simplest example of
these.

This module calls the following functions that reside in other modules:
   RULES.C
     initrules()
     allocrules()
     closerules()
   INPUT1.C
     getdata()
     initreport()
   INPUT2.C
     netsize()
     setreport()
   HYDRAUL.C
     openhyd()
     inithyd()
     runhyd()
     nexthyd()
     closehyd()
     tankvolume()
     getenergy()
     setlinkstatus()
     setlinksetting()
   HYDCOEFFS
     resistcoeff()
   QUALITY.C
     openqual()
     initqual()
     runqual()
     nextqual()
     stepqual()
     closequal()
   REPORT.C
     writeline(pr, )
     writelogo()
     writereport()
   HASH.C
     hashtable_create()
     hashtable_find()
     hashtable_free()

The macro ERRCODE(x) is defined in TYPES.H. It says if the current
value of the error code variable (errcode) is not fatal (< 100) then
execute function x and set the error code equal to its return value.

*******************************************************************************
*/

/*** Need to define WINDOWS to use the getTmpName function ***/
// --- define WINDOWS
#undef WINDOWS
#ifdef _WIN32
#define WINDOWS
#endif
#ifdef __WIN32__
#define WINDOWS
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <float.h>
#include <math.h>

#include "enumstxt.h"
#include "epanet_2_2.h"
#include "funcs.h"
#include "text.h"
#include "types.h"


// Local functions
void errorLookup(int errcode, char *errmsg, int len);

/*
----------------------------------------------------------------
   Functions for opening & closing the EPANET system
----------------------------------------------------------------
*/

/// allocate a project pointer
int DLLEXPORT EN_createproject(EN_Project *pr)
// Note: No error handling available until project allocation
{
  int errorcode = 0;
  Project *project = calloc(1, sizeof(Project));

  if (project != NULL){
      project->error_handle = new_errormanager(&errorLookup);
      *pr = project;
  }
  else
      errorcode = -1;

  return errorcode;
}

int DLLEXPORT EN_deleteproject(EN_Project *pr)
// Note: No error handling available after project deallocation
{
    int errorcode = 0;

    if (*pr == NULL)
        errorcode = -1;
    else
    {
        dst_errormanager((*pr)->error_handle);
        free(*pr);

        *pr = NULL;
    }

    return 0;
}

int DLLEXPORT EN_runproject(EN_Project pr, const char *f1, const char *f2,
  const char *f3, void (*pviewprog)(char *))
{
    int errcode = 0;

    ERRCODE(EN_open(pr, f1, f2, f3));
    pr->viewprog = pviewprog;

    if (pr->out_files.Hydflag != USE) {
      ERRCODE(EN_solveH(pr));
    }

    ERRCODE(EN_solveQ(pr));
    ERRCODE(EN_report(pr));

    EN_close(pr);

    if (pr->Warnflag) errcode = MAX(errcode, pr->Warnflag);
    return errcode;
}

int DLLEXPORT EN_init(EN_Project pr, const char *f2, const char *f3,
                      EN_FlowUnits UnitsType, EN_FormType HeadlossFormula)
/*----------------------------------------------------------------
 **  Input:
 **           f2               = pointer to name of report file
 **           f3               = pointer to name of binary output file
 **           UnitsType        = flow units flag
 **           HeadlossFormula  = headloss formula flag
 **  Output:  none
 **  Returns: error code
 **  Purpose: opens EPANET
 **----------------------------------------------------------------
 */
{
  int errcode = 0;
/*** Updated 9/7/00 ***/
/* Reset math coprocessor */
#ifdef DLL
  _fpreset();
#endif

  /* Set system flags */
  pr->Openflag = TRUE;
  pr->hydraulics.OpenHflag = FALSE;
  pr->quality.OpenQflag = FALSE;
  pr->save_options.SaveHflag = FALSE;
  pr->save_options.SaveQflag = FALSE;
  pr->Warnflag = FALSE;
  pr->parser.Coordflag = TRUE;

  /*** Updated 9/7/00 ***/
  pr->report.Messageflag = TRUE;
  pr->report.Rptflag = 1;

  /* Initialize global pointers to NULL. */
  initpointers(pr);

  ERRCODE(netsize(pr));
  ERRCODE(allocdata(pr));

  setdefaults(pr);

  pr->parser.Flowflag = UnitsType;
  pr->hydraulics.Formflag = HeadlossFormula;

  adjustdata(pr);
  initreport(&pr->report);
  initunits(pr);
  inittanks(pr);
  convertunits(pr);

  pr->parser.MaxPats = 0;
  // initialize default pattern
  getpatterns(pr);

  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_open(EN_Project pr, const char *f1, const char *f2, const char *f3)
/*----------------------------------------------------------------
 **  Input:   f1 = pointer to name of input file
 **           f2 = pointer to name of report file
 **           f3 = pointer to name of binary output file
 **  Output:  none
 **  Returns: error code
 **  Purpose: opens EPANET input file & reads in network data
 **----------------------------------------------------------------
 */
{
  int errcode = 0;

/*** Updated 9/7/00 ***/
/* Reset math coprocessor */
#ifdef DLL
  _fpreset();
#endif

  /* Set system flags */
  pr->Openflag = FALSE;
  pr->hydraulics.OpenHflag = FALSE;
  pr->quality.OpenQflag = FALSE;
  pr->save_options.SaveHflag = FALSE;
  pr->save_options.SaveQflag = FALSE;
  pr->Warnflag = FALSE;
  pr->parser.Coordflag = TRUE;

  /*** Updated 9/7/00 ***/
  pr->report.Messageflag = TRUE;
  pr->report.Rptflag = 1;

  /* Initialize global pointers to NULL. */
  initpointers(pr);

  /* Open input & report files */
  ERRCODE(openfiles(pr, f1, f2, f3));
  if (errcode > 0) {
    errmsg(pr, errcode);
    return set_error(pr->error_handle, errcode);
  }
  writelogo(pr);

  /* Find network size & allocate memory for data */
  writewin(pr->viewprog, FMT100);
  ERRCODE(netsize(pr));
  ERRCODE(allocdata(pr));

  /* Retrieve input data */
  ERRCODE(getdata(pr));

  /* Free temporary linked lists used for Patterns & Curves */
  freeTmplist(pr->parser.Patlist);
  freeTmplist(pr->parser.Curvelist);

  /* If using previously saved hydraulics then open its file */
  if (pr->out_files.Hydflag == USE) {
    ERRCODE(openhydfile(pr));
  }

  /* Write input summary to report file */
  if (!errcode) {
    if (pr->report.Summaryflag) {
      writesummary(pr);
    }
    writetime(pr, FMT104);
    pr->Openflag = TRUE;
  } else
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_saveinpfile(EN_Project pr, const char *filename)
/*----------------------------------------------------------------
 **  Input:   filename = name of INP file
 **  Output:  none
 **  Returns: error code
 **  Purpose: saves current data base to file
 **----------------------------------------------------------------
 */
{
  if (!pr->Openflag)
	  return set_error(pr->error_handle, 102);

  return set_error(pr->error_handle, saveinpfile(pr, filename));
}

int DLLEXPORT EN_close(EN_Project pr)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: frees all memory & files used by EPANET
 **----------------------------------------------------------------
 */
{
  out_file_t *out;

  if (pr->Openflag) {
    writetime(pr, FMT105);
  }
  freedata(pr);

  out = &pr->out_files;
  if (out->TmpOutFile != out->OutFile) {
    if (out->TmpOutFile != NULL) {
      fclose(out->TmpOutFile);
    }
    remove(out->TmpFname);
  }
  out->TmpOutFile = NULL;

  if (pr->parser.InFile != NULL) {
    fclose(pr->parser.InFile);
    pr->parser.InFile = NULL;
  }
  if (pr->report.RptFile != NULL && pr->report.RptFile != stdout) {
    fclose(pr->report.RptFile);
    pr->report.RptFile = NULL;
  }
  if (out->HydFile != NULL) {
    fclose(out->HydFile);
    out->HydFile = NULL;
  }
  if (out->OutFile != NULL) {
    fclose(out->OutFile);
    out->OutFile = NULL;
  }

  if (out->Hydflag == SCRATCH)
    remove(out->HydFname);
  if (out->Outflag == SCRATCH)
    remove(out->OutFname);

  pr->Openflag = FALSE;
  pr->hydraulics.OpenHflag = FALSE;
  pr->save_options.SaveHflag = FALSE;
  pr->quality.OpenQflag = FALSE;
  pr->save_options.SaveQflag = FALSE;

  return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for running a hydraulic analysis
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_solveH(EN_Project pr)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: solves for network hydraulics in all time periods
 **----------------------------------------------------------------
 */
{
  int errcode;
  long t, tstep;

  /* Open hydraulics solver */
  errcode = EN_openH(pr);
  if (!errcode) {
    /* Initialize hydraulics */
    errcode = EN_initH(pr, EN_SAVE);

    /* Analyze each hydraulic period */
    if (!errcode)
      do {

        /* Display progress message */
        sprintf(pr->Msg, "%-10s",
                clocktime(pr->report.Atime, pr->time_options.Htime));
        sprintf(pr->Msg, FMT101, pr->report.Atime);
        writewin(pr->viewprog, pr->Msg);

        /* Solve for hydraulics & advance to next time period */
        tstep = 0;
        ERRCODE(EN_runH(pr, &t));
        ERRCODE(EN_nextH(pr, &tstep));
      } while (tstep > 0);
  }

  /* Close hydraulics solver */
  EN_closeH(pr);
  errcode = MAX(errcode, pr->Warnflag);

  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_saveH(EN_Project pr)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: saves hydraulic results to binary file.
 **
 **  Must be called before ENreport() if no WQ simulation made.
 **  Should not be called if ENsolveQ() will be used.
 **----------------------------------------------------------------
 */
{
  char tmpflag;
  int errcode;

  /* Check if hydraulic results exist */
  if (!pr->save_options.SaveHflag)
    return set_error(pr->error_handle, 104);

  /* Temporarily turn off WQ analysis */
  tmpflag = pr->quality.Qualflag;
  pr->quality.Qualflag = NONE;

  /* Call WQ solver to simply transfer results */
  /* from Hydraulics file to Output file at    */
  /* fixed length reporting time intervals.    */
  errcode = EN_solveQ(pr);

  /* Restore WQ analysis option */
  pr->quality.Qualflag = tmpflag;
  if (errcode) {
    errmsg(pr, errcode);
  }
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_openH(EN_Project pr)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: sets up data structures for hydraulic analysis
 **----------------------------------------------------------------
 */
{
  int errcode = 0;

  /* Check that input data exists */
  pr->hydraulics.OpenHflag = FALSE;
  pr->save_options.SaveHflag = FALSE;
  if (!pr->Openflag) {
    return set_error(pr->error_handle, 102);
  }

  /* Check that previously saved hydraulics file not in use */
  if (pr->out_files.Hydflag == USE) {
    return set_error(pr->error_handle, 107);
  }

  /* Open hydraulics solver */
  ERRCODE(openhyd(pr));
  if (!errcode)
    pr->hydraulics.OpenHflag = TRUE;
  else
    errmsg(pr, errcode);

  return set_error(pr->error_handle, errcode);
}

/*** Updated 3/1/01 ***/
int DLLEXPORT EN_initH(EN_Project pr, int flag)
/*----------------------------------------------------------------
 **  Input:   flag = 2-digit flag where 1st (left) digit indicates
 **                  if link flows should be re-initialized (1) or
 **                  not (0) and 2nd digit indicates if hydraulic
 **                  results should be saved to file (1) or not (0)
 **  Output:  none
 **  Returns: error code
 **  Purpose: initializes hydraulic analysis
 **----------------------------------------------------------------
 */
{
  int errcode = 0;
  int sflag, fflag;

  /* Reset status flags */
  pr->save_options.SaveHflag = FALSE;
  pr->Warnflag = FALSE;

  /* Get values of save-to-file flag and reinitialize-flows flag */
  fflag = flag / EN_INITFLOW;
  sflag = flag - fflag * EN_INITFLOW;

  /* Check that hydraulics solver was opened */
  if (!pr->hydraulics.OpenHflag)
    return set_error(pr->error_handle, 103);

  /* Open hydraulics file */
  pr->save_options.Saveflag = FALSE;
  if (sflag > 0) {
    errcode = openhydfile(pr);
    if (!errcode)
      pr->save_options.Saveflag = TRUE;
    else {
      errmsg(pr, errcode);
      return set_error(pr->error_handle, errcode);
      }
  }

  /* Initialize hydraulics */
  inithyd(pr, fflag);
  if (pr->report.Statflag > 0)
    writeheader(pr, STATHDR, 0);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_runH(EN_Project pr, long *t) {
  int errcode;

  *t = 0;
  if (!pr->hydraulics.OpenHflag)
    return set_error(pr->error_handle, 103);
  errcode = runhyd(pr, t);
  if (errcode)
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_nextH(EN_Project pr, long *tstep) {
  int errcode;

  *tstep = 0;
  if (!pr->hydraulics.OpenHflag)
    return set_error(pr->error_handle, 103);
  errcode = nexthyd(pr, tstep);
  if (errcode)
    errmsg(pr, errcode);
  else if (pr->save_options.Saveflag && *tstep == 0)
    pr->save_options.SaveHflag = TRUE;
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_closeH(EN_Project pr) {

  if (!pr->Openflag) {
    return set_error(pr->error_handle, 102);
  }
  if (pr->hydraulics.OpenHflag) {
    closehyd(pr);
  }
  pr->hydraulics.OpenHflag = FALSE;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_savehydfile(EN_Project pr, char *filename) {
  FILE *f;
  int c;
  FILE *HydFile;

  /* Check that hydraulics results exist */
  if (pr->out_files.HydFile == NULL || !pr->save_options.SaveHflag)
    return set_error(pr->error_handle, 104);

  /* Open file */
  if ((f = fopen(filename, "w+b")) == NULL)
    return set_error(pr->error_handle, 305);

  /* Copy from HydFile to f */
  HydFile = pr->out_files.HydFile;
  fseek(HydFile, 0, SEEK_SET);
  while ((c = fgetc(HydFile)) != EOF) {
    fputc(c, f);
  }
  fclose(f);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_usehydfile(EN_Project pr, char *filename) {
  int errcode;

  /* Check that input data exists & hydraulics system closed */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (pr->hydraulics.OpenHflag)
    return set_error(pr->error_handle, 108);

  /* Try to open hydraulics file */
  strncpy(pr->out_files.HydFname, filename, MAXFNAME);
  pr->out_files.Hydflag = USE;
  pr->save_options.SaveHflag = TRUE;
  errcode = openhydfile(pr);

  /* If error, then reset flags */
  if (errcode) {
    strcpy(pr->out_files.HydFname, "");
    pr->out_files.Hydflag = SCRATCH;
    pr->save_options.SaveHflag = FALSE;
  }
  return set_error(pr->error_handle, errcode);
}

/*
 ----------------------------------------------------------------
 Functions for running a WQ analysis
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_solveQ(EN_Project pr) {
  int errcode;
  long t, tstep;

  /* Open WQ solver */
  errcode = EN_openQ(pr);
  if (!errcode) {
    /* Initialize WQ */
    errcode = EN_initQ(pr, EN_SAVE);
    if (!pr->quality.Qualflag) writewin(pr->viewprog, FMT106);

    /* Analyze each hydraulic period */
    if (!errcode)
      do {

        /* Display progress message */
        sprintf(pr->Msg, "%-10s",
                clocktime(pr->report.Atime, pr->time_options.Htime));
        if (pr->quality.Qualflag) {
          sprintf(pr->Msg, FMT102, pr->report.Atime);
          writewin(pr->viewprog, pr->Msg);
        }

        /* Retrieve current network solution & update WQ to next time period */
        tstep = 0;
        ERRCODE(EN_runQ(pr, &t));
        ERRCODE(EN_nextQ(pr, &tstep));
      } while (tstep > 0);
  }

  /* Close WQ solver */
  EN_closeQ(pr);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_openQ(EN_Project pr) {
  int errcode = 0;

  /* Check that hydraulics results exist */
  pr->quality.OpenQflag = FALSE;
  pr->save_options.SaveQflag = FALSE;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  // !LT! todo - check for pr->save_options.SaveHflag / set sequential/step mode
  // if (!pr->save_options.SaveHflag) return(104);

  /* Open WQ solver */
  ERRCODE(openqual(pr));
  if (!errcode)
    pr->quality.OpenQflag = TRUE;
  else
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_initQ(EN_Project pr, int saveflag) {
  int errcode = 0;

  if (!pr->quality.OpenQflag)
    return set_error(pr->error_handle, 105);
  initqual(pr);
  pr->save_options.SaveQflag = FALSE;
  pr->save_options.Saveflag = FALSE;
  if (saveflag) {
    errcode = openoutfile(pr);
    if (!errcode)
      pr->save_options.Saveflag = TRUE;
  }
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_runQ(EN_Project pr, long *t) {
  int errcode;

  *t = 0;
  if (!pr->quality.OpenQflag)
    return set_error(pr->error_handle, 105);
  errcode = runqual(pr, t);
  if (errcode)
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_nextQ(EN_Project pr, long *tstep) {
  int errcode;

  *tstep = 0;
  if (!pr->quality.OpenQflag)
    return set_error(pr->error_handle, 105);
  errcode = nextqual(pr, tstep);
  if (!errcode && pr->save_options.Saveflag && *tstep == 0) {
    pr->save_options.SaveQflag = TRUE;
  }
  if (errcode)
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_stepQ(EN_Project pr, long *tleft) {
  int errcode;

  *tleft = 0;
  if (!pr->quality.OpenQflag)
    return set_error(pr->error_handle, 105);
  errcode = stepqual(pr, tleft);
  if (!errcode && pr->save_options.Saveflag && *tleft == 0) {
    pr->save_options.SaveQflag = TRUE;
  }
  if (errcode)
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_closeQ(EN_Project pr) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  closequal(pr);
  pr->quality.OpenQflag = FALSE;
  return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for generating an output report
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_writeline(EN_Project pr, char *line) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  writeline(pr, line);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_report(EN_Project pr) {
  int errcode;

  /* Check if results saved to binary output file */
  if (!pr->save_options.SaveQflag)
    return set_error(pr->error_handle, 106);
  writewin(pr->viewprog, FMT103);
  errcode = writereport(pr);
  if (errcode)
    errmsg(pr, errcode);
  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_resetreport(EN_Project pr) {
  int i;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  initreport(&pr->report);
  for (i = 1; i <= pr->network.Nnodes; i++)
    pr->network.Node[i].Rpt = 0;
  for (i = 1; i <= pr->network.Nlinks; i++)
    pr->network.Link[i].Rpt = 0;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setreport(EN_Project pr, char *s) {
  char s1[MAXLINE + 1];

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (strlen(s) >= MAXLINE)
    return set_error(pr->error_handle, 250);
  strcpy(s1, s);
  strcat(s1, "\n");
  if (setreport(pr, s1) > 0)
    return set_error(pr->error_handle, 250);
  else
    return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for retrieving network information
 ----------------------------------------------------------------
 */

/*** Updated 10/25/00 ***/
int DLLEXPORT EN_getversion(int *v)
/*----------------------------------------------------------------
 **  Input:    none
 **  Output:   *v = version number of the source code
 **  Returns:  error code (should always be 0)
 **  Purpose:  retrieves a number assigned to the most recent
 **            update of the source code. This number, set by the
 **            constant CODEVERSION found in TYPES.H, is to be
 **            interpreted with implied decimals, i.e., "20100" == "2(.)01(.)00"
 **----------------------------------------------------------------
 */
{
  *v = CODEVERSION;
  return 0;
}

int DLLEXPORT EN_getcontrol(EN_Project pr, int cindex, int *ctype, int *lindex,
                            EN_API_FLOAT_TYPE *setting, int *nindex,
                            EN_API_FLOAT_TYPE *level) {
  double s, lvl;

  EN_Network *net = &pr->network;

  Scontrol *Control = net->Control;
  Snode *Node = net->Node;
  Slink *Link = net->Link;

  const int Njuncs = net->Njuncs;
  double *Ucf = pr->Ucf;

  s = 0.0;
  lvl = 0.0;
  *ctype = 0;
  *lindex = 0;
  *nindex = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (cindex < 1 || cindex > net->Ncontrols)
    return set_error(pr->error_handle, 241);
  *ctype = Control[cindex].Type;
  *lindex = Control[cindex].Link;
  s = Control[cindex].Setting;
  if (Control[cindex].Setting != MISSING) {
    switch (Link[*lindex].Type) {
      case EN_PRV:
      case EN_PSV:
      case EN_PBV:
        s *= Ucf[PRESSURE];
        break;
      case EN_FCV:
        s *= Ucf[FLOW];
      default:
        break;
    }
  } else if (Control[cindex].Status == OPEN) {
    s = 1.0;
  }

  /*** Updated 3/1/01 ***/
  else
    s = 0.0;

  *nindex = Control[cindex].Node;
  if (*nindex > Njuncs)
    lvl = (Control[cindex].Grade - Node[*nindex].El) * Ucf[ELEV];
  else if (*nindex > 0)
    lvl = (Control[cindex].Grade - Node[*nindex].El) * Ucf[PRESSURE];
  else
    lvl = (EN_API_FLOAT_TYPE)Control[cindex].Time;
  *setting = (EN_API_FLOAT_TYPE)s;
  *level = (EN_API_FLOAT_TYPE)lvl;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcount(EN_Project pr, EN_CountType code, int *count) {

  EN_Network *net = &pr->network;

  *count = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  switch (code) {
  case EN_NODECOUNT:
    *count = net->Nnodes;
    break;
  case EN_TANKCOUNT:
    *count = net->Ntanks;
    break;
  case EN_LINKCOUNT:
    *count = net->Nlinks;
    break;
  case EN_PATCOUNT:
    *count = net->Npats;
    break;
  case EN_CURVECOUNT:
    *count = net->Ncurves;
    break;
  case EN_CONTROLCOUNT:
    *count = net->Ncontrols;
    break;
  case EN_RULECOUNT:
    *count = net->Nrules;
    break;
  default:
    return set_error(pr->error_handle, 251);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getoption(EN_Project pr, EN_Option code,
                           EN_API_FLOAT_TYPE *value) {

  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  double *Ucf = pr->Ucf;

  double v = 0.0;
  *value = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  switch (code) {
  case EN_TRIALS:
    v = (double)hyd->MaxIter;
    break;
  case EN_ACCURACY:
    v = hyd->Hacc;
    break;
  case EN_TOLERANCE:
    v = qu->Ctol * Ucf[QUALITY];
    break;
  case EN_EMITEXPON:
    if (hyd->Qexp > 0.0)
      v = 1.0 / hyd->Qexp;
    break;
  case EN_DEMANDMULT:
    v = hyd->Dmult;
    break;
  case EN_HEADERROR:
    v = hyd->HeadErrorLimit * Ucf[HEAD];
    break;
  case EN_FLOWCHANGE:
    v = hyd->FlowChangeLimit * Ucf[FLOW];
    break;
  case EN_DEMANDDEFPAT:
    v = hyd->DefPat;
    break;
  case EN_HEADLOSSFORM:
    v = hyd->Formflag;
    break;

  default:
    return set_error(pr->error_handle, 251);
  }
  *value = (EN_API_FLOAT_TYPE)v;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_gettimeparam(EN_Project pr, int code, long *value) {
  int i;

  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;


  *value = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (code < EN_DURATION || code > EN_NEXTEVENTIDX)
    return set_error(pr->error_handle, 251);
  switch (code) {
  case EN_DURATION:
    *value = time->Dur;
    break;
  case EN_HYDSTEP:
    *value = time->Hstep;
    break;
  case EN_QUALSTEP:
    *value = qu->Qstep;
    break;
  case EN_PATTERNSTEP:
    *value = time->Pstep;
    break;
  case EN_PATTERNSTART:
    *value = time->Pstart;
    break;
  case EN_REPORTSTEP:
    *value = time->Rstep;
    break;
  case EN_REPORTSTART:
    *value = time->Rstart;
    break;
  case EN_STATISTIC:
    *value = rep->Tstatflag;
    break;
  case EN_RULESTEP:
    *value = time->Rulestep;
    break;
  case EN_PERIODS:
    *value = rep->Nperiods;
    break;
  case EN_STARTTIME:
    *value = time->Tstart;
    break; /* Added TNT 10/2/2009 */
  case EN_HTIME:
    *value = time->Htime;
    break;
  case EN_NEXTEVENT:
    *value = time->Hstep; // find the lesser of the hydraulic time step length, or the
                    // time to next fill/empty
    tanktimestep(pr,value);
    break;
  case EN_NEXTEVENTIDX:
      *value = time->Hstep;
      i = tanktimestep(pr, value);
      *value = i;
      break;
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getflowunits(EN_Project pr, int *code) {

  *code = -1;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  *code = pr->parser.Flowflag;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setflowunits(EN_Project pr, int code) {
  int i, j;
  double qfactor, vfactor, hfactor, efactor, xfactor, yfactor;

  double *Ucf = pr->Ucf;
  EN_Network *net = &pr->network;

  if (!pr->Openflag) {
    return set_error(pr->error_handle, 102);
  }

  /* Determine unit system based on flow units */
  qfactor = Ucf[FLOW];
  vfactor = Ucf[VOLUME];
  hfactor = Ucf[HEAD];
  efactor = Ucf[ELEV];

  pr->parser.Flowflag = code;
  switch (code)
  {
    case LPS:          /* Liters/sec */
    case LPM:          /* Liters/min */
    case MLD:          /* megaliters/day  */
    case CMH:          /* cubic meters/hr */
    case CMD:          /* cubic meters/day */
      pr->parser.Unitsflag = SI;
      break;
    default:
      pr->parser.Unitsflag = US;
      break;
  }

  /* Revise pressure units depending on flow units */
  if (pr->parser.Unitsflag != SI) {
    pr->parser.Pressflag = PSI;
  }
  else if (pr->parser.Pressflag == PSI) {
    pr->parser.Pressflag = METERS;
  }

  initunits(pr);

  //update curves
  for (i=1; i <= net->Ncurves; i++)
  {
    switch (net->Curve[i].Type)
    {
      case V_CURVE:
        xfactor = efactor/Ucf[ELEV];
        yfactor = vfactor/Ucf[VOLUME];
        break;
      case H_CURVE:
      case P_CURVE:
        xfactor = qfactor/Ucf[FLOW];
        yfactor = hfactor/Ucf[HEAD];
        break;
      case E_CURVE:
        xfactor = qfactor/Ucf[FLOW];
        yfactor = 1;
        break;
      default:
        xfactor = 1;
        yfactor = 1;
    }

    for (j=0; j < net->Curve[i].Npts; j++)
    {
      net->Curve[i].X[j] = net->Curve[i].X[j]/xfactor;
      net->Curve[i].Y[j] = net->Curve[i].Y[j]/yfactor;
    }
  }

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getdemandmodel(EN_Project pr, int *type, EN_API_FLOAT_TYPE *pmin,
              EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp) {

    *type = pr->hydraulics.DemandModel;
    *pmin = (EN_API_FLOAT_TYPE)(pr->hydraulics.Pmin * pr->Ucf[PRESSURE]);
    *preq = (EN_API_FLOAT_TYPE)(pr->hydraulics.Preq * pr->Ucf[PRESSURE]);
    *pexp = (EN_API_FLOAT_TYPE)(pr->hydraulics.Pexp);

	return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setdemandmodel(EN_Project pr, int type, EN_API_FLOAT_TYPE pmin,
              EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp)
{
    if (type < 0 || type > EN_PDA) return set_error(pr->error_handle, 251);
    if (pmin > preq || pexp <= 0.0) return set_error(pr->error_handle, 202);
    pr->hydraulics.DemandModel = type;
    pr->hydraulics.Pmin = pmin / pr->Ucf[PRESSURE];
    pr->hydraulics.Preq = preq / pr->Ucf[PRESSURE];
    pr->hydraulics.Pexp = pexp;

    return set_error(pr->error_handle, 0);
}


int DLLEXPORT EN_getpatternindex(EN_Project pr, char *id, int *index) {
  int i;

  *index = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  for (i = 1; i <= pr->network.Npats; i++) {
    if (strcmp(id, pr->network.Pattern[i].ID) == 0) {
      *index = i;
      return set_error(pr->error_handle, 0);
    }
  }
  *index = 0;
  return set_error(pr->error_handle, 205);
}

int DLLEXPORT EN_getpatternid(EN_Project pr, int index, char *id) {

  strcpy(id, "");
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Npats)
    return set_error(pr->error_handle, 205);
  strcpy(id, pr->network.Pattern[index].ID);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getpatternlen(EN_Project pr, int index, int *len) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Npats)
    return set_error(pr->error_handle, 205);
  *len = pr->network.Pattern[index].Length;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getpatternvalue(EN_Project pr, int index, int period,
                                 EN_API_FLOAT_TYPE *value) {

  *value = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Npats)
    return set_error(pr->error_handle, 205);
  if (period < 1 || period > pr->network.Pattern[index].Length)
    return set_error(pr->error_handle, 251);
  *value = (EN_API_FLOAT_TYPE)pr->network.Pattern[index].F[period - 1];
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcurveindex(EN_Project pr, char *id, int *index) {
  int i;

  *index = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  for (i = 1; i <= pr->network.Ncurves; i++) {
    if (strcmp(id, pr->network.Curve[i].ID) == 0) {
      *index = i;
      return set_error(pr->error_handle, 0);
    }
  }
  *index = 0;
  return set_error(pr->error_handle, 206);
}

int DLLEXPORT EN_getcurveid(EN_Project pr, int index, char *id) {

  strcpy(id, "");
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Ncurves)
    return set_error(pr->error_handle, 206);
  strcpy(id, pr->network.Curve[index].ID);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcurvelen(EN_Project pr, int index, int *len) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Ncurves)
    return set_error(pr->error_handle, 206);
  *len = pr->network.Curve[index].Npts;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcurvevalue(EN_Project pr, int index, int pnt,
                               EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y) {

  *x = 0.0;
  *y = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Ncurves)
    return set_error(pr->error_handle, 206);
  if (pnt < 1 || pnt > pr->network.Curve[index].Npts)
    return set_error(pr->error_handle, 251);
  *x = (EN_API_FLOAT_TYPE)pr->network.Curve[index].X[pnt - 1];
  *y = (EN_API_FLOAT_TYPE)pr->network.Curve[index].Y[pnt - 1];
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getqualtype(EN_Project pr, int *qualcode, int *tracenode) {

  *tracenode = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  *qualcode = pr->quality.Qualflag;
  if (pr->quality.Qualflag == TRACE)
    *tracenode = pr->quality.TraceNode;
  return set_error(pr->error_handle, 0);
}


int DLLEXPORT EN_getqualinfo(EN_Project pr, int *qualcode, char *chemname,
                             char *chemunits, int *tracenode) {

  EN_getqualtype(pr, qualcode, tracenode);

  if (pr->quality.Qualflag == TRACE) {
    strncpy(chemname, "", MAXID);
    strncpy(chemunits, "dimensionless", MAXID);
  } else {
    strncpy(chemname, pr->quality.ChemName, MAXID);
    strncpy(chemunits, pr->quality.ChemUnits, MAXID);
  }
  return set_error(pr->error_handle, 0);
}

void errorLookup(int errcode, char *dest_msg, int dest_len)
// Purpose: takes error code returns error message
{
    char *msg = NULL;

    switch (errcode)
    {
    case 1: msg = WARN1;
    break;
    case 2: msg = WARN2;
    break;
    case 3: msg = WARN3;
    break;
    case 4: msg = WARN4;
    break;
    case 5: msg = WARN5;
    break;
    case 6: msg = WARN6;
    break;
    default:
        msg = geterrmsg(errcode, msg);
    }
    strncpy(dest_msg, msg, MAXMSG);
}

void DLLEXPORT EN_clearError(EN_Project pr)
{
    clear_error(pr->error_handle);
}

int DLLEXPORT EN_checkError(EN_Project pr, char** msg_buffer)
//
// Purpose: Returns the error message or NULL.
//
// Note: Caller must free memory allocated by EN_check_error
//
{
    int errorcode = 0;
    char *temp = NULL;
    //EN_Project *p = (EN_Project*)ph;


    if (pr == NULL) return -1;
    else
    {
        errorcode = pr->error_handle->error_status;
        if (errorcode)
            temp = check_error(pr->error_handle);

        *msg_buffer = temp;
    }

    return errorcode;
}

int DLLEXPORT EN_geterror(int errcode, char *errmsg, int n) {
	// Deprecate?
  char newMsg[MAXMSG+1];

  switch (errcode) {
  case 1:
    strncpy(errmsg, WARN1, n);
    break;
  case 2:
    strncpy(errmsg, WARN2, n);
    break;
  case 3:
    strncpy(errmsg, WARN3, n);
    break;
  case 4:
    strncpy(errmsg, WARN4, n);
    break;
  case 5:
    strncpy(errmsg, WARN5, n);
    break;
  case 6:
    strncpy(errmsg, WARN6, n);
    break;
  default:
    geterrmsg(errcode, newMsg);
    strncpy(errmsg, newMsg, n);
  }
  if (strlen(errmsg) == 0)
    return 251;
  else
    return 0;
}

int DLLEXPORT EN_getstatistic(EN_Project pr, int code, EN_API_FLOAT_TYPE *value) {

  switch (code) {
  case EN_ITERATIONS:
    *value = (EN_API_FLOAT_TYPE)pr->hydraulics.Iterations;
    break;
  case EN_RELATIVEERROR:
    *value = (EN_API_FLOAT_TYPE)pr->hydraulics.RelativeError;
    break;
  case EN_MAXHEADERROR:
      *value = (EN_API_FLOAT_TYPE)(pr->hydraulics.MaxHeadError * pr->Ucf[HEAD]);
      break;
  case EN_MAXFLOWCHANGE:
      *value = (EN_API_FLOAT_TYPE)(pr->hydraulics.MaxFlowChange * pr->Ucf[FLOW]);
      break;
  case EN_MASSBALANCE:
      *value = (EN_API_FLOAT_TYPE)(pr->quality.massbalance.ratio);
      break;
  default:
    break;
  }
  return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for retrieving node data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_getnodeindex(EN_Project pr, char *id, int *index) {

  *index = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  *index = findnode(&pr->network,id);
  if (*index == 0)
    return set_error(pr->error_handle, 203);
  else
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getnodeid(EN_Project pr, int index, char *id) {

  strcpy(id, "");
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  strcpy(id, pr->network.Node[index].ID);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getnodetype(EN_Project pr, int index, int *code) {

  *code = -1;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  if (index <= pr->network.Njuncs)
    *code = EN_JUNCTION;
  else {
    if (pr->network.Tank[index - pr->network.Njuncs].A == 0.0)
      *code = EN_RESERVOIR;
    else
      *code = EN_TANK;
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcoord(EN_Project pr, int index, EN_API_FLOAT_TYPE *x,
                          EN_API_FLOAT_TYPE *y) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  if (!pr->parser.Coordflag)
    return set_error(pr->error_handle, 255);

  // check if node have coords
  if (pr->network.Coord[index].HaveCoords == FALSE)
    return set_error(pr->error_handle, 254);

  *x = (EN_API_FLOAT_TYPE)pr->network.Coord[index].X;
  *y = (EN_API_FLOAT_TYPE)pr->network.Coord[index].Y;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setcoord(EN_Project pr, int index, EN_API_FLOAT_TYPE x,
                          EN_API_FLOAT_TYPE y) {

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  if (!pr->parser.Coordflag)
    return set_error(pr->error_handle, 255);

  pr->network.Coord[index].X = x;
  pr->network.Coord[index].Y = y;
  pr->network.Coord[index].HaveCoords = TRUE;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getnodevalue(EN_Project pr, int index, int code,
                              EN_API_FLOAT_TYPE *value) {
  double v = 0.0;
  Pdemand demand;
  Psource source;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  Snode *Node = net->Node;
  Stank *Tank = net->Tank;

  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;

  double *Ucf = pr->Ucf;
  double *NodeDemand = hyd->NodeDemand;
  double *NodeQual = qu->NodeQual;


  /* Check for valid arguments */
  *value = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Nnodes)
    return set_error(pr->error_handle, 203);

  /* Retrieve called-for parameter */
  switch (code) {
  case EN_ELEVATION:
    v = Node[index].El * Ucf[ELEV];
    break;

  case EN_BASEDEMAND:
    v = 0.0;
    /* NOTE: primary demand category is last on demand list */
    if (index <= Njuncs)
      for (demand = Node[index].D; demand != NULL; demand = demand->next)
        v = (demand->Base);
    v *= Ucf[FLOW];
    break;

  case EN_PATTERN:
    v = 0.0;
    /* NOTE: primary demand category is last on demand list */
    if (index <= Njuncs) {
      for (demand = Node[index].D; demand != NULL; demand = demand->next)
        v = (double)(demand->Pat);
    } else
      v = (double)(Tank[index - Njuncs].Pat);
    break;

  case EN_EMITTER:
    v = 0.0;
    if (Node[index].Ke > 0.0)
      v = Ucf[FLOW] / pow((Ucf[PRESSURE] * Node[index].Ke), (1.0 / hyd->Qexp));
    break;

  case EN_INITQUAL:
    v = Node[index].C0 * Ucf[QUALITY];
    break;

    /*** Additional parameters added for retrieval ***/
  case EN_SOURCEQUAL:
  case EN_SOURCETYPE:
  case EN_SOURCEMASS:
  case EN_SOURCEPAT:
    source = Node[index].S;
    if (source == NULL)
      return set_error(pr->error_handle, 240);
    if (code == EN_SOURCEQUAL)
      v = source->C0;
    else if (code == EN_SOURCEMASS)
      v = source->Smass * 60.0;
    else if (code == EN_SOURCEPAT)
      v = source->Pat;
    else
      v = source->Type;
    break;

  case EN_TANKLEVEL:
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    v = (Tank[index - Njuncs].H0 - Node[index].El) * Ucf[ELEV];
    break;

    /*** New parameter added for retrieval ***/
  case EN_INITVOLUME:
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].V0 * Ucf[VOLUME];
    break;

    /*** New parameter added for retrieval ***/
  case EN_MIXMODEL:
    v = MIX1;
    if (index > Njuncs)
      v = Tank[index - Njuncs].MixModel;
    break;

    /*** New parameter added for retrieval ***/
  case EN_MIXZONEVOL:
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].V1max * Ucf[VOLUME];
    break;

  case EN_DEMAND:
    v = NodeDemand[index] * Ucf[FLOW];
    break;

  case EN_HEAD:
    v = hyd->NodeHead[index] * Ucf[HEAD];
    break;

  case EN_PRESSURE:
    v = (hyd->NodeHead[index] - Node[index].El) * Ucf[PRESSURE];
    break;

  case EN_QUALITY:
    v = NodeQual[index] * Ucf[QUALITY];
    break;

    /*** New parameters added for retrieval begins here   ***/
  /*** (Thanks to Nicolas Basile of Ecole Polytechnique ***/
  /***  de Montreal for suggesting some of these.)      ***/

  case EN_TANKDIAM:
    v = 0.0;
    if (index > Njuncs) {
      v = sqrt(4.0 / PI * Tank[index - Njuncs].A) * Ucf[ELEV];
    }
    break;

  case EN_MINVOLUME:
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].Vmin * Ucf[VOLUME];
    break;

  case EN_MAXVOLUME: // !sph
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].Vmax * Ucf[VOLUME];
    break;

  case EN_VOLCURVE:
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].Vcurve;
    break;

  case EN_MINLEVEL:
    v = 0.0;
    if (index > Njuncs) {
      v = (Tank[index - Njuncs].Hmin - Node[index].El) * Ucf[ELEV];
    }
    break;

  case EN_MAXLEVEL:
    v = 0.0;
    if (index > Njuncs) {
      v = (Tank[index - Njuncs].Hmax - Node[index].El) * Ucf[ELEV];
    }
    break;

  case EN_MIXFRACTION:
    v = 1.0;
    if (index > Njuncs && Tank[index - Njuncs].Vmax > 0.0) {
      v = Tank[index - Njuncs].V1max / Tank[index - Njuncs].Vmax;
    }
    break;

  case EN_TANK_KBULK:
    v = 0.0;
    if (index > Njuncs)
      v = Tank[index - Njuncs].Kb * SECperDAY;
    break;

    /***  New parameter additions ends here. ***/

  case EN_TANKVOLUME:
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    v = tankvolume(pr, index - Njuncs, hyd->NodeHead[index]) * Ucf[VOLUME];
    break;

  default:
    return set_error(pr->error_handle, 251);
  }
  *value = (EN_API_FLOAT_TYPE)v;
  return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for retrieving link data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_getlinkindex(EN_Project pr, char *id, int *index) {

  *index = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  *index = findlink(&pr->network,id);
  if (*index == 0)
    return set_error(pr->error_handle, 204);
  else
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getlinkid(EN_Project pr, int index, char *id) {

  strcpy(id, "");
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nlinks)
    return set_error(pr->error_handle, 204);
  strcpy(id, pr->network.Link[index].ID);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getlinktype(EN_Project pr, int index, EN_LinkType *code) {

  *code = -1;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nlinks)
    return set_error(pr->error_handle, 204);
  *code = pr->network.Link[index].Type;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getlinknodes(EN_Project pr, int index, int *node1,
                              int *node2) {

  *node1 = 0;
  *node2 = 0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > pr->network.Nlinks)
    return set_error(pr->error_handle, 204);
  *node1 = pr->network.Link[index].N1;
  *node2 = pr->network.Link[index].N2;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getlinkvalue(EN_Project pr, int index, EN_LinkProperty code,
                                                        EN_API_FLOAT_TYPE *value) {
  double a, h, q, v = 0.0;
  int returnValue = 0;
  int pmp;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;

  Slink *Link = net->Link;
  Spump *Pump = net->Pump;

  const int Nlinks = net->Nlinks;

  double *Ucf = pr->Ucf;
  double *LinkFlows = hyd->LinkFlows;
  double *LinkSetting = hyd->LinkSetting;

  /* Check for valid arguments */
  *value = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Nlinks)
    return set_error(pr->error_handle, 204);

  /* Retrieve called-for parameter */
  switch (code) {
    case EN_DIAMETER:
      if (Link[index].Type == EN_PUMP)
        v = 0.0;
      else
        v = Link[index].Diam * Ucf[DIAM];
      break;

    case EN_LENGTH:
      v = Link[index].Len * Ucf[ELEV];
      break;

    case EN_ROUGHNESS:
      if (Link[index].Type <= EN_PIPE) {
        if (hyd->Formflag == DW)
          v = Link[index].Kc * (1000.0 * Ucf[ELEV]);
        else
          v = Link[index].Kc;
      } else
        v = 0.0;
      break;

    case EN_MINORLOSS:
      if (Link[index].Type != EN_PUMP) {
        v = Link[index].Km;
        v *= (SQR(Link[index].Diam) * SQR(Link[index].Diam) / 0.02517);
      } else
        v = 0.0;
      break;

    case EN_INITSTATUS:
      if (Link[index].Stat <= CLOSED)
        v = 0.0;
      else
        v = 1.0;
      break;

    case EN_INITSETTING:
      if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
        return set_error(pr->error_handle, EN_getlinkvalue(pr, index, EN_ROUGHNESS, value));
      v = Link[index].Kc;
      switch (Link[index].Type) {
        case EN_PRV:
        case EN_PSV:
        case EN_PBV:
          v *= Ucf[PRESSURE];
          break;
        case EN_FCV:
          v *= Ucf[FLOW];
        default:
          break;
      }
      break;

    case EN_KBULK:
      v = Link[index].Kb * SECperDAY;
      break;

    case EN_KWALL:
      v = Link[index].Kw * SECperDAY;
      break;

    case EN_FLOW:

      /*** Updated 10/25/00 ***/
      if (hyd->LinkStatus[index] <= CLOSED)
        v = 0.0;
      else
        v = LinkFlows[index] * Ucf[FLOW];
      break;

    case EN_VELOCITY:
      if (Link[index].Type == EN_PUMP) {
        v = 0.0;
      }

      /*** Updated 11/19/01 ***/
      else if (hyd->LinkStatus[index] <= CLOSED)
        v = 0.0;

      else {
        q = ABS(LinkFlows[index]);
        a = PI * SQR(Link[index].Diam) / 4.0;
        v = q / a * Ucf[VELOCITY];
      }
      break;

    case EN_HEADLOSS:

      /*** Updated 11/19/01 ***/
      if (hyd->LinkStatus[index] <= CLOSED)
        v = 0.0;

      else {
        h = hyd->NodeHead[Link[index].N1] - hyd->NodeHead[Link[index].N2];
        if (Link[index].Type != EN_PUMP)
          h = ABS(h);
        v = h * Ucf[HEADLOSS];
      }
      break;

    case EN_STATUS:
      if (hyd->LinkStatus[index] <= CLOSED)
        v = 0.0;
      else
        v = 1.0;
      break;

    case EN_STATE:
      v = hyd->LinkStatus[index];

      if (Link[index].Type == EN_PUMP) {
         pmp = findpump(net, index);
         if (hyd->LinkStatus[index] >= OPEN) {
            if (hyd->LinkFlows[index] > hyd->LinkSetting[index] * Pump[pmp].Qmax)
               v = XFLOW;
            if (hyd->LinkFlows[index] < 0.0)
               v = XHEAD;
         }
      }
      break;

    case EN_CONST_POWER:
      v = 0;
      if (Link[index].Type == EN_PUMP) {
         pmp = findpump(net, index);
         if (Pump[pmp].Ptype == CONST_HP) {
             v = Link[index].Km; // Power in HP
         }
      }
      break;

    case EN_SPEED:
      v = 0;
      if (Link[index].Type == EN_PUMP) {
         pmp = findpump(net, index);
         v = Link[index].Kc;
      }
      break;

    case EN_SETTING:
      if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE) {
        return set_error(pr->error_handle, EN_getlinkvalue(pr, index, EN_ROUGHNESS, value));
      }
      if (LinkSetting[index] == MISSING) {
        v = 0.0;
      } else {
        v = LinkSetting[index];
      }
      switch (Link[index].Type) {
        case EN_PRV:
        case EN_PSV:
        case EN_PBV:
          v *= Ucf[PRESSURE];
          break;
        case EN_FCV:
          v *= Ucf[FLOW];
        default:
          break;
      }
      break;

    case EN_ENERGY:
      getenergy(pr, index, &v, &a);
      break;

    case EN_LINKQUAL:
      v = avgqual(pr, index) * Ucf[LINKQUAL];
      break;

    case EN_LINKPATTERN:
      if (Link[index].Type == EN_PUMP)
        v = (double)Pump[findpump(&pr->network, index)].Upat;
      break;

    case EN_EFFICIENCY:
      getenergy(pr, index, &a, &v);
      break;

    case EN_PRICEPATTERN:
      if (Link[index].Type == EN_PUMP)
        v = (double)Pump[findpump(&pr->network, index)].Epat;
      break;

    case EN_HEADCURVE:
      if (Link[index].Type == EN_PUMP) {
        v = (double)Pump[findpump(&pr->network, index)].Hcurve;
        if (v == 0) {
          returnValue = 226;
        }
      }
      else {
        v = 0;
        returnValue = 211;
      }
      break;

    case EN_EFFICIENCYCURVE:
      if (Link[index].Type == EN_PUMP) {
        v = (double)Pump[findpump(&pr->network, index)].Ecurve;
        if (v == 0) {
          returnValue = 268;
        }
      }
      else {
        v = 0;
        returnValue = 211;
      }

    default:
      v = 0;
      returnValue = 251;
  }
  *value = (EN_API_FLOAT_TYPE)v;
  return set_error(pr->error_handle, returnValue);
}

int DLLEXPORT EN_getcurve(EN_Project pr, int curveIndex, char *id, int *nValues,
                          EN_API_FLOAT_TYPE **xValues,
                          EN_API_FLOAT_TYPE **yValues) {
  int iPoint, nPoints;
  Scurve curve;
  EN_API_FLOAT_TYPE *pointX, *pointY;

  /* Check that input file opened */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);

  curve = pr->network.Curve[curveIndex];
  nPoints = curve.Npts;

  pointX = calloc(nPoints, sizeof(EN_API_FLOAT_TYPE));
  pointY = calloc(nPoints, sizeof(EN_API_FLOAT_TYPE));

  for (iPoint = 0; iPoint < nPoints; iPoint++) {
    double x = curve.X[iPoint];
    double y = curve.Y[iPoint];
    pointX[iPoint] = (EN_API_FLOAT_TYPE)x;
    pointY[iPoint] = (EN_API_FLOAT_TYPE)y;
  }

  strncpy(id, "", MAXID);
  strncpy(id, curve.ID, MAXID);
  *nValues = nPoints;
  *xValues = pointX;
  *yValues = pointY;

  return set_error(pr->error_handle, 0);
}

/*
 ----------------------------------------------------------------
 Functions for changing network data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_addcontrol(EN_Project pr, int *cindex, int ctype, int lindex,
                            EN_API_FLOAT_TYPE setting, int nindex,
                            EN_API_FLOAT_TYPE level) {
  char status = ACTIVE;
  int i, n;
  long t = 0, nControls;
  double s = setting, lvl = level;
  EN_Network *net;
  Snode *Node;
  Slink *Link;
  Scontrol *Control;
  Scontrol *tmpControl;

  int Nnodes;
  int Njuncs;
  int Nlinks;
  double *Ucf;

  parser_data_t *par = &pr->parser;

  /* Check that input file opened */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);

  net = &pr->network;
  Node = net->Node;
  Link = net->Link;
  Control = net->Control;

  Nnodes = net->Nnodes;
  Njuncs = net->Njuncs;
  Nlinks = net->Nlinks;
  nControls = net->Ncontrols;

  Ucf = pr->Ucf;

  /* Check that controlled link exists */
  if (lindex < 0 || lindex > Nlinks)
    return set_error(pr->error_handle, 204);

  /* Cannot control check valve. */
  if (Link[lindex].Type == EN_CVPIPE)
    return set_error(pr->error_handle, 207);

  /* Check for valid parameters */
  if (ctype < 0 || ctype > EN_TIMEOFDAY)
    return set_error(pr->error_handle, 251);
  if (ctype == EN_LOWLEVEL || ctype == EN_HILEVEL) {
    if (nindex < 1 || nindex > Nnodes)
      return set_error(pr->error_handle, 203);
  } else
    nindex = 0;
  if (s < 0.0 || lvl < 0.0)
    return set_error(pr->error_handle, 202);

  /* Adjust units of control parameters */
  switch (Link[lindex].Type) {
    case EN_PRV:
    case EN_PSV:
    case EN_PBV:
      s /= Ucf[PRESSURE];
      break;
    case EN_FCV:
      s /= Ucf[FLOW];
      break;
    case EN_GPV:
      if (s == 0.0)
        status = CLOSED;
      else if (s == 1.0)
        status = OPEN;
      else
        return set_error(pr->error_handle, 202);
      s = Link[lindex].Kc;
      break;
    case EN_PIPE:
    case EN_PUMP:
      status = OPEN;
      if (s == 0.0)
        status = CLOSED;
    default:
      break;
  }

  if (ctype == LOWLEVEL || ctype == HILEVEL) {
    if (nindex > Njuncs)
      lvl = Node[nindex].El + level / Ucf[ELEV];
    else
      lvl = Node[nindex].El + level / Ucf[PRESSURE];
  }
  if (ctype == TIMER)
    t = (long)ROUND(lvl);
  if (ctype == TIMEOFDAY)
    t = (long)ROUND(lvl) % SECperDAY;

  //new control is good
  /* Allocate memory for a new array of controls */
  n = nControls + 1;
  tmpControl = (Scontrol *)calloc(n + 1, sizeof(Scontrol));
  if (tmpControl == NULL)
    return set_error(pr->error_handle, 101);

  /* Copy contents of old controls array to new one */
  for (i = 0; i <= nControls; i++) {
    tmpControl[i].Type = Control[i].Type;
    tmpControl[i].Link = Control[i].Link;
    tmpControl[i].Node = Control[i].Node;
    tmpControl[i].Status = Control[i].Status;
    tmpControl[i].Setting = Control[i].Setting;
    tmpControl[i].Grade = Control[i].Grade;
    tmpControl[i].Time = Control[i].Time;
  }

  /* Add the new control to the new array of controls */
  tmpControl[n].Type = (char)ctype;
  tmpControl[n].Link = lindex;
  tmpControl[n].Node = nindex;
  tmpControl[n].Status = status;
  tmpControl[n].Setting = s;
  tmpControl[n].Grade = lvl;
  tmpControl[n].Time = t;

  // Replace old control array with new one
  free(Control);
  net->Control = tmpControl;
  net->Ncontrols = n;
  par->MaxControls = n;

  // return the new control index
  *cindex = n;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setcontrol(EN_Project pr, int cindex, int ctype, int lindex,
                            EN_API_FLOAT_TYPE setting, int nindex,
                            EN_API_FLOAT_TYPE level) {
  char status = ACTIVE;
  long t = 0;
  double s = setting, lvl = level;
  EN_Network *net;
  Snode *Node;
  Slink *Link;
  Scontrol *Control;

  int Nnodes;
  int Njuncs;
  int Nlinks;
  double *Ucf;

  /* Check that input file opened */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);

  /* Check that control exists */
  if (cindex < 1 || cindex > pr->network.Ncontrols)
    return set_error(pr->error_handle, 241);

  net = &pr->network;

  Node = net->Node;
  Link = net->Link;
  Control = net->Control;

  Nnodes = net->Nnodes;
  Njuncs = net->Njuncs;
  Nlinks = net->Nlinks;

  Ucf = pr->Ucf;

  /* Check that controlled link exists */
  if (lindex == 0) {
    Control[cindex].Link = 0;
    return set_error(pr->error_handle, 0);
  }
  if (lindex < 0 || lindex > Nlinks)
    return set_error(pr->error_handle, 204);

  /* Cannot control check valve. */
  if (Link[lindex].Type == EN_CVPIPE)
    return set_error(pr->error_handle, 207);

  /* Check for valid parameters */
  if (ctype < 0 || ctype > EN_TIMEOFDAY)
    return set_error(pr->error_handle, 251);
  if (ctype == EN_LOWLEVEL || ctype == EN_HILEVEL) {
    if (nindex < 1 || nindex > Nnodes)
      return set_error(pr->error_handle, 203);
  } else
    nindex = 0;
  if (s < 0.0 || lvl < 0.0)
    return set_error(pr->error_handle, 202);

  /* Adjust units of control parameters */
  switch (Link[lindex].Type) {
    case EN_PRV:
    case EN_PSV:
    case EN_PBV:
      s /= Ucf[PRESSURE];
      break;
    case EN_FCV:
      s /= Ucf[FLOW];
      break;

      /*** Updated 9/7/00 ***/
    case EN_GPV:
      if (s == 0.0)
        status = CLOSED;
      else if (s == 1.0)
        status = OPEN;
      else
        return set_error(pr->error_handle, 202);
      s = Link[lindex].Kc;
      break;

    case EN_PIPE:
    case EN_PUMP:
      status = OPEN;
      if (s == 0.0)
        status = CLOSED;
    default:
      break;
  }
  if (ctype == LOWLEVEL || ctype == HILEVEL) {
    if (nindex > Njuncs)
      lvl = Node[nindex].El + level / Ucf[ELEV];
    else
      lvl = Node[nindex].El + level / Ucf[PRESSURE];
  }
  if (ctype == TIMER)
    t = (long)ROUND(lvl);
  if (ctype == TIMEOFDAY)
    t = (long)ROUND(lvl) % SECperDAY;

  /* Reset control's parameters */
  Control[cindex].Type = (char)ctype;
  Control[cindex].Link = lindex;
  Control[cindex].Node = nindex;
  Control[cindex].Status = status;
  Control[cindex].Setting = s;
  Control[cindex].Grade = lvl;
  Control[cindex].Time = t;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setnodeid(EN_Project pr, int index, char *newid)
{
    EN_Network *net = &pr->network;
    size_t n;

    // Check for valid arguments
    if (index <= 0 || index > net->Nnodes)
    {
        return set_error(pr->error_handle, 203);
    }
    n = strlen(newid);
    if (n < 1 || n > MAXID) return set_error(pr->error_handle, 209);
    if (strcspn(newid, " ;") < n) return set_error(pr->error_handle, 209);

    // Check if another node with same name exists
    if (hashtable_find(net->NodeHashTable, newid) > 0)
    {
        return set_error(pr->error_handle, 215);
    }

    // Replace the existing node ID with the new value
    hashtable_delete(net->NodeHashTable, net->Node[index].ID);
    strncpy(net->Node[index].ID, newid, MAXID);
    hashtable_insert(net->NodeHashTable, net->Node[index].ID, index);
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setnodevalue(EN_Project pr, int index, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
 **  Input:   index = node index
 **           code  = node parameter code (see EPANET2.H)
 **           value = parameter value
 **  Output:  none
 **  Returns: error code
 **  Purpose: sets input parameter value for a node
 **----------------------------------------------------------------
 */
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  Snode *Node = net->Node;
  Stank *Tank = net->Tank;

  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  const int Npats = net->Npats;

  double *Ucf = pr->Ucf;

  int j;
  Pdemand demand;
  Psource source;
  double Htmp;
  double value = v;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Nnodes)
    return set_error(pr->error_handle, 203);
  switch (code) {
  case EN_ELEVATION:
    if (index <= Njuncs)
      Node[index].El = value / Ucf[ELEV];
    else {
      value = (value / Ucf[ELEV]) - Node[index].El;
      j = index - Njuncs;
      Tank[j].H0 += value;
      Tank[j].Hmin += value;
      Tank[j].Hmax += value;
      Node[index].El += value;
      hyd->NodeHead[index] += value;
    }
    break;

  case EN_BASEDEMAND:
    /* NOTE: primary demand category is last on demand list */
    if (index <= Njuncs) {
      for (demand = Node[index].D; demand != NULL; demand = demand->next) {
        if (demand->next == NULL)
          demand->Base = value / Ucf[FLOW];
      }
    }
    break;

  case EN_PATTERN:
    /* NOTE: primary demand category is last on demand list */
    j = ROUND(value);
    if (j < 0 || j > Npats)
      return set_error(pr->error_handle, 205);
    if (index <= Njuncs) {
      for (demand = Node[index].D; demand != NULL; demand = demand->next) {
        if (demand->next == NULL)
          demand->Pat = j;
      }
    } else
      Tank[index - Njuncs].Pat = j;
    break;

  case EN_EMITTER:
    if (index > Njuncs)
      return set_error(pr->error_handle, 203);
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    if (value > 0.0)
      value = pow((Ucf[FLOW] / value), hyd->Qexp) / Ucf[PRESSURE];
    Node[index].Ke = value;
    break;

  case EN_INITQUAL:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    Node[index].C0 = value / Ucf[QUALITY];
    if (index > Njuncs)
      Tank[index - Njuncs].C = Node[index].C0;
    break;

  case EN_SOURCEQUAL:
  case EN_SOURCETYPE:
  case EN_SOURCEPAT:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    source = Node[index].S;
    if (source == NULL) {
      source = (struct Ssource *)malloc(sizeof(struct Ssource));
      if (source == NULL)
        return set_error(pr->error_handle, 101);
      source->Type = CONCEN;
      source->C0 = 0.0;
      source->Pat = 0;
      Node[index].S = source;
    }
    if (code == EN_SOURCEQUAL) {
      source->C0 = value;
    } else if (code == EN_SOURCEPAT) {
      j = ROUND(value);
      if (j < 0 || j > Npats)
        return set_error(pr->error_handle, 205);
      source->Pat = j;
    } else // code == EN_SOURCETYPE
    {
      j = ROUND(value);
      if (j < CONCEN || j > FLOWPACED)
        return set_error(pr->error_handle, 251);
      else
        source->Type = (char)j;
    }
    return set_error(pr->error_handle, 0);

  case EN_TANKLEVEL:
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    j = index - Njuncs;
    if (Tank[j].A == 0.0) /* Tank is a reservoir */
    {
      Tank[j].H0 = value / Ucf[ELEV];
      Tank[j].Hmin = Tank[j].H0;
      Tank[j].Hmax = Tank[j].H0;
      Node[index].El = Tank[j].H0;
      hyd->NodeHead[index] = Tank[j].H0;
    } else {
      value = Node[index].El + value / Ucf[ELEV];
      if (value > Tank[j].Hmax || value < Tank[j].Hmin)
        return set_error(pr->error_handle, 202);
      Tank[j].H0 = value;
      Tank[j].V0 = tankvolume(pr, j, Tank[j].H0);
      // Resetting Volume in addition to initial volume
      Tank[j].V = Tank[j].V0;
      hyd->NodeHead[index] = Tank[j].H0;
    }
    break;

    /*** New parameters added for retrieval begins here   ***/
  /*** (Thanks to Nicolas Basile of Ecole Polytechnique ***/
  /***  de Montreal for suggesting some of these.)      ***/

  case EN_TANKDIAM:
    if (value <= 0.0)
      return set_error(pr->error_handle, 202);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      value /= Ucf[ELEV];
      Tank[j].A = PI * SQR(value) / 4.0;
      Tank[j].Vmin = tankvolume(pr, j, Tank[j].Hmin);
      Tank[j].V0 = tankvolume(pr, j, Tank[j].H0);
      Tank[j].Vmax = tankvolume(pr, j, Tank[j].Hmax);
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

  case EN_MINVOLUME:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].Vmin = value / Ucf[VOLUME];
      Tank[j].V0 = tankvolume(pr, j, Tank[j].H0);
      Tank[j].Vmax = tankvolume(pr, j, Tank[j].Hmax);
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

  case EN_MINLEVEL:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251); // not a tank or reservoir
    j = index - Njuncs;
    if (Tank[j].A == 0.0)
      return set_error(pr->error_handle, 251); // node is a reservoir
    Htmp = value / Ucf[ELEV] + Node[index].El;
    if (Htmp < Tank[j].Hmax && Htmp <= Tank[j].H0) {
      if (Tank[j].Vcurve > 0)
        return set_error(pr->error_handle, 202);
      Tank[j].Hmin = Htmp;
      Tank[j].Vmin = (Tank[j].Hmin - Node[index].El) * Tank[j].A;
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

  case EN_MAXLEVEL:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251); // not a tank or reservoir
    j = index - Njuncs;
    if (Tank[j].A == 0.0)
      return set_error(pr->error_handle, 251); // node is a reservoir
    Htmp = value / Ucf[ELEV] + Node[index].El;
    if (Htmp > Tank[j].Hmin && Htmp >= Tank[j].H0) {
      if (Tank[j].Vcurve > 0)
        return set_error(pr->error_handle, 202);
      Tank[j].Hmax = Htmp;
      Tank[j].Vmax = tankvolume(pr, j, Tank[j].Hmax);
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

  case EN_MIXMODEL:
    j = ROUND(value);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    if (j < MIX1 || j > LIFO)
      return set_error(pr->error_handle, 202);
    if (index > Njuncs && Tank[index - Njuncs].A > 0.0) {
      Tank[index - Njuncs].MixModel = (char)j;
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

  case EN_MIXFRACTION:
    if (value < 0.0 || value > 1.0)
      return set_error(pr->error_handle, 202);
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].V1max = value * Tank[j].Vmax;
    }
    break;

  case EN_TANK_KBULK:
    if (index <= Njuncs)
      return set_error(pr->error_handle, 251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].Kb = value / SECperDAY;
      qu->Reactflag = 1;
    } else {
      return set_error(pr->error_handle, 251);
    }
    break;

    /***  New parameter additions ends here. ***/

  default:
    return set_error(pr->error_handle, 251);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setlinkid(EN_Project pr, int index, char *newid)
{
    EN_Network *net = &pr->network;
    size_t n;

    // Check for valid arguments
    if (index <= 0 || index > net->Nlinks)
    {
        return set_error(pr->error_handle, 204);
    }
    n = strlen(newid);
    if (n < 1 || n > MAXID) return set_error(pr->error_handle, 211);
    if (strcspn(newid, " ;") < n) return set_error(pr->error_handle, 211);

    // Check if another link with same name exists
    if (hashtable_find(net->LinkHashTable, newid) > 0)
    {
        return set_error(pr->error_handle, 215);
    }

    // Replace the existing link ID with the new value
    hashtable_delete(net->LinkHashTable, net->Link[index].ID);
    strncpy(net->Link[index].ID, newid, MAXID);
    hashtable_insert(net->LinkHashTable, net->Link[index].ID, index);
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setlinknodes(EN_Project pr, int index, int node1, int node2)
{
    int type;
    EN_Network *net = &pr->network;

    // Check that end and start nodes are not the same
    if (node1 == node2) return set_error(pr->error_handle, 222);

    // Check that nodes exist
    if (node1 < 0 || node1 > net->Nnodes) return set_error(pr->error_handle, 203);
    if (node2 < 0 || node2 > net->Nnodes) return set_error(pr->error_handle, 203);

    // Check for illegal valve connection
    type = net->Link[index].Type;
    if (type == EN_PRV || type == EN_PSV || type == EN_FCV)
    {
        // Can't be connected to a fixed grade node
        if (node1 > net->Njuncs ||
            node2 > net->Njuncs) return set_error(pr->error_handle, 219);

        // Can't be connected to another pressure/flow control valve
        if (!valvecheck(pr, type, node1, node2))
        {
            return set_error(pr->error_handle, 220);
        }
    }

    // Assign new end nodes to link
    net->Link[index].N1 = node1;
    net->Link[index].N2 = node2;
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setlinkvalue(EN_Project pr, int index, int code,
                              EN_API_FLOAT_TYPE v)

/*----------------------------------------------------------------
 **  Input:   index = link index
 **           code  = link parameter code (see EPANET2.H)
 **           v = parameter value
 **  Output:  none
 **  Returns: error code
 **  Purpose: sets input parameter value for a link
 **----------------------------------------------------------------
 */
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  Slink *Link = net->Link;

  const int Nlinks = net->Nlinks;

  double *Ucf = pr->Ucf;
  double *LinkSetting = hyd->LinkSetting;

  char s;
  double r, value = v;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Nlinks)
    return set_error(pr->error_handle, 204);
  switch (code) {
  case EN_DIAMETER:
    if (Link[index].Type != EN_PUMP) {
      if (value <= 0.0)
        return set_error(pr->error_handle, 202);
      value /= Ucf[DIAM];                /* Convert to feet */
      r = Link[index].Diam / value;      /* Ratio of old to new diam */
      Link[index].Km *= SQR(r) * SQR(r); /* Adjust minor loss factor */
      Link[index].Diam = value;          /* Update diameter */
      resistcoeff(pr, index);             /* Update resistance coeff. */
    }
    break;

  case EN_LENGTH:
    if (Link[index].Type <= EN_PIPE) {
      if (value <= 0.0)
        return set_error(pr->error_handle, 202);
      Link[index].Len = value / Ucf[ELEV];
      resistcoeff(pr, index);
    }
    break;

  case EN_ROUGHNESS:
    if (Link[index].Type <= EN_PIPE) {
      if (value <= 0.0)
        return set_error(pr->error_handle, 202);
      Link[index].Kc = value;
      if (hyd->Formflag == DW)
        Link[index].Kc /= (1000.0 * Ucf[ELEV]);
      resistcoeff(pr, index);
    }
    break;

  case EN_MINORLOSS:
    if (Link[index].Type != EN_PUMP) {
      if (value <= 0.0)
        return set_error(pr->error_handle, 202);
      Link[index].Km =
          0.02517 * value / SQR(Link[index].Diam) / SQR(Link[index].Diam);
    }
    break;

  case EN_INITSTATUS:
  case EN_STATUS:
    /* Cannot set status for a check valve */
    if (Link[index].Type == EN_CVPIPE)
      return set_error(pr->error_handle, 207);
    s = (char)ROUND(value);
    if (s < 0 || s > 1)
      return set_error(pr->error_handle, 251);
    if (code == EN_INITSTATUS)
      setlinkstatus(pr, index, s, &Link[index].Stat, &Link[index].Kc);
    else
      setlinkstatus(pr, index, s, &hyd->LinkStatus[index], &LinkSetting[index]);
    break;

  case EN_INITSETTING:
  case EN_SETTING:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
      return set_error(pr->error_handle, EN_setlinkvalue(pr, index, EN_ROUGHNESS, v));
    else {
      switch (Link[index].Type) {
      case EN_PUMP:
        break;
      case EN_PRV:
      case EN_PSV:
      case EN_PBV:
        value /= Ucf[PRESSURE];
        break;
      case EN_FCV:
        value /= Ucf[FLOW];
        break;
      case EN_TCV:
        break;

      /***  Updated 9/7/00  ***/
      case EN_GPV:
        return set_error(pr->error_handle, 202); /* Cannot modify setting for GPV */

      default:
        return set_error(pr->error_handle, 251);
      }
      if (code == EN_INITSETTING)
        setlinksetting(pr, index, value, &Link[index].Stat, &Link[index].Kc);
      else
        setlinksetting(pr, index, value, &hyd->LinkStatus[index],
                       &LinkSetting[index]);
    }
    break;

  case EN_KBULK:
    if (Link[index].Type <= EN_PIPE) {
      Link[index].Kb = value / SECperDAY;
      qu->Reactflag = 1;
    }
    break;

  case EN_KWALL:
    if (Link[index].Type <= EN_PIPE) {
      Link[index].Kw = value / SECperDAY;
      qu->Reactflag = 1;
    }
    break;

  default:
    return set_error(pr->error_handle, 251);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_addpattern(EN_Project pr, char *id) {
  int i, j, n, err = 0;
  Spattern *tmpPat;

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  hydraulics_t *hyd = &pr->hydraulics;
  Spattern *Pattern = net->Pattern;

  const int Npats = net->Npats;


  /* Check if a pattern with same id already exists */

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (EN_getpatternindex(pr, id, &i) == 0)
    return set_error(pr->error_handle, 215);

  /* Check that id name is not too long */

  if (strlen(id) > MAXID)
    return set_error(pr->error_handle, 250);

  /* Allocate memory for a new array of patterns */

  n = Npats + 1;
  tmpPat = (Spattern *)calloc(n + 1, sizeof(Spattern));
  if (tmpPat == NULL)
    return set_error(pr->error_handle, 101);

  /* Copy contents of old pattern array to new one */

  for (i = 0; i <= net->Npats; i++) {
    strcpy(tmpPat[i].ID, Pattern[i].ID);
    tmpPat[i].Length = Pattern[i].Length;
    tmpPat[i].F = (double *)calloc(Pattern[i].Length, sizeof(double));
    if (tmpPat[i].F == NULL)
      err = 1;
    else
      for (j = 0; j < Pattern[i].Length; j++)
        tmpPat[i].F[j] = Pattern[i].F[j];
  }

  /* Add the new pattern to the new array of patterns */

  strcpy(tmpPat[n].ID, id);
  tmpPat[n].Length = 1;
  tmpPat[n].F = (double *)calloc(tmpPat[n].Length, sizeof(double));
  if (tmpPat[n].F == NULL)
    err = 1;
  else
    tmpPat[n].F[0] = 1.0;

  /* Abort if memory allocation error */

  if (err) {
    for (i = 0; i <= n; i++)
      if (tmpPat[i].F)
        free(tmpPat[i].F);
    free(tmpPat);
    return set_error(pr->error_handle, 101);
  }

  // Replace old pattern array with new one

  for (i = 0; i <= Npats; i++)
    free(Pattern[i].F);
  free(Pattern);
  net->Pattern = tmpPat;
  net->Npats = n;
  par->MaxPats = n;

  if (strcmp(id, par->DefPatID) == 0) {
      hyd->DefPat = n;
   }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpattern(EN_Project pr, int index, EN_API_FLOAT_TYPE *f, int n) {
  int j;

  EN_Network *net = &pr->network;
  Spattern *Pattern = net->Pattern;
  const int Npats = net->Npats;


  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Npats)
    return set_error(pr->error_handle, 205);
  if (n <= 0)
    return set_error(pr->error_handle, 202);

  /* Re-set number of time periods & reallocate memory for multipliers */
  Pattern[index].Length = n;
  Pattern[index].F = (double *)realloc(Pattern[index].F, n * sizeof(double));
  if (Pattern[index].F == NULL)
    return set_error(pr->error_handle, 101);

  /* Load multipliers into pattern */
  for (j = 0; j < n; j++)
    Pattern[index].F[j] = f[j];
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpatternvalue(EN_Project pr, int index, int period, EN_API_FLOAT_TYPE value) {

  EN_Network *net = &pr->network;

  Spattern *Pattern = net->Pattern;

  const int Npats = net->Npats;


  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Npats)
    return set_error(pr->error_handle, 205);
  if (period <= 0 || period > Pattern[index].Length)
    return set_error(pr->error_handle, 251);
  Pattern[index].F[period - 1] = value;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_addcurve(EN_Project pr, char *id) {

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Scurve *Curve = net->Curve;

  int i, j, n, err = 0;
  Scurve *tmpCur;

  /* Check if a curve with same id already exists */

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (EN_getcurveindex(pr, id, &i) == 0)
    return set_error(pr->error_handle, 215);

  /* Check that id name is not too long */

  if (strlen(id) > MAXID)
    return set_error(pr->error_handle, 250);

  /* Allocate memory for a new array of curves */

  n = net->Ncurves + 1;
  tmpCur = (Scurve *)calloc(n + 1, sizeof(Scurve));
  if (tmpCur == NULL)
    return set_error(pr->error_handle, 101);

  /* Copy contents of old curve array to new one */

  for (i = 0; i <= net->Ncurves; i++) {
    strcpy(tmpCur[i].ID, net->Curve[i].ID);
    tmpCur[i].Npts = Curve[i].Npts;
    tmpCur[i].X = (double *)calloc(net->Curve[i].Npts, sizeof(double));
    tmpCur[i].Y = (double *)calloc(net->Curve[i].Npts, sizeof(double));
    if (tmpCur[i].X == NULL)
      err = 1;
    else if (tmpCur[i].Y == NULL)
      err = 1;
    else
      for (j = 0; j < Curve[i].Npts; j++) {
        tmpCur[i].X[j] = net->Curve[i].X[j];
        tmpCur[i].Y[j] = net->Curve[i].Y[j];
      }
  }

  /* Add the new Curve to the new array of curves */

  strcpy(tmpCur[n].ID, id);
  tmpCur[n].Npts = 1;
  tmpCur[n].Type = G_CURVE;
  tmpCur[n].X = (double *)calloc(tmpCur[n].Npts, sizeof(double));
  tmpCur[n].Y = (double *)calloc(tmpCur[n].Npts, sizeof(double));
  if (tmpCur[n].X == NULL)
    err = 1;
  else if (tmpCur[n].Y == NULL)
    err = 1;
  else {
    tmpCur[n].X[0] = 1.0;
    tmpCur[n].Y[0] = 1.0;
  }

  /* Abort if memory allocation error */

  if (err) {
    for (i = 0; i <= n; i++) {
      if (tmpCur[i].X)
        free(tmpCur[i].X);
      if (tmpCur[i].Y)
        free(tmpCur[i].Y);
    }
    free(tmpCur);
    return set_error(pr->error_handle, 101);
  }

  // Replace old curves array with new one

  for (i = 0; i <= net->Ncurves; i++) {
    free(net->Curve[i].X);
    free(net->Curve[i].Y);
  }
  free(net->Curve);
  net->Curve = tmpCur;
  net->Ncurves = n;
  par->MaxCurves = n;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setcurve(EN_Project pr, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y, int n) {

  EN_Network *net = &pr->network;
  Scurve *Curve = net->Curve;
  int j;

  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > net->Ncurves)
    return set_error(pr->error_handle, 206);
  if (n <= 0)
    return set_error(pr->error_handle, 202);

  /* Re-set number of points & reallocate memory for values */
  Curve[index].Npts = n;
  Curve[index].X = (double *)realloc(Curve[index].X, n * sizeof(double));
  Curve[index].Y = (double *)realloc(Curve[index].Y, n * sizeof(double));
  if (Curve[index].X == NULL)
    return set_error(pr->error_handle, 101);
  if (Curve[index].Y == NULL)
    return set_error(pr->error_handle, 101);

  /* Load values into curve */
  for (j = 0; j < n; j++) {
    Curve[index].X[j] = x[j];
    Curve[index].Y[j] = y[j];
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setcurvevalue(EN_Project pr, int index, int pnt, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y) {

  EN_Network *net = &pr->network;
  Scurve *Curve = net->Curve;
  const int Ncurves = net->Ncurves;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index <= 0 || index > Ncurves)
    return set_error(pr->error_handle, 206);
  if (pnt <= 0 || pnt > Curve[index].Npts)
    return set_error(pr->error_handle, 251);
  Curve[index].X[pnt - 1] = x;
  Curve[index].Y[pnt - 1] = y;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_settimeparam(EN_Project pr, int code, long value)
{
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (pr->hydraulics.OpenHflag || pr->quality.OpenQflag) {
    // --> there's nothing wrong with changing certain time parameters during a
    // simulation run, or before the run has started.
    // todo -- how to tell?
    /*
     if (code == EN_DURATION || code == EN_HTIME || code == EN_REPORTSTEP ||
     code == EN_DURATION || Htime == 0) {
     // it's ok
     }
     else {
     return(109);
     }
     */
  }
  if (value < 0)
    return set_error(pr->error_handle, 202);
  switch (code) {
  case EN_DURATION:
    time->Dur = value;
    if (time->Rstart > time->Dur)
      time->Rstart = 0;
    break;

  case EN_HYDSTEP:
    if (value == 0)
      return set_error(pr->error_handle, 202);
    time->Hstep = value;
    time->Hstep = MIN(time->Pstep, time->Hstep);
    time->Hstep = MIN(time->Rstep, time->Hstep);
    qu->Qstep = MIN(qu->Qstep, time->Hstep);
    break;

  case EN_QUALSTEP:
    if (value == 0)
      return set_error(pr->error_handle, 202);
    qu->Qstep = value;
    qu->Qstep = MIN(qu->Qstep, time->Hstep);
    break;

  case EN_PATTERNSTEP:
    if (value == 0)
      return set_error(pr->error_handle, 202);
    time->Pstep = value;
    if (time->Hstep > time->Pstep)
      time->Hstep = time->Pstep;
    break;

  case EN_PATTERNSTART:
    time->Pstart = value;
    break;

  case EN_REPORTSTEP:
    if (value == 0)
      return set_error(pr->error_handle, 202);
    time->Rstep = value;
    if (time->Hstep > time->Rstep)
      time->Hstep = time->Rstep;
    break;

  case EN_REPORTSTART:
    if (time->Rstart > time->Dur)
      return set_error(pr->error_handle, 202);
    time->Rstart = value;
    break;

  case EN_RULESTEP:
    if (value == 0)
      return set_error(pr->error_handle, 202);
    time->Rulestep = value;
    time->Rulestep = MIN(time->Rulestep, time->Hstep);
    break;

  case EN_STATISTIC:
    if (value > RANGE)
      return set_error(pr->error_handle, 202);
    rep->Tstatflag = (char)value;
    break;

  case EN_HTIME:
    time->Htime = value;
    break;

  case EN_QTIME:
    qu->Qtime = value;
    break;

  default:
    return set_error(pr->error_handle, 251);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setoption(EN_Project pr, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
 **  Input:   code  = option code (see EPANET2.H)
 **           v = option value
 **  Output:  none
 **  Returns: error code
 **  Purpose: sets value for an analysis option
 **----------------------------------------------------------------
 */
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  Snode *Node = net->Node;
  const int Njuncs = net->Njuncs;

  double *Ucf = pr->Ucf;

  int i, j;
  int tmpPat, error;
  char tmpId[MAXID+1];
  Pdemand demand; /* Pointer to demand record */
  double Ke, n, ucf, value = v;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  switch (code) {
  case EN_TRIALS:
    if (value < 1.0)
      return set_error(pr->error_handle, 202);
    hyd->MaxIter = (int)value;
    break;
  case EN_ACCURACY:
    if (value < 1.e-5 || value > 1.e-1)
      return set_error(pr->error_handle, 202);
    hyd->Hacc = value;
    break;
  case EN_TOLERANCE:
    if (value < 0.0)
      return set_error(pr->error_handle, 202);
    qu->Ctol = value / Ucf[QUALITY];
    break;
  case EN_EMITEXPON:
    if (value <= 0.0)
      return set_error(pr->error_handle, 202);
    n = 1.0 / value;
    ucf = pow(Ucf[FLOW], n) / Ucf[PRESSURE];
    for (i = 1; i <= Njuncs; i++) {
      j = EN_getnodevalue(pr, i, EN_EMITTER, &v);
      Ke = v;
      if (j == 0 && Ke > 0.0)
        Node[i].Ke = ucf / pow(Ke, n);
    }
    hyd->Qexp = n;
    break;
  case EN_DEMANDMULT:
    if (value <= 0.0)
      return set_error(pr->error_handle, 202);
    hyd->Dmult = value;
    break;
  case EN_HEADERROR:
    if (value < 0.0)
        return set_error(pr->error_handle, 202);
    hyd->HeadErrorLimit = value / Ucf[HEAD];
    break;
  case EN_FLOWCHANGE:
      if (value < 0.0)
          return set_error(pr->error_handle, 202);
      hyd->FlowChangeLimit = value / Ucf[FLOW];
      break;
  case EN_DEMANDDEFPAT:
    //check that the pattern exists or is set to zero to delete the default pattern
    if (value < 0 || value > net->Npats)
        return set_error(pr->error_handle, 205);
    tmpPat = hyd->DefPat;
    //get the new pattern ID
    if (value == 0)
    {
        strncpy(tmpId, "1", MAXID); // should be DEFPATID
    }
    else
    {
        error = EN_getpatternid(pr, (int)value, tmpId);
        if (error != 0)
            return set_error(pr->error_handle, error);
    }
    // replace node patterns for default
    for (i = 1; i <= net->Nnodes; i++) {
        Snode *node = &net->Node[i];
        for (demand = node->D; demand != NULL; demand = demand->next) {
            if (demand->Pat == tmpPat) {
               demand->Pat = (int)value;
               strcpy(demand->Name, "");
            }
        }
    }
    strncpy(pr->parser.DefPatID, tmpId, MAXID);
    hyd->DefPat = (int)value;
    break;

  default:
    return set_error(pr->error_handle, 251);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setstatusreport(EN_Project pr, int code) {
  int errcode = 0;

  if (code >= 0 && code <= 2)
    pr->report.Statflag = (char)code;
  else
    errcode = 202;

  return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_setqualtype(EN_Project pr, int qualcode, char *chemname, char *chemunits, char *tracenode) {

  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;

  double *Ucf = pr->Ucf;
  int i;

  /*** Updated 3/1/01 ***/
  double ccf = 1.0;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (qualcode < EN_NONE || qualcode > EN_TRACE)
    return set_error(pr->error_handle, 251);
  qu->Qualflag = (char)qualcode;
  qu->Ctol *= Ucf[QUALITY];
  if (qu->Qualflag == CHEM) /* Chemical constituent */
  {
    strncpy(qu->ChemName, chemname, MAXID);
    strncpy(qu->ChemUnits, chemunits, MAXID);

    /*** Updated 3/1/01 ***/
    strncpy(rep->Field[QUALITY].Units, qu->ChemUnits, MAXID);
    strncpy(rep->Field[REACTRATE].Units, qu->ChemUnits, MAXID);
    strcat(rep->Field[REACTRATE].Units, t_PERDAY);
    ccf = 1.0 / LperFT3;
  }
  if (qu->Qualflag == TRACE) /* Source tracing option */
  {
    qu->TraceNode = findnode(net,tracenode);
    if (qu->TraceNode == 0)
      return set_error(pr->error_handle, 203);
    strncpy(qu->ChemName, u_PERCENT, MAXID);
    strncpy(qu->ChemUnits, tracenode, MAXID);

    /*** Updated 3/1/01 ***/
    strcpy(rep->Field[QUALITY].Units, u_PERCENT);
  }
  if (qu->Qualflag == AGE) /* Water age analysis */
  {
    strncpy(qu->ChemName, w_AGE, MAXID);
    strncpy(qu->ChemUnits, u_HOURS, MAXID);

    /*** Updated 3/1/01 ***/
    strcpy(rep->Field[QUALITY].Units, u_HOURS);
  }

  /* when changing from CHEM to AGE or TRACE, nodes initial quality values must be returned to their original ones */
  if ((qu->Qualflag == AGE || qu->Qualflag == TRACE) & (Ucf[QUALITY] != 1)) {
    for (i=1; i<=pr->network.Nnodes; i++) {
      pr->network.Node[i].C0 *= Ucf[QUALITY];
    }
  }

  /*** Updated 3/1/01 ***/
  Ucf[QUALITY] = ccf;
  Ucf[LINKQUAL] = ccf;
  Ucf[REACTRATE] = ccf;
  qu->Ctol /= Ucf[QUALITY];

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getheadcurveindex(EN_Project pr, int index, int *curveindex) {

  EN_Network *net = &pr->network;
  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  const int Nlinks = net->Nlinks;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type)
    return set_error(pr->error_handle, 204);
  *curveindex = Pump[findpump(net, index)].Hcurve;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setheadcurveindex(EN_Project pr, int index, int curveindex) {

  EN_Network *net = &pr->network;

  Slink *Link = net->Link;
  const int Nlinks = net->Nlinks;
  const int Ncurves = net->Ncurves;

  double *Ucf = pr->Ucf;
  int pIdx;
  Spump *pump;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type) {
    return set_error(pr->error_handle, 204);
  }
  if (curveindex <= 0 || curveindex > Ncurves) {
    return set_error(pr->error_handle, 206);
  }
  pIdx = findpump(net, index);
  pump = &pr->network.Pump[pIdx];
  pump->Ptype = NOCURVE;
  pump->Hcurve = curveindex;
  // update pump parameters
  updatepumpparams(pr, pIdx);
  // convert units
  if (pump->Ptype == POWER_FUNC) {
    pump->H0 /= Ucf[HEAD];
    pump->R *= (pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
  }
  /* Convert flow range & max. head units */
  pump->Q0 /= Ucf[FLOW];
  pump->Qmax /= Ucf[FLOW];
  pump->Hmax /= Ucf[HEAD];

  pr->network.Curve[curveindex].Type = P_CURVE;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getpumptype(EN_Project pr, int index, int *type) {

  EN_Network *net = &pr->network;

  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  const int Nlinks = net->Nlinks;

  *type = -1;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type)
    return set_error(pr->error_handle, 204);
  *type = Pump[findpump(&pr->network, index)].Ptype;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getcurvetype(EN_Project pr, int curveindex, int *type) {

  EN_Network *net = &pr->network;

  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (curveindex < 1 || curveindex > net->Ncurves)
    return set_error(pr->error_handle, 206);
  *type = net->Curve[curveindex].Type;

  return set_error(pr->error_handle, 0);
}


int findnode(EN_Network *n, char *id)
/*----------------------------------------------------------------
**  Input:   id = node ID
**  Output:  none
**  Returns: index of node with given ID, or 0 if ID not found
**  Purpose: uses hash table to find index of node with given ID
**----------------------------------------------------------------
*/
{
  return (hashtable_find(n->NodeHashTable, id));
}

int findlink(EN_Network *n, char *id)
/*----------------------------------------------------------------
**  Input:   id = link ID
**  Output:  none
**  Returns: index of link with given ID, or 0 if ID not found
**  Purpose: uses hash table to find index of link with given ID
**----------------------------------------------------------------
*/
{
  return (hashtable_find(n->LinkHashTable, id));
}

int findtank(EN_Network *n, int index)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  none
**  Returns: index of tank with given node id, or NOTFOUND if tank not found
**  Purpose: for use in the deletenode function
**----------------------------------------------------------------
*/
{
  int i;
  for (i = 1; i <= n->Ntanks; i++) {
    if (n->Tank[i].Node == index) {
      return (i);
    }
  }
  return (NOTFOUND);
}

int findpump(EN_Network *n, int index)
/*----------------------------------------------------------------
**  Input:   index = link ID
**  Output:  none
**  Returns: index of pump with given link id, or NOTFOUND if pump not found
**  Purpose: for use in the deletelink function
**----------------------------------------------------------------
*/
{
  int i;
  for (i = 1; i <= n->Npumps; i++) {
    if (n->Pump[i].Link == index) {
      return (i);
    }
  }
  return (NOTFOUND);
}

int findvalve(EN_Network *n, int index)
/*----------------------------------------------------------------
**  Input:   index = link ID
**  Output:  none
**  Returns: index of valve with given link id, or NOTFOUND if valve not found
**  Purpose: for use in the deletelink function
**----------------------------------------------------------------
*/
{
  int i;
  for (i = 1; i <= n->Nvalves; i++) {
    if (n->Valve[i].Link == index) {
      return (i);
    }
  }
  return (NOTFOUND);
}

char *geterrmsg(int errcode, char *msg)
/*----------------------------------------------------------------
**  Input:   errcode = error code
**  Output:  none
**  Returns: pointer to string with error message
**  Purpose: retrieves text of error message
**----------------------------------------------------------------
*/
{
  switch (errcode) { /* Warnings */
#define DAT(code,enumer,string) case code: strcpy(msg, string); break;
//#define DAT(code,enumer,string) case code: sprintf(msg, "Error %d: %s", code, string); break;
#include "errors.dat"
#undef DAT
    default:
      strcpy(msg, "");
  }
  return (msg);
}

void errmsg(Project *pr, int errcode)
/*----------------------------------------------------------------
**  Input:   errcode = error code
**  Output:  none
**  Purpose: writes error message to report file
**----------------------------------------------------------------
*/
{
  if (errcode == 309) /* Report file write error -  */
  {                   /* Do not write msg to file.  */

  } else if (pr->report.RptFile != NULL && pr->report.Messageflag) {
    writeline(pr, geterrmsg(errcode,pr->Msg));
  }
}

void writewin(void (*vp)(char *), char *s)
/*----------------------------------------------------------------
**  Input:   text string
**  Output:  none
**  Purpose: passes character string to viewprog() in
**           application which calls the EPANET DLL
**----------------------------------------------------------------
*/
{
  char progmsg[MAXMSG + 1];
  if (vp != NULL) {
    strncpy(progmsg, s, MAXMSG);
    vp(progmsg);
  }
}

int DLLEXPORT EN_getnumdemands(EN_Project pr, int nodeIndex, int *numDemands) {
  Pdemand d;
  int n = 0;

  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  for (d = pr->network.Node[nodeIndex].D; d != NULL; d = d->next)
    n++;
  *numDemands = n;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getbasedemand(EN_Project pr, int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE *baseDemand) {
  Pdemand d;
  int n = 1;

  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > pr->network.Nnodes)
    return set_error(pr->error_handle, 203);
  if (nodeIndex <= pr->network.Njuncs) {
    for (d = pr->network.Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next) {
      n++;
    }
    if (n != demandIdx) {
      return set_error(pr->error_handle, 253);
    }
    *baseDemand = (EN_API_FLOAT_TYPE)(d->Base * pr->Ucf[FLOW]);
  } else {
    *baseDemand = (EN_API_FLOAT_TYPE)(0.0);
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setbasedemand(EN_Project pr, int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE baseDemand) {

  EN_Network *net = &pr->network;
  Snode *Node = net->Node;

  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;

  double *Ucf = pr->Ucf;

  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > Nnodes)
    return set_error(pr->error_handle, 203);
  if (nodeIndex <= Njuncs) {
    for (d = Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next)
      n++;
    if (n != demandIdx)
      return set_error(pr->error_handle, 253);
    d->Base = baseDemand / Ucf[FLOW];
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getdemandname(EN_Project pr, int nodeIndex, int demandIdx, char *demandName) {
  Pdemand d;
  int n = 1;

  strcpy(demandName, "");
  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > pr->network.Njuncs)
    return set_error(pr->error_handle, 203);
  for (d = pr->network.Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next) {
    n++;
  }
  if (n != demandIdx) {
    return set_error(pr->error_handle, 253);
  }
  strcpy(demandName, d->Name);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setdemandname(EN_Project pr, int nodeIndex, int demandIdx, char *demandName) {

  EN_Network *net = &pr->network;
  Snode *Node = net->Node;

  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;

  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > Njuncs)
    return set_error(pr->error_handle, 203);
  for (d = Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next)
    n++;
  if (n != demandIdx)
    return set_error(pr->error_handle, 253);
  strncpy(d->Name, demandName, MAXMSG);
  return set_error(pr->error_handle, 0);
}

int  DLLEXPORT EN_setdemandpattern(EN_Project pr, int nodeIndex, int demandIdx, int patIndex) {

  EN_Network *net = &pr->network;
  Snode *Node = net->Node;

  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  const int Npats = net->Npats;

  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > Nnodes)
    return set_error(pr->error_handle, 203);
  if (patIndex < 1 || patIndex > Npats)
    return(205);
  if (nodeIndex <= Njuncs) {
    for (d = Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next)
      n++;
    if (n != demandIdx)
      return set_error(pr->error_handle, 253);
  d->Pat = patIndex;
  }
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getdemandpattern(EN_Project pr, int nodeIndex, int demandIdx, int *pattIdx) {

  EN_Network *net = &pr->network;
  Snode *Node = net->Node;
  const int Nnodes = net->Nnodes;

  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (nodeIndex <= 0 || nodeIndex > Nnodes)
    return set_error(pr->error_handle, 203);
  for (d = Node[nodeIndex].D; n < demandIdx && d->next != NULL; d = d->next)
    n++;
  if (n != demandIdx)
    return set_error(pr->error_handle, 253);
  *pattIdx = d->Pat;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getaveragepatternvalue(EN_Project pr, int index, EN_API_FLOAT_TYPE *value) {

  EN_Network *net = &pr->network;

  Spattern *Pattern = net->Pattern;
  const int Npats = net->Npats;

  int i;
  *value = 0.0;
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (index < 1 || index > Npats)
    return set_error(pr->error_handle, 205);
  // if (period < 1 || period > Pattern[index].Length) return(251);
  for (i = 0; i < Pattern[index].Length; i++) {
    *value += (EN_API_FLOAT_TYPE)Pattern[index].F[i];
  }
  *value /= (EN_API_FLOAT_TYPE)Pattern[index].Length;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setlinktype(EN_Project pr, int *index, EN_LinkType type) {

    int i = *index, n1, n2;
    char id[MAXID+1];
    char id1[MAXID+1];
    char id2[MAXID+1];
    int errcode;
    EN_LinkType oldtype;
    EN_Network *net = &pr->network;

    if (!pr->Openflag) return set_error(pr->error_handle, 102);
    if (type < 0 || type > EN_GPV) return set_error(pr->error_handle, 211);

    // Check if a link with the id exists
    if (i <= 0 || i > net->Nlinks) return set_error(pr->error_handle, 204);

    // Get the current type of the link
    EN_getlinktype(pr, i, &oldtype);
    if (oldtype == type) return set_error(pr->error_handle, 0);

    // Pipe changing from or to having a check valve
    if (oldtype <= EN_PIPE && type <= EN_PIPE)
    {
        net->Link[i].Type = type;
        if (type == EN_CVPIPE) net->Link[i].Stat = OPEN;
        return set_error(pr->error_handle, 0);
    }

    // Get ID's of link & its end nodes
    EN_getlinkid(pr, i, id);
    EN_getlinknodes(pr, i, &n1, &n2);
    EN_getnodeid(pr, n1, id1);
    EN_getnodeid(pr, n2, id2);

    // Delete the original link
    EN_deletelink(pr, i);

    // Create a new link of new type and old id
    errcode = EN_addlink(pr, id, type, id1, id2);

    // Find the index of this new link
    EN_getlinkindex(pr, id, index);
    return set_error(pr->error_handle, errcode);
}

int DLLEXPORT EN_addnode(EN_Project pr, char *id, EN_NodeType nodeType) {
  int i, nIdx;
  int index;
  struct Sdemand *demand;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  Stank *tank;
  Snode *node;
  Scoord *coord;
  Scontrol *control;

  /* Check if a node with same id already exists */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (EN_getnodeindex(pr, id, &i) == 0)
    return set_error(pr->error_handle, 215);

  /* Check that id name is not too long */
  if (strlen(id) > MAXID)
    return set_error(pr->error_handle, 250);

  /* Grow arrays to accomodate the new values */
  net->Node = (Snode *)realloc(net->Node, (net->Nnodes + 2) * sizeof(Snode));
  net->Coord = realloc(net->Coord, (net->Nnodes + 2) * sizeof(Scoord));
  hyd->NodeDemand = (double *)realloc(hyd->NodeDemand, (net->Nnodes + 2) * sizeof(double));
  qu->NodeQual = (double *)realloc(qu->NodeQual, (net->Nnodes + 2) * sizeof(double));
  hyd->NodeHead = (double *)realloc(hyd->NodeHead, (net->Nnodes + 2) * sizeof(double));

  // Actions taken when a new Junction is added
  if (nodeType == EN_JUNCTION) {
    net->Njuncs++;
    nIdx = net->Njuncs;
    node = &net->Node[nIdx];
    coord = &net->Coord[nIdx];

    demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
    demand->Base = 0.0;
    demand->Pat = hyd->DefPat; // Use default pattern
    strcpy(demand->Name, "");
    demand->next = NULL;
    node->D = demand;

    // shift rest of Node array
    for (index = net->Nnodes; index >= net->Njuncs; index--) {
      hashtable_update(net->NodeHashTable, net->Node[index].ID, index + 1);
      net->Node[index + 1] = net->Node[index];
      net->Coord[index + 1] = net->Coord[index];
    }
    // shift indices of Tank array
    for (index = 1; index <= net->Ntanks; index++) {
      net->Tank[index].Node += 1;
    }

    // shift indices of Links, if necessary
    for (index = 1; index <= net->Nlinks; index++) {
      if (net->Link[index].N1 > net->Njuncs - 1) {
        net->Link[index].N1 += 1;
      }
      if (net->Link[index].N2 > net->Njuncs - 1) {
        net->Link[index].N2 += 1;
      }
    }

    // shift indices of Controls,
    // for high-index nodes (tanks/reservoirs)
    for (index = 1; index <= net->Ncontrols; ++index) {
      control = &net->Control[index];
      if (control->Node > net->Njuncs - 1) {
        control->Node += 1;
      }
    }

    // adjust indices of tanks/reservoirs in Rule premises (see RULES.C)
    adjusttankrules(pr);

  // Actions taken when a new Tank/Reservoir is added
  } else {
    nIdx = net->Nnodes+1;
    node = &net->Node[nIdx];
    coord = &net->Coord[nIdx];
    net->Ntanks++;

    /* resize tanks array */
    net->Tank = (Stank *)realloc(net->Tank, (net->Ntanks + 1) * sizeof(Stank));

    tank = &net->Tank[net->Ntanks];

    /* set default values for new tank or reservoir */
    tank->Node = nIdx;
    tank->Pat = 0;
    if (nodeType == EN_TANK) {
      tank->A = 1.0;
    } else {
      tank->A = 0;
    }
    tank->Hmin = 0;
    tank->Hmax = 0;
    tank->H0 = 0;
    tank->Vmin = 0;
    tank->Vmax = 0;
    tank->V0 = 0;
    tank->Kb = 0;
    tank->V = 0;
    tank->C = 0;
    tank->Pat = 0;
    tank->Vcurve = 0;
    tank->MixModel = 0;
    tank->V1max = 10000;
  }

  net->Nnodes++;

  /* set default values for new node */
  strncpy(node->ID, id, MAXID);

  node->El = 0;
  node->S = NULL;
  node->C0 = 0;
  node->Ke = 0;
  node->Rpt = 0;
  strcpy(node->Comment, "");

  coord->HaveCoords = FALSE;
  coord->X = 0;
  coord->Y = 0;

  /* Insert new node into hash table */
  hashtable_insert(net->NodeHashTable, node->ID, nIdx); /* see HASH.C */
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_addlink(EN_Project pr, char *id, EN_LinkType linkType, char *fromNode,
                        char *toNode) {
  int i, n;
  int N1, N2;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *link;
  Spump *pump;

  /* Check if a link with same id already exists */
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (EN_getlinkindex(pr, id, &i) == 0)
    return set_error(pr->error_handle, 215);

  /* Lookup the from and to nodes */
  N1 = hashtable_find(net->NodeHashTable, fromNode);
  N2 = hashtable_find(net->NodeHashTable, toNode);

  if (N1 == 0 || N2 == 0) {
    return set_error(pr->error_handle, 203);
  }

  /* Check that id name is not too long */
  if (strlen(id) > MAXID)
    return set_error(pr->error_handle, 250);

  net->Nlinks++;
  n = net->Nlinks;

  /* Grow arrays to accomodate the new value */
  net->Link = (Slink *)realloc(net->Link, (net->Nlinks + 1) * sizeof(Slink));
  hyd->LinkFlows = (double *)realloc(hyd->LinkFlows, (net->Nlinks + 1) * sizeof(double));
  hyd->LinkSetting = (double *)realloc(hyd->LinkSetting, (net->Nlinks + 1) * sizeof(double));
  hyd->LinkStatus = (StatType *)realloc(hyd->LinkStatus, (net->Nlinks + 1) * sizeof(StatType));

  link = &net->Link[net->Nlinks];

  strncpy(net->Link[n].ID, id, MAXID);

  if (linkType <= EN_PIPE) {
    net->Npipes++;
  } else if (linkType == EN_PUMP) {
    net->Npumps++;
    /* Grow pump array to accomodate the new value */
    net->Pump = (Spump *)realloc(net->Pump, (net->Npumps + 1) * sizeof(Spump));
    pump = &net->Pump[net->Npumps];

    pump->Link = n;
    pump->Ptype = 0;
    pump->Q0 = 0;
    pump->Qmax = 0;
    pump->Hmax = 0;
    pump->H0 = 0;
    pump->R = 0;
    pump->N = 0;
    pump->Hcurve = 0;
    pump->Ecurve = 0;
    pump->Upat = 0;
    pump->Epat = 0;
    pump->Ecost = 0;
    pump->Energy[5] = MISSING;

  } else {

    /* Grow valve array to accomodate the new value */
    net->Nvalves++;
    net->Valve = (Svalve *)realloc(net->Valve, (net->Nvalves + 1) * sizeof(Svalve));
    net->Valve[net->Nvalves].Link = n;
  }

  link->Type = linkType;
  link->N1 = N1;
  link->N2 = N2;
  link->Stat = OPEN;

  if (linkType == EN_PUMP) {
    link->Kc = 1.0; // Speed factor
    link->Km = 0.0; // Horsepower
    link->Len = 0.0;
  } else if (linkType <= EN_PIPE) { // pipe or cvpipe
    link->Diam = 10 / pr->Ucf[DIAM];
    link->Kc = 100; // Rough. coeff
    link->Km = 0.0; // Loss coeff
    link->Len = 1000;
  } else { // Valve
    link->Diam = 10 / pr->Ucf[DIAM];
    link->Kc = 0.0; // Valve setting.
    link->Km = 0.0; // Loss coeff
    link->Len = 0.0;
    link->Stat = ACTIVE;
  }
  link->Kb = 0;
  link->Kw = 0;
  link->R = 0;
  link->Rc = 0;
  link->Rpt = 0;
  strcpy(link->Comment, "");

  hashtable_insert(net->LinkHashTable, link->ID, n);
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_deletelink(EN_Project pr, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  none
**  Returns: error code
**  Purpose: deletes a link from a project.
**----------------------------------------------------------------
*/
{
    int i;
    int pumpindex;
    int valveindex;

    EN_LinkType linkType;
    EN_Network *net = &pr->network;
    Slink *link;

    // Check that link exists
    if (!pr->Openflag) return set_error(pr->error_handle, 102);
    if (index <= 0 || index > net->Nlinks) return set_error(pr->error_handle, 203);

    // Get references to the link and its type
    link = &net->Link[index];
    EN_getlinktype(pr, index, &linkType);

    // Remove link from hash table
    hashtable_delete(net->LinkHashTable, link->ID);

    // Shift position of higher entries in Link array down one
    for (i = index; i <= net->Nlinks - 1; i++)
    {
        net->Link[i] = net->Link[i + 1];
        // ... update hashtable
        hashtable_update(net->LinkHashTable, net->Link[i].ID, i);
    }

    // Adjust references to higher numbered links for pumps & valves
    for (i = 1; i <= net->Npumps; i++)
    {
        if (net->Pump[i].Link > index) net->Pump[i].Link -= 1;
    }
    for (i = 1; i <= net->Nvalves; i++)
    {
        if (net->Valve[i].Link > index) net->Valve[i].Link -= 1;
    }

    // Delete any pump associated with the deleted link
    if (linkType == EN_PUMP)
    {
        pumpindex = findpump(net,index);
        for (i = pumpindex; i <= net->Npumps - 1; i++)
        {
            net->Pump[i] = net->Pump[i + 1];
        }
        net->Npumps--;
    }

    // Delete any valve (linkType > EN_PUMP) associated with the deleted link
    if (linkType > EN_PUMP)
    {
        valveindex = findvalve(net,index);
        for (i = valveindex; i <= net->Nvalves - 1; i++)
        {
            net->Valve[i] = net->Valve[i + 1];
        }
        net->Nvalves--;
    }

    // Delete any control containing the link
    for (i = net->Ncontrols; i >= 1; i--)
    {
        if (net->Control[i].Link == index) EN_deletecontrol(pr, i);
    }

    // Adjust higher numbered link indices in remaining controls
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (net->Control[i].Link > index) net->Control[i].Link--;
    }

    // Make necessary adjustments to rule-based controls (r_LINK = 7)
    adjustrules(pr, 7, index);  // see RULES.C

    // Reduce link count by one
    net->Nlinks--;
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_deletenode(EN_Project pr, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  none
**  Returns: error code
**  Purpose: deletes a node from a project.
**----------------------------------------------------------------
*/
{
    int i, nodeType, tankindex;
    EN_Network *net = &pr->network;
    Snode *node;
    Pdemand demand, nextdemand;
    Psource source;

    // Check that node exists
    if (!pr->Openflag) return set_error(pr->error_handle, 102);
    if (index <= 0 || index > net->Nnodes)
    {
        return set_error(pr->error_handle, 203);
    }

    // Get a reference to the node & its type
    node = &net->Node[index];
    EN_getnodetype(pr, index, &nodeType);

    // Remove node from hash table
    hashtable_delete(net->NodeHashTable, node->ID);

    // Free memory allocated to node's demands & WQ source
    demand = node->D;
    while (demand != NULL)
    {
        nextdemand = demand->next;
        free(demand);
        demand = nextdemand;
    }
    source = node->S;
    if (source != NULL) free(source);

    // Shift position of higher entries in Node 7 Cord arrays down one
    for (i = index; i <= net->Nnodes - 1; i++)
    {
        net->Node[i] = net->Node[i + 1];
        net->Coord[i] = net->Coord[i + 1];
        // ... update hashtable
        hashtable_update(net->NodeHashTable, net->Node[i].ID, i);
    }

    // Remove references to demands & source in last (inactive) Node array entry
    net->Node[net->Nnodes].D = NULL;
    net->Node[net->Nnodes].S = NULL;

    // If deleted node is a tank, remove it from the Tank array
    if (nodeType != EN_JUNCTION)
    {
        tankindex = findtank(net, index);
        for (i = tankindex; i <= net->Ntanks - 1; i++)
        {
            net->Tank[i] = net->Tank[i + 1];
        }
    }

    // Shift higher node indices in Tank array down one
    for (i = 1; i <= net->Ntanks; i++)
    {
        if (net->Tank[i].Node > index) net->Tank[i].Node -= 1;
    }

    // Delete any links connected to the deleted node
    // (Process links in reverse order to maintain their indexing)
    for (i = net->Nlinks; i >= 1; i--)
    {
        if (net->Link[i].N1 == index ||
            net->Link[i].N2 == index)  EN_deletelink(pr, i);
    }

    // Adjust indices of all link end nodes
    for (i = 1; i <= net->Nlinks; i++)
    {
        if (net->Link[i].N1 > index) net->Link[i].N1 -= 1;
        if (net->Link[i].N2 > index) net->Link[i].N2 -= 1;
    }

    // Delete any control containing the node
    for (i = net->Ncontrols; i >= 1; i--)
    {
        if (net->Control[i].Node == index) EN_deletecontrol(pr, i);
    }

    // Adjust higher numbered link indices in remaining controls
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (net->Control[i].Node > index) net->Control[i].Node--;
    }

    // Make necessary adjustments to rule-based controls (r_NODE = 6)
    adjustrules(pr, 6, index);

    // Set water quality analysis to NONE if deleted node is trace node
    if (pr->quality.Qualflag == TRACE && pr->quality.TraceNode == index)
    {
        pr->quality.TraceNode = 0;
        pr->quality.Qualflag = NONE;
    }

    // Reduce counts of node types
    if (nodeType == EN_JUNCTION) net->Njuncs--;
    else net->Ntanks--;
    net->Nnodes--;
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_deletecontrol(EN_Project pr, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  none
**  Returns: error code
**  Purpose: deletes a simple control from a project.
**----------------------------------------------------------------
*/
{
    int i;
    EN_Network *net = &pr->network;

    if (index <= 0 || index > net->Ncontrols)
    {
        return set_error(pr->error_handle, 241);
    }
    for (i = index; i <= net->Ncontrols - 1; i++)
    {
        net->Control[i] = net->Control[i + 1];
    }
    net->Ncontrols--;
    return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getrule(EN_Project pr, int index, int *nPremises, int *nTrueActions, int *nFalseActions, EN_API_FLOAT_TYPE *priority)
/*----------------------------------------------------------------
**  Input:   index  = index of the rule
**           nPremises  = number of conditions (IF AND OR)
**           nTrueActions = number of actions with true conditions (THEN)
**           nFalseActions = number of actions with false conditions (ELSE)
**           priority = rule priority
**  Output:  none
**  Returns: error code
**----------------------------------------------------------------
*/
{
  int count;
  Premise *p;
  Action *c;

  EN_Network *net = &pr->network;

  if (index > net->Nrules)
    return set_error(pr->error_handle, 257);
  *priority = (EN_API_FLOAT_TYPE)pr->rules.Rule[index].priority;
  count = 1;
  p = pr->rules.Rule[index].Pchain;
  while (p->next != NULL) {
    count++;
    p = p->next;
  }
  *nPremises = count;
  count = 1;
  c = pr->rules.Rule[index].Tchain;
  while (c->next != NULL) {
    count++;
    c = c->next;
  }
  *nTrueActions = count;

  c = pr->rules.Rule[index].Fchain;
  count = 0;
  if (c != NULL) {
    count = 1;
    while (c->next != NULL) {
      count++;
      c = c->next;
    }
  }
  *nFalseActions = count;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getpremise(EN_Project pr, int indexRule, int idxPremise, int *logop,
                           int *object, int *indexObj, int *variable,
                           int *relop, int *status, EN_API_FLOAT_TYPE *value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (idxPremise > nPremises) {
    return set_error(pr->error_handle, 258);
  }

  p = pr->rules.Rule[indexRule].Pchain;
  while (count < idxPremise) {
    count++;
    p = p->next;
  }
  *logop = p->logop;
  *object = p->object;
  *indexObj = p->index;
  *variable = p->variable;
  *relop = p->relop;
  *status = p->status;
  *value = (EN_API_FLOAT_TYPE)p[0].value;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setrulepriority(EN_Project pr, int index, EN_API_FLOAT_TYPE priority)
/*----------------------------------------------------------------
**  Input:   index  = index of the rule
**           priority = rule priority
**  Output:  none
**  Returns: error code
**----------------------------------------------------------------
*/
{
  if (index > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  pr->rules.Rule[index].priority = priority;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpremise(EN_Project pr, int indexRule, int indexPremise, int logop,
                           int object, int indexObj, int variable, int relop,
                           int status, EN_API_FLOAT_TYPE value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return set_error(pr->error_handle, 258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->logop = logop;
  p->object = object;
  p->index = indexObj;
  p->variable = variable;
  p->relop = relop;
  p->status = status;
  p->value = value;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpremiseindex(EN_Project pr, int indexRule, int indexPremise, int indexObj) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules)
    return set_error(pr->error_handle, 257);
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return set_error(pr->error_handle, 258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->index = indexObj;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpremisestatus(EN_Project pr, int indexRule, int indexPremise, int status) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return set_error(pr->error_handle, 258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->status = status;
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setpremisevalue(EN_Project pr, int indexRule, int indexPremise,
                                EN_API_FLOAT_TYPE value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules)
    return set_error(pr->error_handle, 257);
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return set_error(pr->error_handle, 258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->value = value;
  
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_gettrueaction(EN_Project pr, int indexRule, int indexAction, int *indexLink,
                              int *status, EN_API_FLOAT_TYPE *setting) {
  int count = 1, error = 0, nTrueAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 252);
  }
  error = EN_getrule(pr, indexRule, &c, &nTrueAction, &b, &priority);
  if (indexAction > nTrueAction) {
    return set_error(pr->error_handle, 253);
  }
  a = pr->rules.Rule[indexRule].Tchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  *indexLink = a->link;
  *status = a->status;
  *setting = (EN_API_FLOAT_TYPE)a->setting;
  
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_settrueaction(EN_Project pr, int indexRule, int indexAction, int indexLink,
                              int status, EN_API_FLOAT_TYPE setting) {
  int count = 1, error = 0, nTrueAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &c, &nTrueAction, &b, &priority);
  if (indexAction > nTrueAction) {
    return set_error(pr->error_handle, 258);
  }
  a = pr->rules.Rule[indexRule].Tchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  a->link = indexLink;
  a->status = status;
  a->setting = setting;
  
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getfalseaction(EN_Project pr, int indexRule, int indexAction, int *indexLink,
                               int *status, EN_API_FLOAT_TYPE *setting) {
  int count = 1, error = 0, nFalseAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &c, &b, &nFalseAction, &priority);
  if (indexAction > nFalseAction) {
    return set_error(pr->error_handle, 258);
  }
  a = pr->rules.Rule[indexRule].Fchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  *indexLink = a->link;
  *status = a->status;
  *setting = (EN_API_FLOAT_TYPE)a->setting;
  
  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_setfalseaction(EN_Project pr, int indexRule, int indexAction, int indexLink,
                               int status, EN_API_FLOAT_TYPE setting) {
  int count = 1, error = 0, nFalseAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return set_error(pr->error_handle, 257);
  }
  error = EN_getrule(pr, indexRule, &c, &b, &nFalseAction, &priority);
  if (indexAction > nFalseAction) {
    return set_error(pr->error_handle, 258);
  }
  a = pr->rules.Rule[indexRule].Fchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  a->link = indexLink;
  a->status = status;
  a->setting = setting;

  return set_error(pr->error_handle, 0);
}

int DLLEXPORT EN_getruleID(EN_Project pr, int indexRule, char *id) {

  strcpy(id, "");
  if (!pr->Openflag)
    return set_error(pr->error_handle, 102);
  if (indexRule < 1 || indexRule > pr->network.Nrules)
    return set_error(pr->error_handle, 257);
  strcpy(id, pr->rules.Rule[indexRule].label);
  
  return set_error(pr->error_handle, 0);
}

/*************************** END OF EPANET.C ***************************/
