/*
*******************************************************************************

EPANET.C -- Hydraulic & Water Quality Simulator for Water Distribution Networks

VERSION:    2.00
DATE:       5/30/00
            9/7/00
            10/25/00
            3/1/01
            11/19/01
            6/24/02
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
 AUTHORS:     L. Rossman - US EPA - NRMRL
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
     resistance()
     tankvolume()
     getenergy()
     setlinkstatus()
     setlinksetting()
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
     ENHashTablecreate()
     ENHashTableFind()
     ENHashTableFree()

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
#include "epanet2.h"
#include "funcs.h"
#include "text.h"
#include "types.h"
#define EXTERN
#include "epanet2.h"
#include "vars.h"

/****************************************************************

 LEGACY (v <= 2.1) API: uses global project variable

*****************************************************************/

/*------------------------------------------------------------------------
 **   Input:   f1 = pointer to name of input file
 **            f2 = pointer to name of report file
 **            f3 = pointer to name of binary output file
 **            pviewprog = see note below
 **   Output:  none
 **  Returns: error code
 **  Purpose: runs a complete EPANET simulation
 **
 **  The pviewprog() argument is a pointer to a callback function
 **  that takes a character string (char *) as its only parameter.
 **  The function would reside in and be used by the calling
 **  program to display the progress messages that EPANET generates
 **  as it carries out its computations. If this feature is not
 **  needed then the argument should be NULL.
 **-------------------------------------------------------------------------
 */
int DLLEXPORT ENepanet(char *f1, char *f2, char *f3,
                       void (*pviewprog)(char *)) {
  int errcode = 0;
  ERRCODE(EN_alloc(&_defaultModel));
  ERRCODE(EN_open(_defaultModel, f1, f2, f3));
  _defaultModel->viewprog = pviewprog;
  if (_defaultModel->out_files.Hydflag != USE) {
    ERRCODE(EN_solveH(_defaultModel));
  }
  ERRCODE(EN_solveQ(_defaultModel));
  ERRCODE(EN_report(_defaultModel));
  EN_close(_defaultModel);
  EN_free(_defaultModel);
  return (errcode);
}
int DLLEXPORT ENopen(char *f1, char *f2, char *f3) {
  return EN_open(_defaultModel, f1, f2, f3);
}
int DLLEXPORT ENsaveinpfile(char *filename) {
  return EN_saveinpfile(_defaultModel, filename);
}
int DLLEXPORT ENclose() { return EN_close(_defaultModel); }
int DLLEXPORT ENsolveH() { return EN_solveH(_defaultModel); }
int DLLEXPORT ENsaveH() { return EN_saveH(_defaultModel); }
int DLLEXPORT ENopenH() { return EN_openH(_defaultModel); }
int DLLEXPORT ENinitH(int flag) { return EN_initH(_defaultModel, flag); }
int DLLEXPORT ENrunH(long *t) { return EN_runH(_defaultModel, t); }
int DLLEXPORT ENnextH(long *tstep) { return EN_nextH(_defaultModel, tstep); }
int DLLEXPORT ENcloseH() { return EN_closeH(_defaultModel); }
int DLLEXPORT ENsavehydfile(char *filename) {
  return EN_savehydfile(_defaultModel, filename);
}
int DLLEXPORT ENusehydfile(char *filename) {
  return EN_usehydfile(_defaultModel, filename);
}
int DLLEXPORT ENsolveQ() { return EN_solveQ(_defaultModel); }
int DLLEXPORT ENopenQ() { return EN_openQ(_defaultModel); }
int DLLEXPORT ENinitQ(int saveflag) {
  return EN_initQ(_defaultModel, saveflag);
}
int DLLEXPORT ENrunQ(long *t) { return EN_runQ(_defaultModel, t); }
int DLLEXPORT ENnextQ(long *tstep) { return EN_nextQ(_defaultModel, tstep); }
int DLLEXPORT ENstepQ(long *tleft) { return EN_stepQ(_defaultModel, tleft); }
int DLLEXPORT ENcloseQ() { return EN_closeQ(_defaultModel); }
int DLLEXPORT ENwriteline(char *line) {
  return EN_writeline(_defaultModel, line);
}
int DLLEXPORT ENreport() { return EN_report(_defaultModel); }
int DLLEXPORT ENresetreport() { return EN_resetreport(_defaultModel); }
int DLLEXPORT ENsetreport(char *s) { return EN_setreport(_defaultModel, s); }
int DLLEXPORT ENgetversion(int *v) { return EN_getversion(v); }
int DLLEXPORT ENgetcontrol(int cindex, int *ctype, int *lindex,
                           EN_API_FLOAT_TYPE *setting, int *nindex,
                           EN_API_FLOAT_TYPE *level) {
  return EN_getcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}
int DLLEXPORT ENgetcount(int code, int *count) {
  return EN_getcount(_defaultModel, (EN_CountType)code, count);
}
int DLLEXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getoption(_defaultModel, (EN_Option)code, value);
}
int DLLEXPORT ENgettimeparam(int code, long *value) {
  return EN_gettimeparam(_defaultModel, code, value);
}
int DLLEXPORT ENgetflowunits(int *code) {
  return EN_getflowunits(_defaultModel, code);
}
int DLLEXPORT ENgetpatternindex(char *id, int *index) {
  return EN_getpatternindex(_defaultModel, id, index);
}
int DLLEXPORT ENgetpatternid(int index, char *id) {
  return EN_getpatternid(_defaultModel, index, id);
}
int DLLEXPORT ENgetpatternlen(int index, int *len) {
  return EN_getpatternlen(_defaultModel, index, len);
}
int DLLEXPORT ENgetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE *value) {
  return EN_getpatternvalue(_defaultModel, index, period, value);
}
int DLLEXPORT ENgetcurveindex(char *id, int *index) {
  return EN_getcurveindex(_defaultModel, id, index);
}
int DLLEXPORT ENgetcurveid(int index, char *id) {
  return EN_getcurveid(_defaultModel, index, id);
}
int DLLEXPORT ENgetcurvelen(int index, int *len) {
  return EN_getcurvelen(_defaultModel, index, len);
}
int DLLEXPORT ENgetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE *x,
                              EN_API_FLOAT_TYPE *y) {
  return EN_getcurvevalue(_defaultModel, index, pnt, x, y);
}
int DLLEXPORT ENgetqualtype(int *qualcode, int *tracenode) {
  return EN_getqualtype(_defaultModel, qualcode, tracenode);
}
int DLLEXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
                            int *tracenode) {
  return EN_getqualinfo(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}
int DLLEXPORT ENgeterror(int errcode, char *errmsg, int n) {
  return EN_geterror(errcode, errmsg, n);
}
int DLLEXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getstatistic(_defaultModel, code, value);
}
int DLLEXPORT ENgetnodeindex(char *id, int *index) {
  return EN_getnodeindex(_defaultModel, id, index);
}
int DLLEXPORT ENgetnodeid(int index, char *id) {
  return EN_getnodeid(_defaultModel, index, id);
}
int DLLEXPORT ENgetnodetype(int index, int *code) {
  return EN_getnodetype(_defaultModel, index, code);
}
int DLLEXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x,
                         EN_API_FLOAT_TYPE *y) {
  return EN_getcoord(_defaultModel, index, x, y);
}
int DLLEXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y) {
  return EN_setcoord(_defaultModel, index, x, y);
}
int DLLEXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getnodevalue(_defaultModel, index, code, value);
}
int DLLEXPORT ENgetlinkindex(char *id, int *index) {
  return EN_getlinkindex(_defaultModel, id, index);
}
int DLLEXPORT ENgetlinkid(int index, char *id) {
  return EN_getlinkid(_defaultModel, index, id);
}
int DLLEXPORT ENgetlinktype(int index, EN_LinkType *code) {
  return EN_getlinktype(_defaultModel, index, code);
}
int DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2) {
  return EN_getlinknodes(_defaultModel, index, node1, node2);
}
int DLLEXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getlinkvalue(_defaultModel, index, (EN_LinkProperty)code, value);
}
int DLLEXPORT ENgetcurve(int curveIndex, char *id, int *nValues,
                         EN_API_FLOAT_TYPE **xValues,
                         EN_API_FLOAT_TYPE **yValues) {
  return EN_getcurve(_defaultModel, curveIndex, id, nValues, xValues, yValues);
}
int DLLEXPORT ENsetcontrol(int cindex, int ctype, int lindex,
                           EN_API_FLOAT_TYPE setting, int nindex,
                           EN_API_FLOAT_TYPE level) {
  return EN_setcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}
int DLLEXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setnodevalue(_defaultModel, index, code, v);
}
int DLLEXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setlinkvalue(_defaultModel, index, code, v);
}
int DLLEXPORT ENaddpattern(char *id) {
  return EN_addpattern(_defaultModel, id);
}
int DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int n) {
  return EN_setpattern(_defaultModel, index, f, n);
}
int DLLEXPORT ENsetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE value) {
  return EN_setpatternvalue(_defaultModel, index, period, value);
}
int DLLEXPORT ENaddcurve(char *id) { return EN_addcurve(_defaultModel, id); }
int DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y,
                         int n) {
  return EN_setcurve(_defaultModel, index, x, y, n);
}
int DLLEXPORT ENsetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE x,
                              EN_API_FLOAT_TYPE y) {
  return EN_setcurvevalue(_defaultModel, index, pnt, x, y);
}
int DLLEXPORT ENsettimeparam(int code, long value) {
  return EN_settimeparam(_defaultModel, code, value);
}
int DLLEXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v) {
  return EN_setoption(_defaultModel, code, v);
}
int DLLEXPORT ENsetstatusreport(int code) {
  return EN_setstatusreport(_defaultModel, code);
}
int DLLEXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
                            char *tracenode) {
  return EN_setqualtype(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}
int DLLEXPORT ENgetheadcurveindex(int index, int *curveindex) {
  return EN_getheadcurveindex(_defaultModel, index, curveindex);
}
int DLLEXPORT ENsetheadcurveindex(int index, int curveindex) {
  return EN_setheadcurveindex(_defaultModel, index, curveindex);
}
int DLLEXPORT ENgetpumptype(int index, int *type) {
  return EN_getpumptype(_defaultModel, index, type);
}
int DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands) {
  return EN_getnumdemands(_defaultModel, nodeIndex, numDemands);
}
int DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE *baseDemand) {
  return EN_getbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}
int DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE baseDemand) {
  return EN_setbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}
int DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIdx, int *pattIdx) {
  return EN_getdemandpattern(_defaultModel, nodeIndex, demandIdx, pattIdx);
}
int DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value) {
  return EN_getaveragepatternvalue(_defaultModel, index, value);
}
int DLLEXPORT ENsetlinktype(char *id, EN_LinkType toType) {
  return EN_setlinktype(_defaultModel, id, toType);
}
int DLLEXPORT ENaddnode(char *id, EN_NodeType nodeType) {
  return EN_addnode(_defaultModel, id, nodeType);
}
int DLLEXPORT ENaddlink(char *id, EN_LinkType linkType, char *fromNode,
                        char *toNode) {
  return EN_addlink(_defaultModel, id, linkType, fromNode, toNode);
}
int DLLEXPORT ENdeletelink(int index) {
  return EN_deletelink(_defaultModel, index);
}
int DLLEXPORT ENdeletenode(int index) {
  return EN_deletenode(_defaultModel, index);
}
int DLLEXPORT ENgetrule(int index, int *nPremises, int *nTrueActions,
                        int *nFalseActions, EN_API_FLOAT_TYPE *priority) {
  return EN_getrule(_defaultModel, index, nPremises, nTrueActions,
                    nFalseActions, priority);
}
int DLLEXPORT ENgetpremise(int indexRule, int idxPremise, int *logop,
                           int *object, int *indexObj, int *variable,
                           int *relop, int *status, EN_API_FLOAT_TYPE *value) {
  return EN_getpremise(_defaultModel, indexRule, idxPremise, logop, object,
                       indexObj, variable, relop, status, value);
}
int DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority) {
  return EN_setrulepriority(_defaultModel, index, priority);
}
int DLLEXPORT ENsetpremise(int indexRule, int indexPremise, int logop,
                           int object, int indexObj, int variable, int relop,
                           int status, EN_API_FLOAT_TYPE value) {
  return EN_setpremise(_defaultModel, indexRule, indexPremise, logop, object,
                       indexObj, variable, relop, status, value);
}
int DLLEXPORT ENsetpremiseindex(int indexRule, int indexPremise, int indexObj) {
  return EN_setpremiseindex(_defaultModel, indexRule, indexPremise, indexObj);
}
int DLLEXPORT ENsetpremisestatus(int indexRule, int indexPremise, int status) {
  return EN_setpremisestatus(_defaultModel, indexRule, indexPremise, status);
}
int DLLEXPORT ENsetpremisevalue(int indexRule, int indexPremise,
                                EN_API_FLOAT_TYPE value) {
  return EN_setpremisevalue(_defaultModel, indexRule, indexPremise, value);
}
int DLLEXPORT ENgettrueaction(int indexRule, int indexAction, int *indexLink,
                              int *status, EN_API_FLOAT_TYPE *setting) {
  return EN_gettrueaction(_defaultModel, indexRule, indexAction, indexLink,
                          status, setting);
}
int DLLEXPORT ENsettrueaction(int indexRule, int indexAction, int indexLink,
                              int status, EN_API_FLOAT_TYPE setting) {
  return EN_settrueaction(_defaultModel, indexRule, indexAction, indexLink,
                          status, setting);
}
int DLLEXPORT ENgetfalseaction(int indexRule, int indexAction, int *indexLink,
                               int *status, EN_API_FLOAT_TYPE *setting) {
  return EN_getfalseaction(_defaultModel, indexRule, indexAction, indexLink,
                           status, setting);
}
int DLLEXPORT ENsetfalseaction(int indexRule, int indexAction, int indexLink,
                               int status, EN_API_FLOAT_TYPE setting) {
  return EN_setfalseaction(_defaultModel, indexRule, indexAction, indexLink,
                           status, setting);
}
int DLLEXPORT ENgetruleID(int indexRule, char *id) {
  return EN_getruleID(_defaultModel, indexRule, id);
}

/*
----------------------------------------------------------------
   Functions for opening & closing the EPANET system
----------------------------------------------------------------
*/

/// allocate a project pointer
int DLLEXPORT EN_alloc(EN_Project **p)
{
  EN_Project *project = calloc(1, sizeof(EN_Project));
  *p = project;
  return 0;
}

int DLLEXPORT EN_free(EN_Project *p) 
{
  free(p);
  return 0;
}

int DLLEXPORT EN_init(EN_Project *pr, char *f2, char *f3,
                      EN_FlowUnits UnitsType, EN_FormType HeadlossFormula)
/*----------------------------------------------------------------
 **  Input:
 **           f2               = pointer to name of report file
 **           f3               = pointer to name of binary output file
 UnitsType        = flow units flag
 HeadlossFormula  = headloss formula flag

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

  pr->parser.MaxPats = 1;
  // initialize default pattern
  getpatterns(pr);

  return (errcode);
}

int DLLEXPORT EN_open(EN_Project *p, char *f1, char *f2, char *f3)
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
  p->Openflag = FALSE;
  p->hydraulics.OpenHflag = FALSE;
  p->quality.OpenQflag = FALSE;
  p->save_options.SaveHflag = FALSE;
  p->save_options.SaveQflag = FALSE;
  p->Warnflag = FALSE;
  p->parser.Coordflag = TRUE;

  /*** Updated 9/7/00 ***/
  p->report.Messageflag = TRUE;
  p->report.Rptflag = 1;

  /* Initialize global pointers to NULL. */
  initpointers(p);

  /* Open input & report files */
  ERRCODE(openfiles(p, f1, f2, f3));
  if (errcode > 0) {
    errmsg(p, errcode);
    return (errcode);
  }
  writelogo(p);

  /* Find network size & allocate memory for data */
  writecon(FMT02);
  writewin(p->viewprog, FMT100);
  ERRCODE(netsize(p));
  ERRCODE(allocdata(p));

  /* Retrieve input data */
  ERRCODE(getdata(p));

  /* Free temporary linked lists used for Patterns & Curves */
  freeTmplist(p->parser.Patlist);
  freeTmplist(p->parser.Curvelist);

  /* If using previously saved hydraulics then open its file */
  if (p->out_files.Hydflag == USE) {
    ERRCODE(openhydfile(p));
  }

  /* Write input summary to report file */
  if (!errcode) {
    if (p->report.Summaryflag) {
      writesummary(p);
    }
    writetime(p, FMT104);
    p->Openflag = TRUE;
  } else
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_saveinpfile(EN_Project *p, char *filename)
/*----------------------------------------------------------------
 **  Input:   filename = name of INP file
 **  Output:  none
 **  Returns: error code
 **  Purpose: saves current data base to file
 **----------------------------------------------------------------
 */
{
  if (!p->Openflag)
    return (102);
  return (saveinpfile(p, filename));
}

int DLLEXPORT EN_close(EN_Project *p)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: frees all memory & files used by EPANET
 **----------------------------------------------------------------
 */
{
  if (p->Openflag) {
    writetime(p, FMT105);
  }
  freedata(p);

  out_file_t *out = &p->out_files;

  if (out->TmpOutFile != out->OutFile) {
    if (out->TmpOutFile != NULL) {
      fclose(out->TmpOutFile);
    }
    remove(out->TmpFname);
  }
  out->TmpOutFile = NULL;

  if (p->parser.InFile != NULL) {
    fclose(p->parser.InFile);
    p->parser.InFile = NULL;
  }
  if (p->report.RptFile != NULL && p->report.RptFile != stdout) {
    fclose(p->report.RptFile);
    p->report.RptFile = NULL;
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

  p->Openflag = FALSE;
  p->hydraulics.OpenHflag = FALSE;
  p->save_options.SaveHflag = FALSE;
  p->quality.OpenQflag = FALSE;
  p->save_options.SaveQflag = FALSE;
  return (0);
}

/*
 ----------------------------------------------------------------
 Functions for running a hydraulic analysis
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_solveH(EN_Project *p)
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
  errcode = EN_openH(p);
  if (!errcode) {
    /* Initialize hydraulics */
    errcode = EN_initH(p, EN_SAVE);
    writecon(FMT14);

    /* Analyze each hydraulic period */
    if (!errcode)
      do {

        /* Display progress message */

        /*** Updated 6/24/02 ***/
        sprintf(p->Msg, "%-10s",
                clocktime(p->report.Atime, p->time_options.Htime));

        writecon(p->Msg);
        sprintf(p->Msg, FMT101, p->report.Atime);
        writewin(p->viewprog, p->Msg);

        /* Solve for hydraulics & advance to next time period */
        tstep = 0;
        ERRCODE(EN_runH(p, &t));
        ERRCODE(EN_nextH(p, &tstep));

        /*** Updated 6/24/02 ***/
        writecon("\b\b\b\b\b\b\b\b\b\b");
      } while (tstep > 0);
  }

  /* Close hydraulics solver */

  /*** Updated 6/24/02 ***/
  writecon("\b\b\b\b\b\b\b\b                     ");

  EN_closeH(p);
  errcode = MAX(errcode, p->Warnflag);
  return (errcode);
}

int DLLEXPORT EN_saveH(EN_Project *p)
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
  if (!p->save_options.SaveHflag)
    return (104);

  /* Temporarily turn off WQ analysis */
  tmpflag = p->quality.Qualflag;
  p->quality.Qualflag = NONE;

  /* Call WQ solver to simply transfer results */
  /* from Hydraulics file to Output file at    */
  /* fixed length reporting time intervals.    */
  errcode = EN_solveQ(p);

  /* Restore WQ analysis option */
  p->quality.Qualflag = tmpflag;
  if (errcode) {
    errmsg(p, errcode);
  }
  return (errcode);
}

int DLLEXPORT EN_openH(EN_Project *p)
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
  p->hydraulics.OpenHflag = FALSE;
  p->save_options.SaveHflag = FALSE;
  if (!p->Openflag) {
    return (102);
  }

  /* Check that previously saved hydraulics file not in use */
  if (p->out_files.Hydflag == USE) {
    return (107);
  }

  /* Open hydraulics solver */
  ERRCODE(openhyd(p));
  if (!errcode)
    p->hydraulics.OpenHflag = TRUE;
  else
    errmsg(p, errcode);
  return (errcode);
}

/*** Updated 3/1/01 ***/
int DLLEXPORT EN_initH(EN_Project *p, int flag)
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
  p->save_options.SaveHflag = FALSE;
  p->Warnflag = FALSE;

  /* Get values of save-to-file flag and reinitialize-flows flag */
  fflag = flag / EN_INITFLOW;
  sflag = flag - fflag * EN_INITFLOW;

  /* Check that hydraulics solver was opened */
  if (!p->hydraulics.OpenHflag)
    return (103);

  /* Open hydraulics file */
  p->save_options.Saveflag = FALSE;
  if (sflag > 0) {
    errcode = openhydfile(p);
    if (!errcode)
      p->save_options.Saveflag = TRUE;
    else
      errmsg(p, errcode);
  }

  /* Initialize hydraulics */
  inithyd(p, fflag);
  if (p->report.Statflag > 0)
    writeheader(p, STATHDR, 0);
  return (errcode);
}

int DLLEXPORT EN_runH(EN_Project *p, long *t) {
  int errcode;
  *t = 0;
  if (!p->hydraulics.OpenHflag)
    return (103);
  errcode = runhyd(p, t);
  if (errcode)
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_nextH(EN_Project *p, long *tstep) {
  int errcode;
  *tstep = 0;
  if (!p->hydraulics.OpenHflag)
    return (103);
  errcode = nexthyd(p, tstep);
  if (errcode)
    errmsg(p, errcode);
  else if (p->save_options.Saveflag && *tstep == 0)
    p->save_options.SaveHflag = TRUE;
  return (errcode);
}

int DLLEXPORT EN_closeH(EN_Project *p)

{
  if (!p->Openflag) {
    return (102);
  }
  if (p->hydraulics.OpenHflag) {
    closehyd(p);
  }
  p->hydraulics.OpenHflag = FALSE;
  return (0);
}

int DLLEXPORT EN_savehydfile(EN_Project *p, char *filename) {
  FILE *f;
  int c;

  /* Check that hydraulics results exist */
  if (p->out_files.HydFile == NULL || !p->save_options.SaveHflag)
    return (104);

  /* Open file */
  if ((f = fopen(filename, "w+b")) == NULL)
    return (305);

  /* Copy from HydFile to f */
  FILE *HydFile = p->out_files.HydFile;
  fseek(HydFile, 0, SEEK_SET);
  while ((c = fgetc(HydFile)) != EOF) {
    fputc(c, f);
  }
  fclose(f);
  return (0);
}

int DLLEXPORT EN_usehydfile(EN_Project *p, char *filename) {
  int errcode;

  /* Check that input data exists & hydraulics system closed */
  if (!p->Openflag)
    return (102);
  if (p->hydraulics.OpenHflag)
    return (108);

  /* Try to open hydraulics file */
  strncpy(p->out_files.HydFname, filename, MAXFNAME);
  p->out_files.Hydflag = USE;
  p->save_options.SaveHflag = TRUE;
  errcode = openhydfile(p);

  /* If error, then reset flags */
  if (errcode) {
    strcpy(p->out_files.HydFname, "");
    p->out_files.Hydflag = SCRATCH;
    p->save_options.SaveHflag = FALSE;
  }
  return (errcode);
}

/*
 ----------------------------------------------------------------
 Functions for running a WQ analysis
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_solveQ(EN_Project *p) {
  int errcode;
  long t, tstep;

  /* Open WQ solver */
  errcode = EN_openQ(p);
  if (!errcode) {
    /* Initialize WQ */
    errcode = EN_initQ(p, EN_SAVE);
    if (p->quality.Qualflag)
      writecon(FMT15);
    else {
      writecon(FMT16);
      writewin(p->viewprog, FMT103);
    }

    /* Analyze each hydraulic period */
    if (!errcode)
      do {

        /* Display progress message */

        /*** Updated 6/24/02 ***/
        sprintf(p->Msg, "%-10s",
                clocktime(p->report.Atime, p->time_options.Htime));

        writecon(p->Msg);
        if (p->quality.Qualflag) {
          sprintf(p->Msg, FMT102, p->report.Atime);
          writewin(p->viewprog, p->Msg);
        }

        /* Retrieve current network solution & update WQ to next time period */
        tstep = 0;
        ERRCODE(EN_runQ(p, &t));
        ERRCODE(EN_nextQ(p, &tstep));

        /*** Updated 6/24/02 ***/
        writecon("\b\b\b\b\b\b\b\b\b\b");

      } while (tstep > 0);
  }

  /* Close WQ solver */

  /*** Updated 6/24/02 ***/
  writecon("\b\b\b\b\b\b\b\b                     ");
  EN_closeQ(p);
  return (errcode);
}

int DLLEXPORT EN_openQ(EN_Project *p) {
  int errcode = 0;

  /* Check that hydraulics results exist */
  p->quality.OpenQflag = FALSE;
  p->save_options.SaveQflag = FALSE;
  if (!p->Openflag)
    return (102);
  // !LT! todo - check for p->save_options.SaveHflag / set sequential/step mode
  // if (!p->save_options.SaveHflag) return(104);

  /* Open WQ solver */
  ERRCODE(openqual(p));
  if (!errcode)
    p->quality.OpenQflag = TRUE;
  else
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_initQ(EN_Project *p, int saveflag) {
  int errcode = 0;
  if (!p->quality.OpenQflag)
    return (105);
  initqual(p);
  p->save_options.SaveQflag = FALSE;
  p->save_options.Saveflag = FALSE;
  if (saveflag) {
    errcode = openoutfile(p);
    if (!errcode)
      p->save_options.Saveflag = TRUE;
  }
  return (errcode);
}

int DLLEXPORT EN_runQ(EN_Project *p, long *t) {
  int errcode;
  *t = 0;
  if (!p->quality.OpenQflag)
    return (105);
  errcode = runqual(p, t);
  if (errcode)
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_nextQ(EN_Project *p, long *tstep) {
  int errcode;
  *tstep = 0;
  if (!p->quality.OpenQflag)
    return (105);
  errcode = nextqual(p, tstep);
  if (!errcode && p->save_options.Saveflag && *tstep == 0) {
    p->save_options.SaveQflag = TRUE;
  }
  if (errcode)
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_stepQ(EN_Project *p, long *tleft) {
  int errcode;
  *tleft = 0;
  if (!p->quality.OpenQflag)
    return (105);
  errcode = stepqual(p, tleft);
  if (!errcode && p->save_options.Saveflag && *tleft == 0) {
    p->save_options.SaveQflag = TRUE;
  }
  if (errcode)
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_closeQ(EN_Project *p)

{
  if (!p->Openflag)
    return (102);
  closequal(p);
  p->quality.OpenQflag = FALSE;
  return (0);
}

/*
 ----------------------------------------------------------------
 Functions for generating an output report
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_writeline(EN_Project *p, char *line) {
  if (!p->Openflag)
    return (102);
  writeline(p, line);
  return (0);
}

int DLLEXPORT EN_report(EN_Project *p) {
  int errcode;

  /* Check if results saved to binary output file */
  if (!p->save_options.SaveQflag)
    return (106);
  errcode = writereport(p);
  if (errcode)
    errmsg(p, errcode);
  return (errcode);
}

int DLLEXPORT EN_resetreport(EN_Project *p) {
  int i;
  if (!p->Openflag)
    return (102);
  initreport(&p->report);
  for (i = 1; i <= p->network.Nnodes; i++)
    p->network.Node[i].Rpt = 0;
  for (i = 1; i <= p->network.Nlinks; i++)
    p->network.Link[i].Rpt = 0;
  return (0);
}

int DLLEXPORT EN_setreport(EN_Project *p, char *s) {
  char s1[MAXLINE + 1];
  if (!p->Openflag)
    return (102);
  if (strlen(s) > MAXLINE)
    return (250);
  strcpy(s1, s);
  if (setreport(p, s1) > 0)
    return (250);
  else
    return (0);
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
  return (0);
}

int DLLEXPORT EN_getcontrol(EN_Project *pr, int cindex, int *ctype, int *lindex,
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
    return (102);
  if (cindex < 1 || cindex > net->Ncontrols)
    return (241);
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
  return (0);
}

int DLLEXPORT EN_getcount(EN_Project *pr, EN_CountType code, int *count) {
  EN_Network *net = &pr->network;

  *count = 0;
  if (!pr->Openflag)
    return (102);
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
    return (251);
  }
  return (0);
}

int DLLEXPORT EN_getoption(EN_Project *pr, EN_Option code,
                           EN_API_FLOAT_TYPE *value) {
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  double *Ucf = pr->Ucf;

  double v = 0.0;
  *value = 0.0;
  if (!pr->Openflag)
    return (102);
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
  default:
    return (251);
  }
  *value = (EN_API_FLOAT_TYPE)v;
  return (0);
}

int DLLEXPORT EN_gettimeparam(EN_Project *pr, int code, long *value) {

  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;


  *value = 0;
  if (!pr->Openflag)
    return (102);
  if (code < EN_DURATION || code > EN_NEXTEVENTIDX)
    return (251);
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
      int i = tanktimestep(pr, value);
      *value = i;
      break;
  }
  return (0);
}

int DLLEXPORT EN_getflowunits(EN_Project *p, int *code) {
  *code = -1;
  if (!p->Openflag)
    return (102);
  *code = p->parser.Flowflag;
  return (0);
}

int DLLEXPORT EN_getpatternindex(EN_Project *p, char *id, int *index) {
  int i;
  *index = 0;
  if (!p->Openflag)
    return (102);
  for (i = 1; i <= p->network.Npats; i++) {
    if (strcmp(id, p->network.Pattern[i].ID) == 0) {
      *index = i;
      return (0);
    }
  }
  *index = 0;
  return (205);
}

int DLLEXPORT EN_getpatternid(EN_Project *p, int index, char *id) {
  strcpy(id, "");
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Npats)
    return (205);
  strcpy(id, p->network.Pattern[index].ID);
  return (0);
}

int DLLEXPORT EN_getpatternlen(EN_Project *p, int index, int *len) {
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Npats)
    return (205);
  *len = p->network.Pattern[index].Length;
  return (0);
}

int DLLEXPORT EN_getpatternvalue(EN_Project *p, int index, int period,
                                 EN_API_FLOAT_TYPE *value) {
  *value = 0.0;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Npats)
    return (205);
  if (period < 1 || period > p->network.Pattern[index].Length)
    return (251);
  *value = (EN_API_FLOAT_TYPE)p->network.Pattern[index].F[period - 1];
  return (0);
}

int DLLEXPORT EN_getcurveindex(EN_Project *p, char *id, int *index) {
  int i;
  *index = 0;
  if (!p->Openflag)
    return (102);
  for (i = 1; i <= p->network.Ncurves; i++) {
    if (strcmp(id, p->network.Curve[i].ID) == 0) {
      *index = i;
      return (0);
    }
  }
  *index = 0;
  return (206);
}

int DLLEXPORT EN_getcurveid(EN_Project *p, int index, char *id) {
  strcpy(id, "");
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Ncurves)
    return (206);
  strcpy(id, p->network.Curve[index].ID);
  return (0);
}

int DLLEXPORT EN_getcurvelen(EN_Project *p, int index, int *len) {
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Ncurves)
    return (206);
  *len = p->network.Curve[index].Npts;
  return (0);
}

int DLLEXPORT EN_getcurvevalue(EN_Project *p, int index, int pnt,
                               EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y) {
  *x = 0.0;
  *y = 0.0;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Ncurves)
    return (206);
  if (pnt < 1 || pnt > p->network.Curve[index].Npts)
    return (251);
  *x = (EN_API_FLOAT_TYPE)p->network.Curve[index].X[pnt - 1];
  *y = (EN_API_FLOAT_TYPE)p->network.Curve[index].Y[pnt - 1];
  return (0);
}

int DLLEXPORT EN_getqualtype(EN_Project *p, int *qualcode, int *tracenode) {
  *tracenode = 0;
  if (!p->Openflag)
    return (102);
  *qualcode = p->quality.Qualflag;
  if (p->quality.Qualflag == TRACE)
    *tracenode = p->quality.TraceNode;
  return (0);
}

int DLLEXPORT EN_getqualinfo(EN_Project *p, int *qualcode, char *chemname, char *chemunits, int *tracenode) {
  ENgetqualtype(qualcode, tracenode);
  if (p->quality.Qualflag == TRACE) {
    strncpy(chemname, "", MAXID);
    strncpy(chemunits, "dimensionless", MAXID);
  } else {
    strncpy(chemname, p->quality.ChemName, MAXID);
    strncpy(chemunits, p->quality.ChemUnits, MAXID);
  }
  return 0;
}

int DLLEXPORT EN_geterror(int errcode, char *errmsg, int n) {
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
    geterrmsg(n, newMsg);
    strncpy(errmsg, newMsg, n);
  }
  if (strlen(errmsg) == 0)
    return (251);
  else
    return (0);
}

int DLLEXPORT EN_getstatistic(EN_Project *p, int code, EN_API_FLOAT_TYPE *value) {
  switch (code) {
  case EN_ITERATIONS:
    *value = (EN_API_FLOAT_TYPE)p->hydraulics.iterations;
    break;
  case EN_RELATIVEERROR:
    *value = (EN_API_FLOAT_TYPE)p->hydraulics.relativeError;
    break;
  default:
    break;
  }
  return 0;
}

/*
 ----------------------------------------------------------------
 Functions for retrieving node data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_getnodeindex(EN_Project *p, char *id, int *index) {
  *index = 0;
  if (!p->Openflag)
    return (102);
  *index = findnode(&p->network,id);
  if (*index == 0)
    return (203);
  else
    return (0);
}

int DLLEXPORT EN_getnodeid(EN_Project *p, int index, char *id) {
  strcpy(id, "");
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nnodes)
    return (203);
  strcpy(id, p->network.Node[index].ID);
  return (0);
}

int DLLEXPORT EN_getnodetype(EN_Project *p, int index, int *code) {
  *code = -1;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nnodes)
    return (203);
  if (index <= p->network.Njuncs)
    *code = EN_JUNCTION;
  else {
    if (p->network.Tank[index - p->network.Njuncs].A == 0.0)
      *code = EN_RESERVOIR;
    else
      *code = EN_TANK;
  }
  return (0);
}

int DLLEXPORT EN_getcoord(EN_Project *p, int index, EN_API_FLOAT_TYPE *x,
                          EN_API_FLOAT_TYPE *y) {
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nnodes)
    return (203);
  if (!p->parser.Coordflag)
    return (255);

  // check if node have coords
  if (p->network.Coord[index].HaveCoords == FALSE)
    return (254);

  *x = p->network.Coord[index].X;
  *y = p->network.Coord[index].Y;
  return 0;
}

int DLLEXPORT EN_setcoord(EN_Project *p, int index, EN_API_FLOAT_TYPE x,
                          EN_API_FLOAT_TYPE y) {
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nnodes)
    return (203);
  if (!p->parser.Coordflag)
    return (255);

  p->network.Coord[index].X = x;
  p->network.Coord[index].Y = y;
  p->network.Coord[index].HaveCoords = TRUE;
  return 0;
}

int DLLEXPORT EN_getnodevalue(EN_Project *p, int index, int code,
                              EN_API_FLOAT_TYPE *value) {
  double v = 0.0;
  Pdemand demand;
  Psource source;

  
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  
  Snode *Node = net->Node;
  Stank *Tank = net->Tank;
  
  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  
  double *Ucf = p->Ucf;
  double *NodeDemand = hyd->NodeDemand;
  double *NodeQual = qu->NodeQual;
  
  
  /* Check for valid arguments */
  *value = 0.0;
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Nnodes)
    return (203);

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
      return (240);
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
      return (251);
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
      return (251);
    v = tankvolume(p, index - Njuncs, hyd->NodeHead[index]) * Ucf[VOLUME];
    break;

  default:
    return (251);
  }
  *value = (EN_API_FLOAT_TYPE)v;
  return (0);
}

/*
 ----------------------------------------------------------------
 Functions for retrieving link data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_getlinkindex(EN_Project *p, char *id, int *index) {
  *index = 0;
  if (!p->Openflag)
    return (102);
  *index = findlink(&p->network,id);
  if (*index == 0)
    return (204);
  else
    return (0);
}

int DLLEXPORT EN_getlinkid(EN_Project *p, int index, char *id) {
  strcpy(id, "");
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nlinks)
    return (204);
  strcpy(id, p->network.Link[index].ID);
  return (0);
}

int DLLEXPORT EN_getlinktype(EN_Project *p, int index, EN_LinkType *code) {
  *code = -1;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nlinks)
    return (204);
  *code = p->network.Link[index].Type;
  return (0);
}

int DLLEXPORT EN_getlinknodes(EN_Project *p, int index, int *node1,
                              int *node2) {
  *node1 = 0;
  *node2 = 0;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > p->network.Nlinks)
    return (204);
  *node1 = p->network.Link[index].N1;
  *node2 = p->network.Link[index].N2;
  return (0);
}

int DLLEXPORT EN_getlinkvalue(EN_Project *p, int index, EN_LinkProperty code, EN_API_FLOAT_TYPE *value) {
  double a, h, q, v = 0.0;
  int returnValue = 0;
  
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  
  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  
  const int Nlinks = net->Nlinks;
  
  double *Ucf = p->Ucf;
  double *LinkFlows = hyd->LinkFlows;
  double *LinkSetting = hyd->LinkSetting;

  
  
  /* Check for valid arguments */
  *value = 0.0;
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Nlinks)
    return (204);

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
        return (ENgetlinkvalue(index, EN_ROUGHNESS, value));
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
      
    case EN_SETTING:
      if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE) {
        return (ENgetlinkvalue(index, EN_ROUGHNESS, value));
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
      getenergy(p, index, &v, &a);
      break;
      
    case EN_LINKQUAL:
      v = avgqual(p, index) * Ucf[LINKQUAL];
      break;
      
    case EN_LINKPATTERN:
      if (Link[index].Type == EN_PUMP)
        v = (double)Pump[findpump(&p->network, index)].Upat;
      break;
      
    case EN_EFFICIENCY:
      getenergy(p, index, &a, &v);
      break;
      
    case EN_HEADCURVE:
      if (Link[index].Type == EN_PUMP) {
        v = (double)Pump[findpump(&p->network, index)].Hcurve;
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
        v = (double)Pump[findpump(&p->network, index)].Ecurve;
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
  return returnValue;
}

int DLLEXPORT EN_getcurve(EN_Project *p, int curveIndex, char *id, int *nValues,
                          EN_API_FLOAT_TYPE **xValues,
                          EN_API_FLOAT_TYPE **yValues) {
  int iPoint, nPoints;
  Scurve curve;
  EN_API_FLOAT_TYPE *pointX, *pointY;

  /* Check that input file opened */
  if (!p->Openflag)
    return (102);

  curve = p->network.Curve[curveIndex];
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

  return (0);
}

/*
 ----------------------------------------------------------------
 Functions for changing network data
 ----------------------------------------------------------------
 */

int DLLEXPORT EN_setcontrol(EN_Project *p, int cindex, int ctype, int lindex,
                            EN_API_FLOAT_TYPE setting, int nindex,
                            EN_API_FLOAT_TYPE level) {
  char status = ACTIVE;
  long t = 0;
  double s = setting, lvl = level;

  /* Check that input file opened */
  if (!p->Openflag)
    return (102);

  /* Check that control exists */
  if (cindex < 1 || cindex > p->network.Ncontrols)
    return (241);

  
  EN_Network *net = &p->network;
  
  Snode *Node = net->Node;
  Slink *Link = net->Link;
  Scontrol *Control = net->Control;
  
  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  const int Nlinks = net->Nlinks;
  
  double *Ucf = p->Ucf;
  
  
  
  /* Check that controlled link exists */
  if (lindex == 0) {
    Control[cindex].Link = 0;
    return (0);
  }
  if (lindex < 0 || lindex > Nlinks)
    return (204);

  /* Cannot control check valve. */
  if (Link[lindex].Type == EN_CVPIPE)
    return (207);

  /* Check for valid parameters */
  if (ctype < 0 || ctype > EN_TIMEOFDAY)
    return (251);
  if (ctype == EN_LOWLEVEL || ctype == EN_HILEVEL) {
    if (nindex < 1 || nindex > Nnodes)
      return (203);
  } else
    nindex = 0;
  if (s < 0.0 || lvl < 0.0)
    return (202);

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
        return (202);
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
  return (0);
}

int DLLEXPORT EN_setnodevalue(EN_Project *p, int index, int code,
                              EN_API_FLOAT_TYPE v)
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
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  
  Snode *Node = net->Node;
  Stank *Tank = net->Tank;
  
  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  const int Npats = net->Npats;
  
  double *Ucf = p->Ucf;
  
  int j;
  Pdemand demand;
  Psource source;
  double Htmp;
  double value = v;

  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Nnodes)
    return (203);
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
      return (205);
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
      return (203);
    if (value < 0.0)
      return (202);
    if (value > 0.0)
      value = pow((Ucf[FLOW] / value), hyd->Qexp) / Ucf[PRESSURE];
    Node[index].Ke = value;
    break;

  case EN_INITQUAL:
    if (value < 0.0)
      return (202);
    Node[index].C0 = value / Ucf[QUALITY];
    if (index > Njuncs)
      Tank[index - Njuncs].C = Node[index].C0;
    break;

  case EN_SOURCEQUAL:
  case EN_SOURCETYPE:
  case EN_SOURCEPAT:
    if (value < 0.0)
      return (202);
    source = Node[index].S;
    if (source == NULL) {
      source = (struct Ssource *)malloc(sizeof(struct Ssource));
      if (source == NULL)
        return (101);
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
        return (205);
      source->Pat = j;
    } else // code == EN_SOURCETYPE
    {
      j = ROUND(value);
      if (j < CONCEN || j > FLOWPACED)
        return (251);
      else
        source->Type = (char)j;
    }
    return (0);

  case EN_TANKLEVEL:
    if (index <= Njuncs)
      return (251);
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
        return (202);
      Tank[j].H0 = value;
      Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
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
      return (202);
    if (index <= Njuncs)
      return (251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      value /= Ucf[ELEV];
      Tank[j].A = PI * SQR(value) / 4.0;
      Tank[j].Vmin = tankvolume(p, j, Tank[j].Hmin);
      Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
      Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
    } else {
      return (251);
    }
    break;

  case EN_MINVOLUME:
    if (value < 0.0)
      return (202);
    if (index <= Njuncs)
      return (251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].Vmin = value / Ucf[VOLUME];
      Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
      Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
    } else {
      return (251);
    }
    break;

  case EN_MINLEVEL:
    if (value < 0.0)
      return (202);
    if (index <= Njuncs)
      return (251); // not a tank or reservoir
    j = index - Njuncs;
    if (Tank[j].A == 0.0)
      return (251); // node is a reservoir
    Htmp = value / Ucf[ELEV] + Node[index].El;
    if (Htmp < Tank[j].Hmax && Htmp <= Tank[j].H0) {
      if (Tank[j].Vcurve > 0)
        return (202);
      Tank[j].Hmin = Htmp;
      Tank[j].Vmin = tankvolume(p, j, Tank[j].Hmin);
    } else {
      return (251);
    }
    break;

  case EN_MAXLEVEL:
    if (value < 0.0)
      return (202);
    if (index <= Njuncs)
      return (251); // not a tank or reservoir
    j = index - Njuncs;
    if (Tank[j].A == 0.0)
      return (251); // node is a reservoir
    Htmp = value / Ucf[ELEV] + Node[index].El;
    if (Htmp > Tank[j].Hmin && Htmp >= Tank[j].H0) {
      if (Tank[j].Vcurve > 0)
        return (202);
      Tank[j].Hmax = Htmp;
      Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
    } else {
      return (251);
    }
    break;

  case EN_MIXMODEL:
    j = ROUND(value);
    if (index <= Njuncs)
      return (251);
    if (j < MIX1 || j > LIFO)
      return (202);
    if (index > Njuncs && Tank[index - Njuncs].A > 0.0) {
      Tank[index - Njuncs].MixModel = (char)j;
    } else {
      return (251);
    }
    break;

  case EN_MIXFRACTION:
    if (value < 0.0 || value > 1.0)
      return (202);
    if (index <= Njuncs)
      return (251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].V1max = value * Tank[j].Vmax;
    }
    break;

  case EN_TANK_KBULK:
    if (index <= Njuncs)
      return (251);
    j = index - Njuncs;
    if (j > 0 && Tank[j].A > 0.0) {
      Tank[j].Kb = value / SECperDAY;
      qu->Reactflag = 1;
    } else {
      return (251);
    }
    break;

    /***  New parameter additions ends here. ***/ 

  default:
    return (251);
  }
  return (0);
}

int DLLEXPORT EN_setlinkvalue(EN_Project *p, int index, int code,
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
  
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  
  Slink *Link = net->Link;
  
  const int Nlinks = net->Nlinks;
  
  double *Ucf = p->Ucf;
  double *LinkSetting = hyd->LinkSetting;
  
  char s;
  double r, value = v;

  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Nlinks)
    return (204);
  switch (code) {
  case EN_DIAMETER:
    if (Link[index].Type != EN_PUMP) {
      if (value <= 0.0)
        return (202);
      value /= Ucf[DIAM];                /* Convert to feet */
      r = Link[index].Diam / value;      /* Ratio of old to new diam */
      Link[index].Km *= SQR(r) * SQR(r); /* Adjust minor loss factor */
      Link[index].Diam = value;          /* Update diameter */
      resistance(p, index);                 /* Update resistance factor */
    }
    break;

  case EN_LENGTH:
    if (Link[index].Type <= EN_PIPE) {
      if (value <= 0.0)
        return (202);
      Link[index].Len = value / Ucf[ELEV];
      resistance(p, index);
    }
    break;

  case EN_ROUGHNESS:
    if (Link[index].Type <= EN_PIPE) {
      if (value <= 0.0)
        return (202);
      Link[index].Kc = value;
      if (hyd->Formflag == DW)
        Link[index].Kc /= (1000.0 * Ucf[ELEV]);
      resistance(p, index);
    }
    break;

  case EN_MINORLOSS:
    if (Link[index].Type != EN_PUMP) {
      if (value <= 0.0)
        return (202);
      Link[index].Km =
          0.02517 * value / SQR(Link[index].Diam) / SQR(Link[index].Diam);
    }
    break;

  case EN_INITSTATUS:
  case EN_STATUS:
    /* Cannot set status for a check valve */
    if (Link[index].Type == EN_CVPIPE)
      return (207);
    s = (char)ROUND(value);
    if (s < 0 || s > 1)
      return (251);
    if (code == EN_INITSTATUS)
      setlinkstatus(p, index, s, &Link[index].Stat, &Link[index].Kc);
    else
      setlinkstatus(p, index, s, &hyd->LinkStatus[index], &LinkSetting[index]);
    break;

  case EN_INITSETTING:
  case EN_SETTING:
    if (value < 0.0)
      return (202);
    if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
      return (ENsetlinkvalue(index, EN_ROUGHNESS, v));
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
        return (202); /* Cannot modify setting for GPV */

      default:
        return (251);
      }
      if (code == EN_INITSETTING)
        setlinksetting(p, index, value, &Link[index].Stat, &Link[index].Kc);
      else
        setlinksetting(p, index, value, &hyd->LinkStatus[index],
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
    return (251);
  }
  return (0);
}

int DLLEXPORT EN_addpattern(EN_Project *p, char *id) {
  int i, j, n, err = 0;
  Spattern *tmpPat;

  EN_Network *net = &p->network;
  parser_data_t *par = &p->parser;
  
  Spattern *Pattern = net->Pattern;
  
  const int Npats = net->Npats;
  
  
  /* Check if a pattern with same id already exists */

  if (!p->Openflag)
    return (102);
  if (ENgetpatternindex(id, &i) == 0)
    return (215);

  /* Check that id name is not too long */

  if (strlen(id) > MAXID)
    return (250);

  /* Allocate memory for a new array of patterns */

  n = Npats + 1;
  tmpPat = (Spattern *)calloc(n + 1, sizeof(Spattern));
  if (tmpPat == NULL)
    return (101);

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
    return (101);
  }

  // Replace old pattern array with new one

  for (i = 0; i <= Npats; i++)
    free(Pattern[i].F);
  free(Pattern);
  Pattern = tmpPat;
  net->Npats = n;
  par->MaxPats = n;
  return 0;
}

int DLLEXPORT EN_setpattern(EN_Project *p, int index, EN_API_FLOAT_TYPE *f, int n) {
  int j;

  EN_Network *net = &p->network;
  Spattern *Pattern = net->Pattern;
  const int Npats = net->Npats;
  
  
  /* Check for valid arguments */
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Npats)
    return (205);
  if (n <= 0)
    return (202);

  /* Re-set number of time periods & reallocate memory for multipliers */
  Pattern[index].Length = n;
  Pattern[index].F = (double *)realloc(Pattern[index].F, n * sizeof(double));
  if (Pattern[index].F == NULL)
    return (101);

  /* Load multipliers into pattern */
  for (j = 0; j < n; j++)
    Pattern[index].F[j] = f[j];
  return (0);
}

int DLLEXPORT EN_setpatternvalue(EN_Project *p, int index, int period, EN_API_FLOAT_TYPE value) {
  
  EN_Network *net = &p->network;
  
  Spattern *Pattern = net->Pattern;
  
  const int Npats = net->Npats;
  
  
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Npats)
    return (205);
  if (period <= 0 || period > Pattern[index].Length)
    return (251);
  Pattern[index].F[period - 1] = value;
  return (0);
}

int DLLEXPORT EN_addcurve(EN_Project *p, char *id) {
  
  EN_Network *net = &p->network;
  parser_data_t *par = &p->parser;
  Scurve *Curve = net->Curve;
  
  
  
  int i, j, n, err = 0;
  Scurve *tmpCur;

  /* Check if a curve with same id already exists */

  if (!p->Openflag)
    return (102);
  if (ENgetcurveindex(id, &i) == 0)
    return (215);

  /* Check that id name is not too long */

  if (strlen(id) > MAXID)
    return (250);

  /* Allocate memory for a new array of curves */

  n = net->Ncurves + 1;
  tmpCur = (Scurve *)calloc(n + 1, sizeof(Scurve));
  if (tmpCur == NULL)
    return (101);

  /* Copy contents of old curve array to new one */

  for (i = 0; i <= net->Ncurves; i++) {
    strcpy(tmpCur[i].ID, Curve[i].ID);
    tmpCur[i].Npts = Curve[i].Npts;
    tmpCur[i].X = (double *)calloc(Curve[i].Npts, sizeof(double));
    tmpCur[i].Y = (double *)calloc(Curve[i].Npts, sizeof(double));
    if (tmpCur[i].X == NULL)
      err = 1;
    else if (tmpCur[i].Y == NULL)
      err = 1;
    else
      for (j = 0; j < Curve[i].Npts; j++) {
        tmpCur[i].X[j] = Curve[i].X[j];
        tmpCur[i].Y[j] = Curve[i].Y[j];
      }
  }

  /* Add the new Curve to the new array of curves */

  strcpy(tmpCur[n].ID, id);
  tmpCur[n].Npts = 1;
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
    return (101);
  }

  // Replace old curves array with new one

  for (i = 0; i <= net->Ncurves; i++) {
    free(Curve[i].X);
    free(Curve[i].Y);
  }
  free(Curve);
  Curve = tmpCur;
  net->Ncurves = n;
  par->MaxCurves = n;
  return 0;
}

int DLLEXPORT EN_setcurve(EN_Project *p, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y, int n) {
  
  EN_Network *net = &p->network;  
  Scurve *Curve = net->Curve;
  
  
  int j;

  /* Check for valid arguments */
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > net->Ncurves)
    return (206);
  if (n <= 0)
    return (202);

  /* Re-set number of points & reallocate memory for values */
  Curve[index].Npts = n;
  Curve[index].X = (double *)realloc(Curve[index].X, n * sizeof(double));
  Curve[index].Y = (double *)realloc(Curve[index].Y, n * sizeof(double));
  if (Curve[index].X == NULL)
    return (101);
  if (Curve[index].Y == NULL)
    return (101);

  /* Load values into curve */
  for (j = 0; j < n; j++) {
    Curve[index].X[j] = x[j];
    Curve[index].Y[j] = y[j];
  }
  return (0);
}

int DLLEXPORT EN_setcurvevalue(EN_Project *p, int index, int pnt, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y) {
  
  EN_Network *net = &p->network;
  
  Scurve *Curve = net->Curve;
  
  const int Ncurves = net->Ncurves;
  
  
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > Ncurves)
    return (206);
  if (pnt <= 0 || pnt > Curve[index].Npts)
    return (251);
  Curve[index].X[pnt - 1] = x;
  Curve[index].Y[pnt - 1] = y;
  return (0);
}

int DLLEXPORT EN_settimeparam(EN_Project *p, int code, long value)
{
  report_options_t *rep = &p->report;
  quality_t *qu = &p->quality;
  time_options_t *time = &p->time_options;
  
  if (!p->Openflag)
    return (102);
  if (p->hydraulics.OpenHflag || p->quality.OpenQflag) {
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
    return (202);
  switch (code) {
  case EN_DURATION:
    time->Dur = value;
    if (time->Rstart > time->Dur)
      time->Rstart = 0;
    break;

  case EN_HYDSTEP:
    if (value == 0)
      return (202);
    time->Hstep = value;
    time->Hstep = MIN(time->Pstep, time->Hstep);
    time->Hstep = MIN(time->Rstep, time->Hstep);
    qu->Qstep = MIN(qu->Qstep, time->Hstep);
    break;

  case EN_QUALSTEP:
    if (value == 0)
      return (202);
    qu->Qstep = value;
    qu->Qstep = MIN(qu->Qstep, time->Hstep);
    break;

  case EN_PATTERNSTEP:
    if (value == 0)
      return (202);
    time->Pstep = value;
    if (time->Hstep > time->Pstep)
      time->Hstep = time->Pstep;
    break;

  case EN_PATTERNSTART:
    time->Pstart = value;
    break;

  case EN_REPORTSTEP:
    if (value == 0)
      return (202);
    time->Rstep = value;
    if (time->Hstep > time->Rstep)
      time->Hstep = time->Rstep;
    break;

  case EN_REPORTSTART:
    if (time->Rstart > time->Dur)
      return (202);
    time->Rstart = value;
    break;

  case EN_RULESTEP:
    if (value == 0)
      return (202);
    time->Rulestep = value;
    time->Rulestep = MIN(time->Rulestep, time->Hstep);
    break;

  case EN_STATISTIC:
    if (value > RANGE)
      return (202);
    rep->Tstatflag = (char)value;
    break;

  case EN_HTIME:
    time->Htime = value;
    break;

  case EN_QTIME:
    qu->Qtime = value;
    break;

  default:
    return (251);
  }
  return (0);
}

int DLLEXPORT EN_setoption(EN_Project *p, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
 **  Input:   code  = option code (see EPANET2.H)
 **           v = option value
 **  Output:  none
 **  Returns: error code
 **  Purpose: sets value for an analysis option
 **----------------------------------------------------------------
 */
{
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  
  Snode *Node = net->Node;
  const int Njuncs = net->Njuncs;
  
  double *Ucf = p->Ucf;
  
  int i, j;
  double Ke, n, ucf, value = v;
  if (!p->Openflag)
    return (102);
  switch (code) {
  case EN_TRIALS:
    if (value < 1.0)
      return (202);
    hyd->MaxIter = (int)value;
    break;
  case EN_ACCURACY:
    if (value < 1.e-5 || value > 1.e-1)
      return (202);
    hyd->Hacc = value;
    break;
  case EN_TOLERANCE:
    if (value < 0.0)
      return (202);
    qu->Ctol = value / Ucf[QUALITY];
    break;
  case EN_EMITEXPON:
    if (value <= 0.0)
      return (202);
    n = 1.0 / value;
    ucf = pow(Ucf[FLOW], n) / Ucf[PRESSURE];
    for (i = 1; i <= Njuncs; i++) {
      j = ENgetnodevalue(i, EN_EMITTER, &v);
      Ke = v;
      if (j == 0 && Ke > 0.0)
        Node[i].Ke = ucf / pow(Ke, n);
    }
    hyd->Qexp = n;
    break;
  case EN_DEMANDMULT:
    if (value <= 0.0)
      return (202);
    hyd->Dmult = value;
    break;
  default:
    return (251);
  }
  return (0);
}

int DLLEXPORT EN_setstatusreport(EN_Project *p, int code) {
  int errcode = 0;
  if (code >= 0 && code <= 2)
    p->report.Statflag = (char)code;
  else
    errcode = 202;
  return (errcode);
}

int DLLEXPORT EN_setqualtype(EN_Project *p, int qualcode, char *chemname, char *chemunits, char *tracenode) {
  
  EN_Network *net = &p->network;
  report_options_t *rep = &p->report;
  quality_t *qu = &p->quality;
  
  double *Ucf = p->Ucf;
  
  
  /*** Updated 3/1/01 ***/
  double ccf = 1.0;

  if (!p->Openflag)
    return (102);
  if (qualcode < EN_NONE || qualcode > EN_TRACE)
    return (251);
  qu->Qualflag = (char)qualcode;
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
      return (203);
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

  /*** Updated 3/1/01 ***/
  Ucf[QUALITY] = ccf;
  Ucf[LINKQUAL] = ccf;
  Ucf[REACTRATE] = ccf;

  return (0);
}

int DLLEXPORT EN_getheadcurveindex(EN_Project *p, int index, int *curveindex) {
  
  EN_Network *net = &p->network;  
  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  const int Nlinks = net->Nlinks;
  
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type)
    return (204);
  *curveindex = Pump[findpump(net, index)].Hcurve;
  return (0);
}

int DLLEXPORT EN_setheadcurveindex(EN_Project *p, int index, int curveindex) {
  
  EN_Network *net = &p->network;
  
  Slink *Link = net->Link;
  const int Nlinks = net->Nlinks;
  const int Ncurves = net->Ncurves;
  
  double *Ucf = p->Ucf;
  
  
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type) {
    return (204);
  }
  if (curveindex <= 0 || curveindex > Ncurves) {
    return (206);
  }
  int pIdx = findpump(net, index);
  Spump *pump = &p->network.Pump[pIdx];
  pump->Ptype = NOCURVE;
  pump->Hcurve = curveindex;
  // update pump parameters
  getpumpparams(p);
  // convert units
  if (pump->Ptype == POWER_FUNC) {
    pump->H0 /= Ucf[HEAD];
    pump->R *= (pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
  }
  /* Convert flow range & max. head units */
  pump->Q0 /= Ucf[FLOW];
  pump->Qmax /= Ucf[FLOW];
  pump->Hmax /= Ucf[HEAD];

  return (0);
}

int DLLEXPORT EN_getpumptype(EN_Project *p, int index, int *type) {
  
  EN_Network *net = &p->network;
  
  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  const int Nlinks = net->Nlinks;
  
  *type = -1;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type)
    return (204);
  *type = Pump[findpump(&p->network, index)].Ptype;
  return (0);
}

/*
----------------------------------------------------------------
   Functions for opening files
----------------------------------------------------------------
*/

int openfiles(EN_Project *p, char *f1, char *f2, char *f3)
/*----------------------------------------------------------------
**  Input:   f1 = pointer to name of input file
**           f2 = pointer to name of report file
**           f3 = pointer to name of binary output file
**  Output:  none
**  Returns: error code
**  Purpose: opens input & report files
**----------------------------------------------------------------
*/
{
  
  out_file_t *out = &p->out_files;
  report_options_t *rep = &p->report;
  parser_data_t *par = &p->parser;
  
  /* Initialize file pointers to NULL */
  par->InFile = NULL;
  rep->RptFile = NULL;
  out->OutFile = NULL;
  out->HydFile = NULL;

  /* Save file names */
  strncpy(par->InpFname, f1, MAXFNAME);
  strncpy(rep->Rpt1Fname, f2, MAXFNAME);
  strncpy(out->OutFname, f3, MAXFNAME);
  if (strlen(f3) > 0)
    out->Outflag = SAVE; 
  else
    out->Outflag = SCRATCH; 

  /* Check that file names are not identical */
  if (strcomp(f1, f2) || strcomp(f1, f3) ||
      (strcomp(f2, f3) && (strlen(f2) > 0 || strlen(f3) > 0))) {
    writecon(FMT04);
    return (301);
  }

  /* Attempt to open input and report files */
  if ((par->InFile = fopen(f1, "rt")) == NULL) {
    writecon(FMT05);
    writecon(f1);
    return (302);
  }
  if (strlen(f2) == 0)
    rep->RptFile = stdout;
  else if ((rep->RptFile = fopen(f2, "wt")) == NULL) {
    writecon(FMT06);
    return (303);
  }

  return (0);
} /* End of openfiles */

int openhydfile(EN_Project *p)
/*----------------------------------------------------------------
** Input:   none
** Output:  none
** Returns: error code
** Purpose: opens file that saves hydraulics solution
**----------------------------------------------------------------
*/
{
  
  EN_Network *net = &p->network;
  out_file_t *out = &p->out_files;
  time_options_t *time = &p->time_options;
  
  const int Nnodes = net->Nnodes;
  const int Ntanks = net->Ntanks;
  const int Nlinks = net->Nlinks;
  const int Nvalves = net->Nvalves;
  const int Npumps = net->Npumps;
  
  INT4 nsize[6]; /* Temporary array */
  INT4 magic;
  INT4 version;
  int errcode = 0;

  /* If HydFile currently open, then close it if its not a scratch file */
  if (out->HydFile != NULL) {
    if (out->Hydflag == SCRATCH)
      return (0);
    fclose(out->HydFile);
  }

  /* Use Hydflag to determine the type of hydraulics file to use. */
  /* Write error message if the file cannot be opened.            */
  out->HydFile = NULL;
  switch (out->Hydflag) {
  case SCRATCH:
    getTmpName(out->HydFname);             
    out->HydFile = fopen(out->HydFname, "w+b"); 
    break;
  case SAVE:
    out->HydFile = fopen(out->HydFname, "w+b");
    break;
  case USE:
    out->HydFile = fopen(out->HydFname, "rb");
    break;
  }
  if (out->HydFile == NULL)
    return (305);

  /* If a previous hydraulics solution is not being used, then */
  /* save the current network size parameters to the file.     */
  if (out->Hydflag != USE) {
    magic = MAGICNUMBER;
    version = ENGINE_VERSION;
    nsize[0] = Nnodes;
    nsize[1] = Nlinks;
    nsize[2] = Ntanks;
    nsize[3] = Npumps;
    nsize[4] = Nvalves;
    nsize[5] = (int)time->Dur;
    fwrite(&magic, sizeof(INT4), 1, out->HydFile);
    fwrite(&version, sizeof(INT4), 1, out->HydFile);
    fwrite(nsize, sizeof(INT4), 6, out->HydFile);
  }

  /* If a previous hydraulics solution is being used, then */
  /* make sure its network size parameters match those of  */
  /* the current network.                                  */
  if (out->Hydflag == USE) {
    fread(&magic, sizeof(INT4), 1, out->HydFile);
    if (magic != MAGICNUMBER)
      return (306);
    fread(&version, sizeof(INT4), 1, out->HydFile);
    if (version != ENGINE_VERSION)
      return (306);
    if (fread(nsize, sizeof(INT4), 6, out->HydFile) < 6)
      return (306);
    if (nsize[0] != Nnodes || nsize[1] != Nlinks || nsize[2] != Ntanks ||
        nsize[3] != Npumps || nsize[4] != Nvalves || nsize[5] != time->Dur)
      return (306);
    p->save_options.SaveHflag = TRUE;
  }

  /* Save current position in hydraulics file  */
  /* where storage of hydraulic results begins */
  out->HydOffset = ftell(out->HydFile);
  return (errcode);
}

int openoutfile(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: opens binary output file.
**----------------------------------------------------------------
*/
{
  int errcode = 0;
  
  out_file_t *out = &p->out_files;
  report_options_t *rep = &p->report;  

  /* Close output file if already opened */
  if (out->OutFile != NULL)
    fclose(out->OutFile);
  out->OutFile = NULL;
  if (out->TmpOutFile != NULL)
    fclose(out->TmpOutFile);
  out->TmpOutFile = NULL;

  if (out->Outflag == SCRATCH) {
    remove(out->OutFname); 
  }
  remove(out->TmpFname);   

  /* If output file name was supplied, then attempt to */
  /* open it. Otherwise open a temporary output file.  */
  // if (strlen(OutFname) != 0) 
  if (out->Outflag == SAVE) 
  {
    if ((out->OutFile = fopen(out->OutFname, "w+b")) == NULL) {
      writecon(FMT07);
      errcode = 304;
    }
  }
  // else if ( (OutFile = tmpfile()) == NULL) 
  else 
  {
    getTmpName(out->OutFname);                           
    if ((out->OutFile = fopen(out->OutFname, "w+b")) == NULL) 
    {
      writecon(FMT08);
      errcode = 304;
    }
  }

  /* Save basic network data & energy usage results */
  ERRCODE(savenetdata(p));
  out->OutOffset1 = ftell(out->OutFile);
  ERRCODE(saveenergy(p));
  out->OutOffset2 = ftell(out->OutFile);

  /* Open temporary file if computing time series statistic */
  if (!errcode) {
    if (rep->Tstatflag != SERIES) {
      // if ( (TmpOutFile = tmpfile()) == NULL) errcode = 304; 
      getTmpName(out->TmpFname);                
      out->TmpOutFile = fopen(out->TmpFname, "w+b"); 
      if (out->TmpOutFile == NULL)
        errcode = 304; 
    } else
      out->TmpOutFile = out->OutFile;
  }
  return (errcode);
}

/*
----------------------------------------------------------------
   Global memory management functions
----------------------------------------------------------------
*/

void initpointers(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes global pointers to NULL
**----------------------------------------------------------------
*/
{

  hydraulics_t *hyd = &p->hydraulics;
  quality_t *q = &p->quality;
  EN_Network *n = &p->network;
  parser_data_t *pars = &p->parser;
  solver_t *s = &p->hydraulics.solver;

  hyd->NodeDemand = NULL;
  q->NodeQual = NULL;
  hyd->NodeHead = NULL;
  hyd->LinkFlows = NULL;
  q->PipeRateCoeff = NULL;
  hyd->LinkStatus = NULL;
  hyd->LinkSetting = NULL;
  hyd->OldStat = NULL;

  n->Node = NULL;
  n->Link = NULL;
  n->Tank = NULL;
  n->Pump = NULL;
  n->Valve = NULL;
  n->Pattern = NULL;
  n->Curve = NULL;
  n->Control = NULL;
  n->Coord = NULL;

  hyd->X_tmp = NULL;

  pars->Patlist = NULL;
  pars->Curvelist = NULL;
  n->Adjlist = NULL;

  s->Aii = NULL;
  s->Aij = NULL;
  s->F = NULL;
  s->P = NULL;
  s->Y = NULL;
  s->Order = NULL;
  s->Row = NULL;
  s->Ndx = NULL;
  s->XLNZ = NULL;
  s->NZSUB = NULL;
  s->LNZ = NULL;

  n->NodeHashTable = NULL;
  n->LinkHashTable = NULL;
  initrules(&p->rules);
}

int allocdata(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: allocates memory for network data structures
**----------------------------------------------------------------
*/
{
  int n;
  int errcode = 0;

  /* Allocate node & link ID hash tables */
  p->network.NodeHashTable = ENHashTableCreate();
  p->network.LinkHashTable = ENHashTableCreate();
  ERRCODE(MEMCHECK(p->network.NodeHashTable));
  ERRCODE(MEMCHECK(p->network.LinkHashTable));

  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  parser_data_t *par = &p->parser;
  
  
  /* Allocate memory for network nodes */
  /*************************************************************
   NOTE: Because network components of a given type are indexed
         starting from 1, their arrays must be sized 1
         element larger than the number of components.
  *************************************************************/
  if (!errcode) {
    n = par->MaxNodes + 1;
    net->Node = (Snode *)calloc(n, sizeof(Snode));
    hyd->NodeDemand = (double *)calloc(n, sizeof(double));
    qu->NodeQual = (double *)calloc(n, sizeof(double));
    hyd->NodeHead = (double *)calloc(n, sizeof(double));
    ERRCODE(MEMCHECK(net->Node));
    ERRCODE(MEMCHECK(hyd->NodeDemand));
    ERRCODE(MEMCHECK(qu->NodeQual));
    ERRCODE(MEMCHECK(hyd->NodeHead));
  }

  /* Allocate memory for network links */
  if (!errcode) {
    n = par->MaxLinks + 1;
    net->Link = (Slink *)calloc(n, sizeof(Slink));
    hyd->LinkFlows = (double *)calloc(n, sizeof(double));
    hyd->LinkSetting = (double *)calloc(n, sizeof(double));
    hyd->LinkStatus = (StatType *)calloc(n, sizeof(StatType));
    ERRCODE(MEMCHECK(net->Link));
    ERRCODE(MEMCHECK(hyd->LinkFlows));
    ERRCODE(MEMCHECK(hyd->LinkSetting));
    ERRCODE(MEMCHECK(hyd->LinkStatus));
  }

  /* Allocate memory for tanks, sources, pumps, valves,   */
  /* controls, demands, time patterns, & operating curves */
  if (!errcode) {
    net->Tank = (Stank *)calloc(par->MaxTanks + 1, sizeof(Stank));
    net->Pump = (Spump *)calloc(par->MaxPumps + 1, sizeof(Spump));
    net->Valve = (Svalve *)calloc(par->MaxValves + 1, sizeof(Svalve));
    net->Control = (Scontrol *)calloc(par->MaxControls + 1, sizeof(Scontrol));
    net->Pattern = (Spattern *)calloc(par->MaxPats + 1, sizeof(Spattern));
    net->Curve = (Scurve *)calloc(par->MaxCurves + 1, sizeof(Scurve));
    if (p->parser.Coordflag == TRUE) {
      net->Coord = (Scoord *)calloc(par->MaxNodes + 1, sizeof(Scoord));
    }
    ERRCODE(MEMCHECK(net->Tank));
    ERRCODE(MEMCHECK(net->Pump));
    ERRCODE(MEMCHECK(net->Valve));
    ERRCODE(MEMCHECK(net->Control));
    ERRCODE(MEMCHECK(net->Pattern));
    ERRCODE(MEMCHECK(net->Curve));
    if (p->parser.Coordflag == TRUE)
      ERRCODE(MEMCHECK(net->Coord));
  }

  /* Initialize pointers used in patterns, curves, and demand category lists */
  if (!errcode) {
    for (n = 0; n <= par->MaxPats; n++) {
      net->Pattern[n].Length = 0;
      net->Pattern[n].F = NULL;
    }
    for (n = 0; n <= par->MaxCurves; n++) {
      net->Curve[n].Npts = 0;
      net->Curve[n].Type = -1;
      net->Curve[n].X = NULL;
      net->Curve[n].Y = NULL;
    }

    for (n = 0; n <= par->MaxNodes; n++) {
      // node demand
      net->Node[n].D = NULL;
      /* ini coord data */
      if (p->parser.Coordflag == TRUE) {
        net->Coord[n].X = 0;
        net->Coord[n].Y = 0;
        net->Coord[n].HaveCoords = FALSE;
      }
    }
  }

  /* Allocate memory for rule base (see RULES.C) */
  if (!errcode)
    errcode = allocrules(p);
  return (errcode);
} /* End of allocdata */

void freeTmplist(STmplist *t)
/*----------------------------------------------------------------
**  Input:   t = pointer to start of a temporary list
**  Output:  none
**  Purpose: frees memory used for temporary storage
**           of pattern & curve data
**----------------------------------------------------------------
*/
{
  STmplist *tnext;
  while (t != NULL) {
    tnext = t->next;
    freeFloatlist(t->x);
    freeFloatlist(t->y);
    free(t);
    t = tnext;
  }
}

void freeFloatlist(SFloatlist *f)
/*----------------------------------------------------------------
**  Input:   f = pointer to start of list of floats
**  Output:  none
**  Purpose: frees memory used for storing list of floats
**----------------------------------------------------------------
*/
{
  SFloatlist *fnext;
  while (f != NULL) {
    fnext = f->next;
    free(f);
    f = fnext;
  }
}

void freedata(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: frees memory allocated for network data structures.
**----------------------------------------------------------------
*/
{
  
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  parser_data_t *par = &p->parser;
    
  int j;
  Pdemand demand, nextdemand;
  Psource source;

  /* Free memory for computed results */
  free(hyd->NodeDemand);
  free(qu->NodeQual);
  free(hyd->NodeHead);
  free(hyd->LinkFlows);
  free(hyd->LinkSetting);
  free(hyd->LinkStatus);

  /* Free memory for node data */
  if (net->Node != NULL) {
    for (j = 0; j <= par->MaxNodes; j++) {
      /* Free memory used for demand category list */
      demand = net->Node[j].D;
      while (demand != NULL) {
        nextdemand = demand->next;
        free(demand);
        demand = nextdemand;
      }
      /* Free memory used for WQ source data */
      source = net->Node[j].S;
      if (source != NULL)
        free(source);
    }
    free(net->Node);
  }

  /* Free memory for other network objects */
  free(net->Link);
  free(net->Tank);
  free(net->Pump);
  free(net->Valve);
  free(net->Control);

  /* Free memory for time patterns */
  if (net->Pattern != NULL) {
    for (j = 0; j <= par->MaxPats; j++)
      free(net->Pattern[j].F);
    free(net->Pattern);
  }

  /* Free memory for curves */
  if (net->Curve != NULL) {
    for (j = 0; j <= par->MaxCurves; j++) {
      free(net->Curve[j].X);
      free(net->Curve[j].Y);
    }
    free(net->Curve);
  }

  /* Free memory for node coordinates */
  if (p->parser.Coordflag == TRUE) {
    free(net->Coord);
  }

  /* Free memory for rule base (see RULES.C) */
  freerules(p);

  /* Free hash table memory */
  if (net->NodeHashTable != NULL)
    ENHashTableFree(net->NodeHashTable);
  if (net->LinkHashTable != NULL)
    ENHashTableFree(net->LinkHashTable);
}

/*
----------------------------------------------------------------
   General purpose functions
----------------------------------------------------------------
*/

/*** New function for 2.00.12 ***/ 
char *getTmpName(char *fname)
//
//  Input:   fname = file name string
//  Output:  returns pointer to file name
//  Purpose: creates a temporary file name with path prepended to it.
//
{
// --- for Windows systems:
#ifdef WINDOWS
  // --- use system function tmpnam() to create a temporary file name
  char name[MAXFNAME + 1];
  int n;
  tmpnam(name);
  
  // --- if user supplied the name of a temporary directory,
  //     then make it be the prefix of the full file name
  n = (int)strlen(TmpDir);
  if (n > 0) {
    strcpy(fname, TmpDir);
    if (fname[n - 1] != '\\')
      strcat(fname, "\\");
  }

  // --- otherwise, use the relative path notation as the file name
  //     prefix so that the file will be placed in the current directory
  else {
    strcpy(fname, ".\\");
  }

  // --- now add the prefix to the file name
  strcat(fname, name);

// --- for non-Windows systems:
#else
  // --- use system function mkstemp() to create a temporary file name
  strcpy(fname, "enXXXXXX");
  mkstemp(fname);
#endif
  return fname;
}

int strcomp(char *s1, char *s2)
/*---------------------------------------------------------------
**  Input:   s1 = character string
**           s2 = character string
**  Output:  none
**  Returns: 1 if s1 is same as s2, 0 otherwise
**  Purpose: case insensitive comparison of strings s1 & s2
**---------------------------------------------------------------
*/
{
  int i;
  for (i = 0; UCHAR(s1[i]) == UCHAR(s2[i]); i++)
    if (!s1[i + 1] && !s2[i + 1])
      return (1);
  return (0);
} /*  End of strcomp  */

double interp(int n, double x[], double y[], double xx)
/*----------------------------------------------------------------
**  Input:   n  = number of data pairs defining a curve
**           x  = x-data values of curve
**           y  = y-data values of curve
**           xx = specified x-value
**  Output:  none
**  Returns: y-value on curve at x = xx
**  Purpose: uses linear interpolation to find y-value on a
**           data curve corresponding to specified x-value.
**  NOTE:    does not extrapolate beyond endpoints of curve.
**----------------------------------------------------------------
*/
{
  int k, m;
  double dx, dy;

  m = n - 1; /* Highest data index      */
  if (xx <= x[0])
    return (y[0]);         /* xx off low end of curve */
  for (k = 1; k <= m; k++) /* Bracket xx on curve     */
  {
    if (x[k] >= xx) /* Interp. over interval   */
    {
      dx = x[k] - x[k - 1];
      dy = y[k] - y[k - 1];
      if (ABS(dx) < TINY)
        return (y[k]);
      else
        return (y[k] - (x[k] - xx) * dy / dx);
    }
  }
  return (y[m]); /* xx off high end of curve */
} /* End of interp */

int findnode(EN_Network *n, char *id)
/*----------------------------------------------------------------
**  Input:   id = node ID
**  Output:  none
**  Returns: index of node with given ID, or 0 if ID not found
**  Purpose: uses hash table to find index of node with given ID
**----------------------------------------------------------------
*/
{
  return (ENHashTableFind(n->NodeHashTable, id));
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
  return (ENHashTableFind(n->LinkHashTable, id));
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
#include "errors.dat"
#undef DAT
    default:
      strcpy(msg, "");
  }
  return (msg);
}

void errmsg(EN_Project *p, int errcode)
/*----------------------------------------------------------------
**  Input:   errcode = error code
**  Output:  none
**  Purpose: writes error message to report file
**----------------------------------------------------------------
*/
{
  if (errcode == 309) /* Report file write error -  */
  {                   /* Do not write msg to file.  */
    writecon("\n  ");
    writecon(geterrmsg(errcode,p->Msg));
  } else if (p->report.RptFile != NULL && p->report.Messageflag) {
    writeline(p, geterrmsg(errcode,p->Msg));
  }
}

void writecon(char *s)
/*----------------------------------------------------------------
**  Input:   text string
**  Output:  none
**  Purpose: writes string of characters to console
**----------------------------------------------------------------
*/
{
  
  fprintf(stdout, "%s", s);
  fflush(stdout);
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

int DLLEXPORT EN_getnumdemands(EN_Project *p, int nodeIndex, int *numDemands) {
  Pdemand d;
  int n = 0;
  /* Check for valid arguments */
  if (!p->Openflag)
    return (102);
  if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes)
    return (203);
  for (d = p->network.Node[nodeIndex].D; d != NULL; d = d->next)
    n++;
  *numDemands = n;
  return 0;
}

int DLLEXPORT EN_getbasedemand(EN_Project *p, int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE *baseDemand) {
  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!p->Openflag)
    return (102);
  if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes)
    return (203);
  if (nodeIndex <= p->network.Njuncs) {
    for (d = p->network.Node[nodeIndex].D; n < demandIdx && d != NULL; d = d->next) {
      n++;
    }
    if (n != demandIdx) {
      return (253);
    }
    *baseDemand = (EN_API_FLOAT_TYPE)(d->Base * p->Ucf[FLOW]);
  } else {
    *baseDemand = (EN_API_FLOAT_TYPE)(0.0);
  }
  return 0;
}

int DLLEXPORT EN_setbasedemand(EN_Project *pr, int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE baseDemand) {
  
  EN_Network *net = &pr->network;
  Snode *Node = net->Node;
  
  const int Nnodes = net->Nnodes;
  const int Njuncs = net->Njuncs;
  
  double *Ucf = pr->Ucf;
  
  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!pr->Openflag)
    return (102);
  if (nodeIndex <= 0 || nodeIndex > Nnodes)
    return (203);
  if (nodeIndex <= Njuncs) {
    for (d = Node[nodeIndex].D; n < demandIdx && d != NULL; d = d->next)
      n++;
    if (n != demandIdx)
      return (253);
    d->Base = baseDemand / Ucf[FLOW];
  }
  return 0;
}

int DLLEXPORT EN_getdemandpattern(EN_Project *p, int nodeIndex, int demandIdx, int *pattIdx) {
  
  EN_Network *net = &p->network;
  Snode *Node = net->Node;
  const int Nnodes = net->Nnodes;
  
  Pdemand d;
  int n = 1;
  /* Check for valid arguments */
  if (!p->Openflag)
    return (102);
  if (nodeIndex <= 0 || nodeIndex > Nnodes)
    return (203);
  for (d = Node[nodeIndex].D; n < demandIdx && d != NULL; d = d->next)
    n++;
  if (n != demandIdx)
    return (253);
  *pattIdx = d->Pat;
  return 0;
}

int DLLEXPORT EN_getaveragepatternvalue(EN_Project *p, int index, EN_API_FLOAT_TYPE *value) {
  
  EN_Network *net = &p->network;
  
  Spattern *Pattern = net->Pattern;
  const int Npats = net->Npats;
  
  int i;
  *value = 0.0;
  if (!p->Openflag)
    return (102);
  if (index < 1 || index > Npats)
    return (205);
  // if (period < 1 || period > Pattern[index].Length) return(251);
  for (i = 0; i < Pattern[index].Length; i++) {
    *value += Pattern[index].F[i];
  }
  *value /= (EN_API_FLOAT_TYPE)Pattern[index].Length;
  return (0);
}

int DLLEXPORT EN_setlinktype(EN_Project *p, char *id, EN_LinkType toType) {
  int i;
  EN_LinkType fromType;
  
  EN_Network *net = &p->network;

  if (!p->Openflag)
    return (102);

  /* Check if a link with the id exists */
  if (EN_getlinkindex(p, id, &i) != 0)
    return (215);

  /* Get the current type of the link */
  ENgetlinktype(i, &fromType);
  if (fromType == toType)
    return (0);

  /* Change link from Pipe */
  if (toType <= EN_PIPE) {
    net->Npipes++;
  } else if (toType == EN_PUMP) {
    net->Npumps++;
    net->Pump[net->Npumps].Link = i;
  } else {
    net->Nvalves++;
    net->Valve[net->Nvalves].Link = i;
  }

  if (fromType <= EN_PIPE) {
    net->Npipes--;
  } else if (fromType == EN_PUMP) {
    net->Npumps--;
  }
  return 0;
}

int DLLEXPORT EN_addnode(EN_Project *p, char *id, EN_NodeType nodeType) {
  int i, nIdx;
  int index;
  struct Sdemand *demand;
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;
  quality_t *qu = &p->quality;
  
  /* Check if a node with same id already exists */
  if (!p->Openflag)
    return (102);
  if (ENgetnodeindex(id, &i) == 0)
    return (215);

  /* Check that id name is not too long */
  if (strlen(id) > MAXID)
    return (250);

  /* Grow arrays to accomodate the new values */
  net->Node = (Snode *)realloc(net->Node, (net->Nnodes + 2) * sizeof(Snode));
  net->Coord = realloc(net->Coord, (net->Nnodes + 2) * sizeof(Scoord));
  hyd->NodeDemand = (double *)realloc(hyd->NodeDemand, (net->Nnodes + 2) * sizeof(double));
  qu->NodeQual = (double *)realloc(qu->NodeQual, (net->Nnodes + 2) * sizeof(double));
  hyd->NodeHead = (double *)realloc(hyd->NodeHead, (net->Nnodes + 2) * sizeof(double));
  
  nIdx = net->Nnodes + 1;
  Snode *node = &net->Node[nIdx];
  Scoord *coord = &net->Coord[nIdx];
  
  if (nodeType == EN_JUNCTION) {
    net->Njuncs++;

    demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
    demand->Base = 0.0;
    demand->Pat = 0;
    demand->next = NULL;
    node->D = demand;

    // shift rest of Node array
    for (index = net->Nnodes; index >= net->Njuncs; index--) {
      ENHashTableUpdate(net->NodeHashTable, net->Node[index].ID, index + 1);
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
  } else {
    net->Ntanks++;
    
    /* resize tanks array */
    net->Tank = (Stank *)realloc(net->Tank, (net->Ntanks + 1) * sizeof(Stank));
    
    Stank *tank = &net->Tank[net->Ntanks];
    
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
  
  
  coord->HaveCoords = FALSE;
  coord->X = 0;
  coord->Y = 0;

  /* Insert new node into hash table */
  ENHashTableInsert(net->NodeHashTable, node->ID, nIdx); /* see HASH.C */
  return (0);
}

int DLLEXPORT EN_addlink(EN_Project *p, char *id, EN_LinkType linkType, char *fromNode,
                        char *toNode) {
  int i, n;
  int N1, N2;
  EN_Network *net = &p->network;
  hydraulics_t *hyd = &p->hydraulics;

  /* Check if a link with same id already exists */
  if (!p->Openflag)
    return (102);
  if (ENgetlinkindex(id, &i) == 0)
    return (215);

  /* Lookup the from and to nodes */
  N1 = ENHashTableFind(net->NodeHashTable, fromNode);
  N2 = ENHashTableFind(net->NodeHashTable, toNode);

  if (N1 == 0 || N2 == 0) {
    return (203);
  }

  /* Check that id name is not too long */
  if (strlen(id) > MAXID)
    return (250);

  net->Nlinks++;
  n = net->Nlinks;

  /* Grow arrays to accomodate the new value */
  net->Link = (Slink *)realloc(net->Link, (net->Nlinks + 1) * sizeof(Slink));
  hyd->LinkFlows = (double *)realloc(hyd->LinkFlows, (net->Nlinks + 1) * sizeof(double));
  hyd->LinkSetting = (double *)realloc(hyd->LinkSetting, (net->Nlinks + 1) * sizeof(double));
  hyd->LinkStatus = (StatType *)realloc(hyd->LinkStatus, (net->Nlinks + 1) * sizeof(StatType));

  Slink *link = &net->Link[net->Nlinks];
  
  strncpy(net->Link[n].ID, id, MAXID);

  if (linkType <= EN_PIPE) {
    net->Npipes++;
  } else if (linkType == EN_PUMP) {
    net->Npumps++;
    /* Grow pump array to accomodate the new value */
    net->Pump = (Spump *)realloc(net->Pump, (net->Npumps + 1) * sizeof(Spump));
    Spump *pump = &net->Pump[net->Npumps];
    
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

  if (linkType == EN_PUMP) {
    link->Kc = 1.0; // Speed factor
    link->Km = 0.0; // Horsepower
    link->Len = 0.0;
  } else if (linkType == EN_PIPE) {
    link->Diam = 10 / p->Ucf[DIAM];
    link->Kc = 100; // Rough. coeff
    link->Km = 0.0; // Loss coeff
    link->Len = 1000;
  } else { // Valve
    link->Diam = 10 / p->Ucf[DIAM];
    link->Kc = 0.0; // Valve setting.
    link->Km = 0.0; // Loss coeff
    link->Len = 0.0;
  }
  // link->Len = 0.0;
  // link->Kc  = 0.01;
  // link->Km  = 0;
  link->Kb = 0;
  link->Kw = 0;
  link->R = 0;
  link->Rc = 0;
  link->Stat = 0;
  link->Rpt = 0;

  ENHashTableInsert(net->LinkHashTable, link->ID, n);
  return (0);
}

int DLLEXPORT EN_deletelink(EN_Project *p, int index) {
  int i;
  EN_LinkType linkType;
  EN_Network *net = &p->network;
  
  
  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > net->Nlinks)
    return (203);

  EN_getlinktype(p, index, &linkType);

  Slink *link = &net->Link[index];
  
  // remove from hash table
  ENHashTableDelete(net->LinkHashTable, link->ID);
  ENHashTableFind(net->LinkHashTable, link->ID);

  // shift link and pump arrays to re-sort link indices
  net->Nlinks--;
  for (i = index; i <= net->Nlinks - 1; i++) {
    net->Link[i] = net->Link[i + 1];
    // update hashtable
    ENHashTableUpdate(net->LinkHashTable, net->Link[i].ID, i);
  }
  for (i = 1; i <= net->Npumps; i++) {
    if (net->Pump[i].Link > index) {
      net->Pump[i].Link -= 1;
    }
  }
  for (i = 1; i <= net->Nvalves; i++) {
    if (net->Valve[i].Link > index) {
      net->Valve[i].Link -= 1;
    }
  }
  

  // update pumps
  if (linkType == EN_PUMP) {
    int pumpindex = findpump(net,index);
    for (i = pumpindex; i <= net->Npumps - 1; i++) {
      net->Pump[i] = net->Pump[i + 1];
    }
    net->Npumps--;
  }
  
  // update valves
  if (linkType > EN_PUMP) {
    int valveindex = findvalve(net,index);
    for (i = valveindex; i <= net->Nvalves - 1; i++) {
      net->Valve[i] = net->Valve[i + 1];
    }
    net->Nvalves--;
  }

  return 0;
}

int DLLEXPORT EN_deletenode(EN_Project *p, int index) {

  EN_Network *net = &p->network;
  
  int i, nodeType;

  if (!p->Openflag)
    return (102);
  if (index <= 0 || index > net->Nnodes)
    return (203);

  EN_getnodetype(p, index, &nodeType);

  // remove from hash table
  ENHashTableDelete(net->NodeHashTable, net->Node[index].ID);

  // shift node and coord array to remove node
  for (i = index; i <= net->Nnodes - 1; i++) {
    net->Node[i] = net->Node[i + 1];
    net->Coord[i] = net->Coord[i + 1];
    // update hashtable
    ENHashTableUpdate(net->NodeHashTable, net->Node[i].ID, i);
  }

  // update tank array
  if (nodeType != EN_JUNCTION) {
    int tankindex = findtank(net, index);
    for (i = tankindex; i <= net->Ntanks - 1; i++) {
      net->Tank[i] = net->Tank[i + 1];
    }
  }

  // update tank node indices
  for (i = 1; i <= net->Ntanks; i++) {
    if (net->Tank[i].Node > index) {
      net->Tank[i].Node -= 1;
    }
  }

  // gather a list of link ids to remove in reverse order,
  // so that re-shuffling doesn't destroy the indexing
  for (i = net->Nlinks; i >= 1; i--) {
    if (net->Link[i].N1 == index || net->Link[i].N2 == index) {
      EN_deletelink(p,i);
    }
  }

  for (i = 1; i <= net->Nlinks; i++) {
    if (net->Link[i].N1 > index) {
      net->Link[i].N1 -= 1;
    }
    if (net->Link[i].N2 > index) {
      net->Link[i].N2 -= 1;
    }
  }

  // update counters
  if (nodeType == EN_JUNCTION) {
    net->Njuncs--;
  } else {
    net->Ntanks--;
  }

  net->Nnodes--;

  return (0);
}

int DLLEXPORT EN_getrule(EN_Project *pr, int index, int *nPremises, int *nTrueActions, int *nFalseActions, EN_API_FLOAT_TYPE *priority)
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
    return (257);
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
  return (0);
}

int DLLEXPORT EN_getpremise(EN_Project *pr, int indexRule, int idxPremise, int *logop,
                           int *object, int *indexObj, int *variable,
                           int *relop, int *status, EN_API_FLOAT_TYPE *value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;
  
  
  
  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (idxPremise > nPremises) {
    return (258);
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
  *value = p[0].value;
  return (0);
}

int DLLEXPORT EN_setrulepriority(EN_Project *pr, int index, EN_API_FLOAT_TYPE priority)
/*----------------------------------------------------------------
**  Input:   index  = index of the rule
**           priority = rule priority
**  Output:  none
**  Returns: error code
**----------------------------------------------------------------
*/
{
  if (index > pr->network.Nrules) {
    return (257);
  }
  pr->rules.Rule[index].priority = priority;

  return (0);
}

int DLLEXPORT EN_setpremise(EN_Project *pr, int indexRule, int indexPremise, int logop,
                           int object, int indexObj, int variable, int relop,
                           int status, EN_API_FLOAT_TYPE value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;
  
  

  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return (258);
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
  return (0);
}

int DLLEXPORT EN_setpremiseindex(EN_Project *pr, int indexRule, int indexPremise, int indexObj) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules)
    return (257);
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return (258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->index = indexObj;
  return (0);
}

int DLLEXPORT EN_setpremisestatus(EN_Project *pr, int indexRule, int indexPremise, int status) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return (258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->status = status;
  return (0);
}

int DLLEXPORT EN_setpremisevalue(EN_Project *pr, int indexRule, int indexPremise,
                                EN_API_FLOAT_TYPE value) {
  int count = 1, error = 0, nPremises, a, b;
  EN_API_FLOAT_TYPE priority;
  Premise *p;

  if (indexRule > pr->network.Nrules)
    return (257);
  error = EN_getrule(pr, indexRule, &nPremises, &a, &b, &priority);
  if (indexPremise > nPremises) {
    return (258);
  }
  p = pr->rules.Rule[indexRule].Pchain;
  while (count < indexPremise) {
    count++;
    p = p->next;
  }
  p->value = value;
  return (0);
}

int DLLEXPORT EN_gettrueaction(EN_Project *pr, int indexRule, int indexAction, int *indexLink,
                              int *status, EN_API_FLOAT_TYPE *setting) {
  int count = 1, error = 0, nTrueAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return (252);
  }
  error = EN_getrule(pr, indexRule, &c, &nTrueAction, &b, &priority);
  if (indexAction > nTrueAction) {
    return (253);
  }
  a = pr->rules.Rule[indexRule].Tchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  *indexLink = a->link;
  *status = a->status;
  *setting = a->setting;
  return (0);
}

int DLLEXPORT EN_settrueaction(EN_Project *pr, int indexRule, int indexAction, int indexLink,
                              int status, EN_API_FLOAT_TYPE setting) {
  int count = 1, error = 0, nTrueAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &c, &nTrueAction, &b, &priority);
  if (indexAction > nTrueAction) {
    return (258);
  }
  a = pr->rules.Rule[indexRule].Tchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  a->link = indexLink;
  a->status = status;
  a->setting = setting;
  return (0);
}

int DLLEXPORT EN_getfalseaction(EN_Project *pr, int indexRule, int indexAction, int *indexLink,
                               int *status, EN_API_FLOAT_TYPE *setting) {
  int count = 1, error = 0, nFalseAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &c, &b, &nFalseAction, &priority);
  if (indexAction > nFalseAction) {
    return (258);
  }
  a = pr->rules.Rule[indexRule].Fchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  *indexLink = a->link;
  *status = a->status;
  *setting = a->setting;
  return (0);
}

int DLLEXPORT EN_setfalseaction(EN_Project *pr, int indexRule, int indexAction, int indexLink,
                               int status, EN_API_FLOAT_TYPE setting) {
  int count = 1, error = 0, nFalseAction, c, b;
  EN_API_FLOAT_TYPE priority;
  Action *a;

  if (indexRule > pr->network.Nrules) {
    return (257);
  }
  error = EN_getrule(pr, indexRule, &c, &b, &nFalseAction, &priority);
  if (indexAction > nFalseAction) {
    return (258);
  }
  a = pr->rules.Rule[indexRule].Fchain;
  while (count < indexAction) {
    count++;
    a = a->next;
  }
  a->link = indexLink;
  a->status = status;
  a->setting = setting;

  return (0);
}

int DLLEXPORT EN_getruleID(EN_Project *pr, int indexRule, char *id) {
  strcpy(id, "");
  if (!pr->Openflag)
    return (102);
  if (indexRule < 1 || indexRule > pr->network.Nrules)
    return (257);
  strcpy(id, pr->rules.Rule[indexRule].label);
  return (0);
}

/*************************** END OF EPANET.C ***************************/
