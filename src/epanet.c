/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet.c
 Description:  implementation of the EPANET API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/27/2018
 ******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <float.h>
#include <math.h>

#include "epanet2.h"
#include "types.h"
#include "funcs.h"
#include "text.h"
#include "enumstxt.h"



/********************************************************************

    System Functions

********************************************************************/

int DLLEXPORT EN_createproject(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  p = pointer to a new EPANET project
**  Returns: error code
**  Purpose: creates a new EPANET project
**----------------------------------------------------------------
*/
{
    struct Project *project = (struct Project *)calloc(1, sizeof(struct Project));
    if (project == NULL) return -1;
    getTmpName(project->TmpHydFname);
    getTmpName(project->TmpOutFname);
    getTmpName(project->TmpStatFname);
    *p = project;
    return 0;
}

int DLLEXPORT EN_deleteproject(EN_Project *p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: deletes an EPANET project
**----------------------------------------------------------------
*/
{
    if (*p == NULL) return -1;
    if ((*p)->Openflag) EN_close(*p);
    remove((*p)->TmpHydFname);
    remove((*p)->TmpOutFname);
    remove((*p)->TmpStatFname);
    free(*p);
    *p = NULL;
    return 0;
}

int DLLEXPORT EN_runproject(EN_Project p, const char *f1, const char *f2,
                            const char *f3, void (*pviewprog)(char *))
/*------------------------------------------------------------------------
**   Input:   f1 = name of EPANET formatted input file
**            f2 = name of report file
**            f3 = name of binary output file
**            pviewprog = see note below
**   Output:  none
**   Returns: error code
**   Purpose: runs a complete EPANET simulation
**
**  The pviewprog() argument is a pointer to a callback function
**  that takes a character string (char *) as its only parameter.
**  The function would reside in and be used by the calling
**  program to display the progress messages that EPANET generates
**  as it carries out its computations. If this feature is not
**  needed then the argument should be NULL.
**-------------------------------------------------------------------------
*/
{
    int errcode = 0;

    // Read in project data from an input file
    ERRCODE(EN_open(p, f1, f2, f3));
    p->viewprog = pviewprog;

    // Solve for system hydraulics
    if (p->outfile.Hydflag != USE)
    {
      ERRCODE(EN_solveH(p));
    }

    // Solve for system water quality
    ERRCODE(EN_solveQ(p));

    // Write a formatted output report
    ERRCODE(EN_report(p));
    EN_close(p);

    // Return any error or warning code
    if (p->Warnflag) errcode = MAX(errcode, p->Warnflag);
    return errcode;
}

int DLLEXPORT EN_init(EN_Project p, const char *f2, const char *f3,
                      EN_FlowUnits unitsType, EN_HeadLossType headLossType)
/*----------------------------------------------------------------
 **  Input:   f2 = name of report file
 **           f3 = name of binary output file
 **           unitsType = type of flow units (see EN_FlowUnits)
 **           headLossType = type of head loss formula (see EN_HeadLossType)
 **  Output:  none
 **  Returns: error code
 **  Purpose: initializes an EPANET project that isn't opened with
 **           an input file
 **----------------------------------------------------------------
 */
{
    int errcode = 0;

    // Set system flags
    p->Openflag = TRUE;
    p->hydraul.OpenHflag = FALSE;
    p->quality.OpenQflag = FALSE;
    p->outfile.SaveHflag = FALSE;
    p->outfile.SaveQflag = FALSE;
    p->Warnflag = FALSE;
    p->report.Messageflag = TRUE;
    p->report.Rptflag = 1;

    // Open files
    errcode = openfiles(p, "", f2, f3);

    // Initialize memory used for project's data objects
    initpointers(p);
    ERRCODE(netsize(p));
    ERRCODE(allocdata(p));
    if (errcode) return (errcode);

    // Set analysis options
    setdefaults(p);
    p->parser.Flowflag = unitsType;
    p->hydraul.Formflag = headLossType;

    // Perform additional initializations
    adjustdata(p);
    initreport(&p->report);
    initunits(p);
    inittanks(p);
    convertunits(p);

    // Initialize the default demand pattern
    p->parser.MaxPats = 0;
    getpatterns(p);
    return errcode;
}

int DLLEXPORT EN_open(EN_Project p, const char *f1, const char *f2, const char *f3)
/*----------------------------------------------------------------
 **  Input:   f1 = name of input file
 **           f2 = name of report file
 **           f3 = name of binary output file
 **  Output:  none
 **  Returns: error code
 **  Purpose: opens an EPANET input file & reads in network data
 **----------------------------------------------------------------
 */
{
  int errcode = 0;

  // Set system flags
  p->Openflag = FALSE;
  p->hydraul.OpenHflag = FALSE;
  p->quality.OpenQflag = FALSE;
  p->outfile.SaveHflag = FALSE;
  p->outfile.SaveQflag = FALSE;
  p->Warnflag = FALSE;
  p->report.Messageflag = TRUE;
  p->report.Rptflag = 1;

  // Initialize data arrays to NULL
  initpointers(p);

  // Open input & report files
  ERRCODE(openfiles(p, f1, f2, f3));
  if (errcode > 0)
  {
    errmsg(p, errcode);
    return errcode;
  }
  writelogo(p);

  // Allocate memory for project's data arrays
  writewin(p->viewprog, FMT100);
  ERRCODE(netsize(p));
  ERRCODE(allocdata(p));

  // Read input data
  ERRCODE(getdata(p));

  // Free temporary linked lists used for Patterns & Curves
  freeTmplist(p->parser.Patlist);
  freeTmplist(p->parser.Curvelist);

  // If using previously saved hydraulics file then open it
  if (p->outfile.Hydflag == USE) ERRCODE(openhydfile(p));

  // Write input summary to report file
  if (!errcode)
  {
    if (p->report.Summaryflag) writesummary(p);
    writetime(p, FMT104);
    p->Openflag = TRUE;
  }
  else errmsg(p, errcode);
  return errcode;
}

int DLLEXPORT EN_saveinpfile(EN_Project p, const char *filename)
/*----------------------------------------------------------------
 **  Input:   filename = name of file to which project is saved
 **  Output:  none
 **  Returns: error code
 **  Purpose: saves project to an EPANET formatted file
 **----------------------------------------------------------------
 */
{
  if (!p->Openflag) return 102;
  return saveinpfile(p, filename);
}

int DLLEXPORT EN_close(EN_Project p)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: frees all memory & files used by EPANET
 **----------------------------------------------------------------
 */
{
    Outfile *out;

    // Free all project data
    if (p->Openflag) writetime(p, FMT105);
    freedata(p);

    // Close output file
    out = &p->outfile;
    if (out->TmpOutFile != out->OutFile)
    {
        if (out->TmpOutFile != NULL) fclose(out->TmpOutFile);
        out->TmpOutFile = NULL;
    }
    if (out->OutFile != NULL)
    {
        fclose(out->OutFile);
        out->OutFile = NULL;
    }

    // Close input file
    if (p->parser.InFile != NULL)
    {
        fclose(p->parser.InFile);
        p->parser.InFile = NULL;
    }

    // Close report file
    if (p->report.RptFile != NULL && p->report.RptFile != stdout)
    {
        fclose(p->report.RptFile);
        p->report.RptFile = NULL;
    }

    // Close hydraulics file
    if (out->HydFile != NULL)
    {
        fclose(out->HydFile);
        out->HydFile = NULL;
    }

    // Reset system flags
    p->Openflag = FALSE;
    p->hydraul.OpenHflag = FALSE;
    p->outfile.SaveHflag = FALSE;
    p->quality.OpenQflag = FALSE;
    p->outfile.SaveQflag = FALSE;
    return 0;
}

/********************************************************************

    Hydraulic Analysis Functions

 ********************************************************************/

int DLLEXPORT EN_solveH(EN_Project p)
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

    // Open hydraulics solver
    errcode = EN_openH(p);
    if (!errcode)
    {
        // Initialize hydraulics
        errcode = EN_initH(p, EN_SAVE);

        // Analyze each hydraulic time period
        if (!errcode) do
        {
            // Display progress message
            sprintf(p->Msg, "%-10s",
                    clocktime(p->report.Atime, p->times.Htime));
            sprintf(p->Msg, FMT101, p->report.Atime);
            writewin(p->viewprog, p->Msg);

            // Solve for hydraulics & advance to next time period
            tstep = 0;
            ERRCODE(EN_runH(p, &t));
            ERRCODE(EN_nextH(p, &tstep));
        } while (tstep > 0);
    }

    // Close hydraulics solver
    EN_closeH(p);
    errcode = MAX(errcode, p->Warnflag);
    return errcode;
}

int DLLEXPORT EN_saveH(EN_Project p)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: saves hydraulic results to binary file
 **
 **  Must be called before EN_report() if no WQ simulation made.
 **  Should not be called if EN_solveQ() will be used.
 **----------------------------------------------------------------
 */
{
    char tmpflag;
    int errcode;

    // Check if hydraulic results exist
    if (!p->outfile.SaveHflag) return 104;

    // Temporarily turn off WQ analysis
    tmpflag = p->quality.Qualflag;
    p->quality.Qualflag = NONE;

    // Call WQ solver to simply transfer results from Hydraulics file
    // to Output file at fixed length reporting time intervals
    errcode = EN_solveQ(p);

    // Restore WQ analysis option
    p->quality.Qualflag = tmpflag;
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_openH(EN_Project p)
/*----------------------------------------------------------------
 **  Input:   none
 **  Output:  none
 **  Returns: error code
 **  Purpose: opens EPANET's hydraulic solver
 **----------------------------------------------------------------
 */
{
    int errcode = 0;

    // Check that input data exists
    p->hydraul.OpenHflag = FALSE;
    p->outfile.SaveHflag = FALSE;
    if (!p->Openflag) return 102;

    // Check that previously saved hydraulics file not in use
    if (p->outfile.Hydflag == USE) return 107;

    // Open hydraulics solver
    ERRCODE(openhyd(p));
    if (!errcode) p->hydraul.OpenHflag = TRUE;
    else errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_initH(EN_Project p, int flag)
/*----------------------------------------------------------------
 **  Input:   flag = 2-digit flag where 1st (left) digit indicates
 **                  if link flows should be re-initialized (1) or
 **                  not (0) and 2nd digit indicates if hydraulic
 **                  results should be saved to file (1) or not (0)
 **  Output:  none
 **  Returns: error code
 **  Purpose: initializes EPANET's hydraulic solver
 **----------------------------------------------------------------
 */
{
    int errcode = 0;
    int sflag, fflag;

    // Reset status flags
    p->outfile.SaveHflag = FALSE;
    p->Warnflag = FALSE;

    // Get values of save-to-file flag and reinitialize-flows flag
    fflag = flag / EN_INITFLOW;
    sflag = flag - fflag * EN_INITFLOW;

    // Check that hydraulics solver was opened
    if (!p->hydraul.OpenHflag) return 103;

    // Open hydraulics file if requested
    p->outfile.Saveflag = FALSE;
    if (sflag > 0)
    {
        errcode = openhydfile(p);
        if (!errcode) p->outfile.Saveflag = TRUE;
        else
        {
            errmsg(p, errcode);
            return errcode;
        }
    }

    // Initialize hydraulics solver
    inithyd(p, fflag);
    if (p->report.Statflag > 0) writeheader(p, STATHDR, 0);
    return errcode;
}

int DLLEXPORT EN_runH(EN_Project p, long *t)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  t = elapsed time (sec)
**  Returns: error code
**  Purpose: solves network hydraulics at current time point
**----------------------------------------------------------------
*/
{
    int errcode;

    *t = 0;
    if (!p->hydraul.OpenHflag) return 103;
    errcode = runhyd(p, t);
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_nextH(EN_Project p, long *tstep)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  tstep = next hydraulic time step to take (sec)
**  Returns: error code
**  Purpose: determines the time step until the next hydraulic event
**----------------------------------------------------------------
*/
{
    int errcode;

    *tstep = 0;
    if (!p->hydraul.OpenHflag) return 103;
    errcode = nexthyd(p, tstep);
    if (errcode) errmsg(p, errcode);
    else if (p->outfile.Saveflag && *tstep == 0) p->outfile.SaveHflag = TRUE;
    return errcode;
}

int DLLEXPORT EN_closeH(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: closes EPANET's hydraulic solver
**----------------------------------------------------------------
*/
{
  if (!p->Openflag) return 102;
  if (p->hydraul.OpenHflag) closehyd(p);
  p->hydraul.OpenHflag = FALSE;
  return 0;
}

int DLLEXPORT EN_savehydfile(EN_Project p, char *filename)
/*----------------------------------------------------------------
**  Input:   filename = name of file to which hydraulic results are saved
**  Output:  none
**  Returns: error code
**  Purpose: saves results from a scratch hydraulics file to a
**           permanent one.
**----------------------------------------------------------------
*/
{
    FILE *f;
    FILE *HydFile;
    int c;

    // Check that hydraulics results exist
    if (p->outfile.HydFile == NULL || !p->outfile.SaveHflag) return 104;

    // Open the permanent hydraulics file
    if ((f = fopen(filename, "w+b")) == NULL) return 305;

    // Copy from the scratch file to f
    HydFile = p->outfile.HydFile;
    fseek(HydFile, 0, SEEK_SET);
    while ((c = fgetc(HydFile)) != EOF) fputc(c, f);
    fclose(f);
    return 0;
}

int DLLEXPORT EN_usehydfile(EN_Project p, char *filename)
/*----------------------------------------------------------------
**  Input:   filename = name of previously saved hydraulics file
**  Output:  none
**  Returns: error code
**  Purpose: uses contents of a previous saved hydraulics file to
**           run a water quality analysis.
**----------------------------------------------------------------
*/
{
    int errcode;

    // Check that project was opened & hydraulic solver is closed
    if (!p->Openflag) return 102;
    if (p->hydraul.OpenHflag) return 108;

    // Try to open hydraulics file
    strncpy(p->outfile.HydFname, filename, MAXFNAME);
    p->outfile.Hydflag = USE;
    p->outfile.SaveHflag = TRUE;
    errcode = openhydfile(p);

    // If error, then reset flags
    if (errcode)
    {
        strcpy(p->outfile.HydFname, "");
        p->outfile.Hydflag = SCRATCH;
        p->outfile.SaveHflag = FALSE;
    }
    return errcode;
}

/********************************************************************

    Water Quality Analysis Functions

 ********************************************************************/

int DLLEXPORT EN_solveQ(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: solves for network water quality in all time periods
**----------------------------------------------------------------
*/
{
    int errcode;
    long t, tstep;

    // Open WQ solver
    errcode = EN_openQ(p);
    if (!errcode)
    {
        // Initialize WQ solver
        errcode = EN_initQ(p, EN_SAVE);
        if (!p->quality.Qualflag) writewin(p->viewprog, FMT106);

        // Analyze each hydraulic period
        if (!errcode) do
        {
            // Display progress message
            sprintf(p->Msg, "%-10s",
                    clocktime(p->report.Atime, p->times.Htime));
            if (p->quality.Qualflag)
            {
                sprintf(p->Msg, FMT102, p->report.Atime);
                writewin(p->viewprog, p->Msg);
            }

            // Retrieve current hydraulic results & update water quality
            // to start of next time period
            tstep = 0;
            ERRCODE(EN_runQ(p, &t));
            ERRCODE(EN_nextQ(p, &tstep));
        } while (tstep > 0);
    }

    // Close WQ solver
    EN_closeQ(p);
    return errcode;
}

int DLLEXPORT EN_openQ(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: opens EPANET's water quality solver
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    // Check that hydraulics results exist
    p->quality.OpenQflag = FALSE;
    p->outfile.SaveQflag = FALSE;
    if (!p->Openflag) return 102;
    if (!p->hydraul.OpenHflag && !p->outfile.SaveHflag) return 104;

    // Open water quality solver
    ERRCODE(openqual(p));
    if (!errcode) p->quality.OpenQflag = TRUE;
    else errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_initQ(EN_Project p, int saveflag)
/*----------------------------------------------------------------
**  Input:   saveflag = flag indicating if results should be saved
**                      to the binary output file or not
**  Output:  none
**  Returns: error code
**  Purpose: initializes the water quality solver
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    if (!p->quality.OpenQflag) return 105;
    initqual(p);
    p->outfile.SaveQflag = FALSE;
    p->outfile.Saveflag = FALSE;
    if (saveflag)
    {
        errcode = openoutfile(p);
        if (!errcode) p->outfile.Saveflag = TRUE;
    }
    return errcode;
}

int DLLEXPORT EN_runQ(EN_Project p, long *t)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  t = current simulation time (sec)
**  Returns: error code
**  Purpose: retrieves current hydraulic results (at time t)
**           and saves current results to file.
**----------------------------------------------------------------
*/
{
    int errcode;

    *t = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = runqual(p, t);
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_nextQ(EN_Project p, long *tstep)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  tstep = time step over which water quality is updated (sec)
**  Returns: error code
**  Purpose: updates water quality throughout the network until
**           next hydraulic event occurs.
**----------------------------------------------------------------
*/
{
    int errcode;

    *tstep = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = nextqual(p, tstep);
    if (!errcode && p->outfile.Saveflag && *tstep == 0)
    {
        p->outfile.SaveQflag = TRUE;
    }
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_stepQ(EN_Project p, long *tleft)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  tleft = amount of simulation time remaining (sec)
**  Returns: error code
**  Purpose: updates water quality throughout the network over
**           fixed time step.
**----------------------------------------------------------------
*/
{
    int errcode;

    *tleft = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = stepqual(p, tleft);
    if (!errcode && p->outfile.Saveflag && *tleft == 0)
    {
        p->outfile.SaveQflag = TRUE;
    }
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_closeQ(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: closes EPANET's water quality solver
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    closequal(p);
    p->quality.OpenQflag = FALSE;
    return 0;
}

/********************************************************************

    Reporting Functions

 ********************************************************************/

int DLLEXPORT EN_writeline(EN_Project p, char *line)
/*----------------------------------------------------------------
**  Input:   line = line of text
**  Output:  none
**  Returns: error code
**  Purpose: write a line of text to the project's report file
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    writeline(p, line);
    return 0;
}

int DLLEXPORT EN_report(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: writes formatted simulation results to the project's
**           report file
**----------------------------------------------------------------
*/
{
    int errcode;

    // Check if results have been saved to binary output file
    if (!p->outfile.SaveQflag) return 106;
    writewin(p->viewprog, FMT103);

    // Write the formatted report
    errcode = writereport(p);
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_resetreport(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: resets reporting options to their default values
**----------------------------------------------------------------
*/
{
    int i;

    if (!p->Openflag) return 102;
    initreport(&p->report);
    for (i = 1; i <= p->network.Nnodes; i++)
    {
        p->network.Node[i].Rpt = 0;
    }
    for (i = 1; i <= p->network.Nlinks; i++)
    {
        p->network.Link[i].Rpt = 0;
    }
    return 0;
}

int DLLEXPORT EN_setreport(EN_Project p, char *s)
/*----------------------------------------------------------------
**  Input:   s = a report formatting command
**  Output:  none
**  Returns: error code
**  Purpose: sets a specific set of reporting options
**----------------------------------------------------------------
*/
{
    char s1[MAXLINE + 1];

    if (!p->Openflag) return 102;
    if (strlen(s) >= MAXLINE) return 250;
    strcpy(s1, s);
    strcat(s1, "\n");
    if (setreport(p, s1) > 0) return 250;
    else return 0;
}

int DLLEXPORT EN_setstatusreport(EN_Project p, int code)
/*----------------------------------------------------------------
**  Input:   code = level of reporting to use (see EN_StatusReport)
**  Output:  none
**  Returns: error code
**  Purpose: sets the level of hydraulic status reporting
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    if (code >= EN_NO_REPORT && code <= EN_FULL_REPORT)
    {
        p->report.Statflag = (char)code;
    }
    else errcode = 202;
    return errcode;
}

int DLLEXPORT EN_getversion(int *v)
/*----------------------------------------------------------------
**  Input:    none
**  Output:   v = version number of the source code
**  Returns:  error code (should always be 0)
**  Purpose:  retrieves a number assigned to the most recent
**            update of the source code. This number, set by the
**            constant CODEVERSION found in TYPES.H, is to be
**            interpreted with implied decimals, i.e.,
**            "20100" == "2(.)01(.)00"
**----------------------------------------------------------------
*/
{
    *v = CODEVERSION;
    return 0;
}

int DLLEXPORT EN_getcount(EN_Project p, EN_CountType code, int *count)
/*----------------------------------------------------------------
**  Input:   code = type of component to count (see EN_CountType)
**  Output:  count = number of components of specified type
**  Returns: error code
**  Purpose: Retrieves number of network components of a given type
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    *count = 0;
    if (!p->Openflag) return 102;
    switch (code)
    {
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
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_geterror(int errcode, char *errmsg, int n)
/*----------------------------------------------------------------
**  Input:   errcode = an error or warnng code
**           n = maximum characters that errmsg can hold
**  Output:  errmsg = text of error/warning message
**  Returns: error code
**  Purpose: retrieves the text of the message associated with
**           a particular error/warning code
**----------------------------------------------------------------
*/
{
    char newMsg[MAXMSG + 1];

    switch (errcode)
    {
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

int DLLEXPORT EN_getstatistic(EN_Project p, int code, EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   code = simulation statistic (see EN_AnalysisStatistic)
**  Output:  value = simulation analysis statistic value
**  Returns: error code
**  Purpose: retrieves the value of a simulation analysis statistic
**----------------------------------------------------------------
*/
{
    switch (code)
    {
    case EN_ITERATIONS:
        *value = (EN_API_FLOAT_TYPE)p->hydraul.Iterations;
        break;
    case EN_RELATIVEERROR:
        *value = (EN_API_FLOAT_TYPE)p->hydraul.RelativeError;
        break;
    case EN_MAXHEADERROR:
        *value = (EN_API_FLOAT_TYPE)(p->hydraul.MaxHeadError * p->Ucf[HEAD]);
        break;
    case EN_MAXFLOWCHANGE:
        *value = (EN_API_FLOAT_TYPE)(p->hydraul.MaxFlowChange * p->Ucf[FLOW]);
        break;
    case EN_MASSBALANCE:
        *value = (EN_API_FLOAT_TYPE)(p->quality.MassBalance.ratio);
        break;
    default:
        break;
    }
    return 0;
}

/********************************************************************

    Analysis Options Functions

********************************************************************/

int DLLEXPORT EN_getoption(EN_Project p, EN_Option code,
                           EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   code = an analysis option code (see EN_Option)
**  Output:  value = analysis option value
**  Returns: error code
**  Purpose: retrieves the value of an analysis option
**----------------------------------------------------------------
*/
{
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    double *Ucf = p->Ucf;
    double v = 0.0;

    *value = 0.0;
    if (!p->Openflag) return 102;
    switch (code)
    {
    case EN_TRIALS:
        v = (double)hyd->MaxIter;
        break;
    case EN_ACCURACY:
        v = hyd->Hacc;
        break;
    case EN_TOLERANCE:
        v = qual->Ctol * Ucf[QUALITY];
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
        return 251;
    }
    *value = (EN_API_FLOAT_TYPE)v;
    return 0;
}

int DLLEXPORT EN_setoption(EN_Project p, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
**  Input:   code  = analysis option code (see EN_Option)
**           v = analysis option value
**  Output:  none
**  Returns: error code
**  Purpose: sets the value for an analysis option
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    Snode *node;
    Pdemand demand;
    const int Njuncs = net->Njuncs;
    double *Ucf = p->Ucf;
    int i, j;
    int tmpPat, error;
    char tmpId[MAXID + 1];
    double Ke, n, ucf, value = v;

    if (!p->Openflag) return 102;
    switch (code)
    {
    case EN_TRIALS:
        if (value < 1.0) return 202;
        hyd->MaxIter = (int)value;
        break;

    case EN_ACCURACY:
        if (value < 1.e-5 || value > 1.e-1) return 202;
        hyd->Hacc = value;
        break;

    case EN_TOLERANCE:
        if (value < 0.0) return 202;
        qual->Ctol = value / Ucf[QUALITY];
        break;

    case EN_EMITEXPON:
        if (value <= 0.0) return 202;
        n = 1.0 / value;
        ucf = pow(Ucf[FLOW], n) / Ucf[PRESSURE];
        for (i = 1; i <= Njuncs; i++)
        {
            j = EN_getnodevalue(p, i, EN_EMITTER, &v);
            Ke = v;
            if (j == 0 && Ke > 0.0) net->Node[i].Ke = ucf / pow(Ke, n);
        }
        hyd->Qexp = n;
        break;

    case EN_DEMANDMULT:
        if (value <= 0.0) return 202;
        hyd->Dmult = value;
        break;

    case EN_HEADERROR:
        if (value < 0.0) return 202;
        hyd->HeadErrorLimit = value / Ucf[HEAD];
        break;

    case EN_FLOWCHANGE:
        if (value < 0.0) return 202;
        hyd->FlowChangeLimit = value / Ucf[FLOW];
        break;

    case EN_DEMANDDEFPAT:
        //check that the pattern exists or is set to zero to delete the default pattern
        if (value < 0 || value > net->Npats) return 205;
        tmpPat = hyd->DefPat;
        //get the new pattern ID
        if (value == 0)
        {
            strncpy(tmpId, p->parser.DefPatID, MAXID);
        }
        else
        {
            error = EN_getpatternid(p, (int)value, tmpId);
            if (error != 0) return error;
        }
        // replace node patterns with default pattern
        for (i = 1; i <= net->Nnodes; i++)
        {
            node = &net->Node[i];
            for (demand = node->D; demand != NULL; demand = demand->next)
            {
                if (demand->Pat == tmpPat)
                {
                    demand->Pat = (int)value;
                    strcpy(demand->Name, "");
                }
            }
        }
        strncpy(p->parser.DefPatID, tmpId, MAXID);
        hyd->DefPat = (int)value;
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_getflowunits(EN_Project p, int *code)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  code = flow units code (see EN_FlowUnits)
**  Returns: error code
**  Purpose: retrieves the flow units used by a project
**----------------------------------------------------------------
*/
{
    *code = -1;
    if (!p->Openflag) return 102;
    *code = p->parser.Flowflag;
    return 0;
}

int DLLEXPORT EN_setflowunits(EN_Project p, int code)
/*----------------------------------------------------------------
**  Input:   code = flow units code (see EN_FlowUnits)
**  Output:  none
**  Returns: error code
**  Purpose: sets the flow units used by a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i, j;
    double qfactor, vfactor, hfactor, efactor, xfactor, yfactor;
    double *Ucf = p->Ucf;

    if (!p->Openflag) return 102;

    // Determine unit system based on flow units
    qfactor = Ucf[FLOW];
    vfactor = Ucf[VOLUME];
    hfactor = Ucf[HEAD];
    efactor = Ucf[ELEV];

    p->parser.Flowflag = code;
    switch (code)
    {
    case LPS:
    case LPM:
    case MLD:
    case CMH:
    case CMD:
        p->parser.Unitsflag = SI;
        break;
    default:
        p->parser.Unitsflag = US;
        break;
    }

    // Revise pressure units depending on flow units
    if (p->parser.Unitsflag != SI) p->parser.Pressflag = PSI;
    else if (p->parser.Pressflag == PSI) p->parser.Pressflag = METERS;
    initunits(p);

    //update curves
    for (i = 1; i <= net->Ncurves; i++)
    {
        switch (net->Curve[i].Type)
        {
        case V_CURVE:
            xfactor = efactor / Ucf[ELEV];
            yfactor = vfactor / Ucf[VOLUME];
            break;
        case H_CURVE:
        case P_CURVE:
            xfactor = qfactor / Ucf[FLOW];
            yfactor = hfactor / Ucf[HEAD];
            break;
        case E_CURVE:
            xfactor = qfactor / Ucf[FLOW];
            yfactor = 1;
            break;
        default:
            xfactor = 1;
            yfactor = 1;
        }

        for (j = 0; j < net->Curve[i].Npts; j++)
        {
            net->Curve[i].X[j] = net->Curve[i].X[j] / xfactor;
            net->Curve[i].Y[j] = net->Curve[i].Y[j] / yfactor;
        }
    }
    return 0;
}

int DLLEXPORT EN_gettimeparam(EN_Project p, int code, long *value)
/*----------------------------------------------------------------
**  Input:   code = time parameter code (see EN_TimeProperty)
**  Output:  value = time parameter value
**  Returns: error code
**  Purpose: retrieves the value of a time parameter
**----------------------------------------------------------------
*/
{
    Report *rpt = &p->report;
    Times  *time = &p->times;

    int i;

    *value = 0;
    if (!p->Openflag) return 102;
    if (code < EN_DURATION || code > EN_NEXTEVENTIDX) return 251;
    switch (code)
    {
    case EN_DURATION:
        *value = time->Dur;
        break;
    case EN_HYDSTEP:
        *value = time->Hstep;
        break;
    case EN_QUALSTEP:
        *value = time->Qstep;
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
        *value = rpt->Tstatflag;
        break;
    case EN_RULESTEP:
        *value = time->Rulestep;
        break;
    case EN_PERIODS:
        *value = rpt->Nperiods;
        break;
    case EN_STARTTIME:
        *value = time->Tstart;
        break;
    case EN_HTIME:
        *value = time->Htime;
        break;
    case EN_NEXTEVENT:
        *value = time->Hstep; // find the lesser of the hydraulic time step length,
                              // or the time to next full/empty tank
        tanktimestep(p, value);
        break;
    case EN_NEXTEVENTIDX:
        *value = time->Hstep;
        i = tanktimestep(p, value);
        *value = i;
        break;
    }
    return 0;
}

int DLLEXPORT EN_settimeparam(EN_Project p, int code, long value)
/*----------------------------------------------------------------
**  Input:   code = time parameter code (see EN_TimeProperty)
**           value = time parameter value
**  Output:  none
**  Returns: error code
**  Purpose: sets the value of a time parameter
**----------------------------------------------------------------
*/
{
    Report *rpt = &p->report;
    Times  *time = &p->times;

    if (!p->Openflag) return 102;
    if (value < 0) return 202;
    switch (code)
    {
    case EN_DURATION:
        time->Dur = value;
        if (time->Rstart > time->Dur) time->Rstart = 0;
        break;

    case EN_HYDSTEP:
        if (value == 0) return 202;
        time->Hstep = value;
        time->Hstep = MIN(time->Pstep, time->Hstep);
        time->Hstep = MIN(time->Rstep, time->Hstep);
        time->Qstep = MIN(time->Qstep, time->Hstep);
        break;

    case EN_QUALSTEP:
        if (value == 0) return 202;
        time->Qstep = value;
        time->Qstep = MIN(time->Qstep, time->Hstep);
        break;

    case EN_PATTERNSTEP:
        if (value == 0) return 202;
        time->Pstep = value;
        if (time->Hstep > time->Pstep) time->Hstep = time->Pstep;
        break;

    case EN_PATTERNSTART:
        time->Pstart = value;
        break;

    case EN_REPORTSTEP:
        if (value == 0) return 202;
        time->Rstep = value;
        if (time->Hstep > time->Rstep) time->Hstep = time->Rstep;
        break;

    case EN_REPORTSTART:
        if (time->Rstart > time->Dur) return 202;
        time->Rstart = value;
        break;

    case EN_RULESTEP:
        if (value == 0) return 202;
        time->Rulestep = value;
        time->Rulestep = MIN(time->Rulestep, time->Hstep);
        break;

    case EN_STATISTIC:
        if (value > RANGE) return 202;
        rpt->Tstatflag = (char)value;
        break;

    case EN_HTIME:
        time->Htime = value;
        break;

    case EN_QTIME:
        time->Qtime = value;
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_getqualinfo(EN_Project p, int *qualcode, char *chemname,
                             char *chemunits, int *tracenode)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  qualcode = quality analysis code (see EN_QualityType)
**           chemname = name of chemical species
**           chemunits = concentration units
**           tracenode = index of node being traced
**  Returns: error code
**  Purpose: retrieves water quality analysis options
**----------------------------------------------------------------
*/
{
    EN_getqualtype(p, qualcode, tracenode);
    if (p->quality.Qualflag == TRACE)
    {
        strncpy(chemname, "", MAXID);
        strncpy(chemunits, "dimensionless", MAXID);
    }
    else
    {
        strncpy(chemname, p->quality.ChemName, MAXID);
        strncpy(chemunits, p->quality.ChemUnits, MAXID);
    }
    return 0;
}

int DLLEXPORT EN_getqualtype(EN_Project p, int *qualcode, int *tracenode)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  qualcode = quality analysis code (see EN_QualityType)
**           tracenode = index of node being traced
**  Output:  none
**  Returns: error code
**  Purpose: retrieves type of quality analysis being made
**----------------------------------------------------------------
*/
{
    *tracenode = 0;
    if (!p->Openflag) return 102;
    *qualcode = p->quality.Qualflag;
    if (p->quality.Qualflag == TRACE) *tracenode = p->quality.TraceNode;
    return 0;
}

int DLLEXPORT EN_setqualtype(EN_Project p, int qualcode, char *chemname,
                             char *chemunits, char *tracenode)
/*----------------------------------------------------------------
**  Input:   qualcode = quality analysis code (see EN_QualityType)
**           chemname = name of chemical species
**           chemunits = concentration units
**           tracenode = index of node being traced
**  Output:  none
**  Returns: error code
**  Purpose: sets water quality analysis options
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Report  *rpt = &p->report;
    Quality *qual = &p->quality;

    double *Ucf = p->Ucf;
    int i;
    double ccf = 1.0;

    if (!p->Openflag) return 102;
    if (qualcode < EN_NONE || qualcode > EN_TRACE) return 251;
    qual->Qualflag = (char)qualcode;
    qual->Ctol *= Ucf[QUALITY];
    if (qual->Qualflag == CHEM) // Chemical analysis
    {
        strncpy(qual->ChemName, chemname, MAXID);
        strncpy(qual->ChemUnits, chemunits, MAXID);
        strncpy(rpt->Field[QUALITY].Units, qual->ChemUnits, MAXID);
        strncpy(rpt->Field[REACTRATE].Units, qual->ChemUnits, MAXID);
        strcat(rpt->Field[REACTRATE].Units, t_PERDAY);
        ccf = 1.0 / LperFT3;
    }
    if (qual->Qualflag == TRACE) // Source trace analysis
    {
        qual->TraceNode = findnode(net, tracenode);
        if (qual->TraceNode == 0) return 203;
        strncpy(qual->ChemName, u_PERCENT, MAXID);
        strncpy(qual->ChemUnits, tracenode, MAXID);
        strcpy(rpt->Field[QUALITY].Units, u_PERCENT);
    }
    if (qual->Qualflag == AGE) // Water age analysis
    {
        strncpy(qual->ChemName, w_AGE, MAXID);
        strncpy(qual->ChemUnits, u_HOURS, MAXID);
        strcpy(rpt->Field[QUALITY].Units, u_HOURS);
    }

    // When changing from CHEM to AGE or TRACE, nodes initial quality
    // values must be returned to their original ones
    if ((qual->Qualflag == AGE || qual->Qualflag == TRACE) & (Ucf[QUALITY] != 1))
    {
        for (i = 1; i <= p->network.Nnodes; i++)
        {
            p->network.Node[i].C0 *= Ucf[QUALITY];
        }
    }

    Ucf[QUALITY] = ccf;
    Ucf[LINKQUAL] = ccf;
    Ucf[REACTRATE] = ccf;
    qual->Ctol /= Ucf[QUALITY];
    return 0;
}

/********************************************************************

    Node Functions

********************************************************************/

int DLLEXPORT EN_addnode(EN_Project p, char *id, EN_NodeType nodeType)
/*----------------------------------------------------------------
**  Input:   id = node ID name
**           nodeType = type of node (see EN_NodeType)
**  Output:  none
**  Returns: error code
**  Purpose: adds a new node to a project
**----------------------------------------------------------------
*/
{
    Network  *net = &p->network;
    Hydraul  *hyd = &p->hydraul;
    Quality  *qual = &p->quality;

    int i, nIdx;
    int index;
    int size;
    struct Sdemand *demand;
    Stank *tank;
    Snode *node;
    Scontrol *control;

    // Check if a node with same id already exists
    if (!p->Openflag) return 102;
    if (EN_getnodeindex(p, id, &i) == 0) return 215;

    // Check that id name is not too long
    if (strlen(id) > MAXID) return 250;

    // Grow node-related arrays to accomodate the new node
    size = (net->Nnodes + 2) * sizeof(Snode);
    net->Node = (Snode *)realloc(net->Node, size);
    size = (net->Nnodes + 2) * sizeof(double);
    hyd->NodeDemand = (double *)realloc(hyd->NodeDemand, size);
    qual->NodeQual = (double *)realloc(qual->NodeQual, size);
    hyd->NodeHead = (double *)realloc(hyd->NodeHead, size);

    // Actions taken when a new Junction is added
    if (nodeType == EN_JUNCTION)
    {
        net->Njuncs++;
        nIdx = net->Njuncs;
        node = &net->Node[nIdx];

        demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
        demand->Base = 0.0;
        demand->Pat = hyd->DefPat; // Use default pattern
        strcpy(demand->Name, "");
        demand->next = NULL;
        node->D = demand;

        // shift rest of Node array
        for (index = net->Nnodes; index >= net->Njuncs; index--)
        {
            hashtable_update(net->NodeHashTable, net->Node[index].ID, index + 1);
            net->Node[index + 1] = net->Node[index];
        }
        // shift indices of Tank array
        for (index = 1; index <= net->Ntanks; index++)
        {
            net->Tank[index].Node += 1;
        }

        // shift indices of Links, if necessary
        for (index = 1; index <= net->Nlinks; index++)
        {
            if (net->Link[index].N1 > net->Njuncs - 1) net->Link[index].N1 += 1;
            if (net->Link[index].N2 > net->Njuncs - 1) net->Link[index].N2 += 1;
        }

        // shift indices of tanks/reservoir nodes in controls
        for (index = 1; index <= net->Ncontrols; ++index)
        {
            control = &net->Control[index];
            if (control->Node > net->Njuncs - 1) control->Node += 1;
        }

        // adjust indices of tanks/reservoirs in Rule premises (see RULES.C)
        adjusttankrules(p);
    }

    // Actions taken when a new Tank/Reservoir is added
    else
    {
        nIdx = net->Nnodes + 1;
        node = &net->Node[nIdx];
        net->Ntanks++;

        // resize tanks array
        net->Tank = (Stank *)realloc(net->Tank, (net->Ntanks + 1) * sizeof(Stank));
        tank = &net->Tank[net->Ntanks];

        // set default values for new tank or reservoir
        tank->Node = nIdx;
        tank->Pat = 0;
        if (nodeType == EN_TANK) tank->A = 1.0;
        else tank->A = 0;
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
    strncpy(node->ID, id, MAXID);

    // set default values for new node
    node->El = 0;
    node->S = NULL;
    node->C0 = 0;
    node->Ke = 0;
    node->Rpt = 0;
    node->X = MISSING;
    node->Y = MISSING;
    strcpy(node->Comment, "");

    // Insert new node into hash table
    hashtable_insert(net->NodeHashTable, node->ID, nIdx);
    return 0;
}

int DLLEXPORT EN_deletenode(EN_Project p, int index, int actionCode)
/*----------------------------------------------------------------
**  Input:   index  = index of the node to delete
**           actionCode = how to treat controls that contain the link
**           or its incident links:
**           EN_UNCONDITIONAL deletes all such controls plus the node,
**           EN_CONDITIONAL does not delete the node if it or any of
**           its links appear in a control and returns an error code
**  Output:  none
**  Returns: error code
**  Purpose: deletes a node from a project.
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i, nodeType, tankindex, numControls = 0;
    Snode *node;
    Pdemand demand, nextdemand;
    Psource source;

    // Check that node exists
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nnodes) return 204;
    if (actionCode < EN_UNCONDITIONAL || actionCode > EN_CONDITIONAL) return 251;

    // Can't delete a water quality trace node
    if (index == p->quality.TraceNode) return 260;

    // Count number of simple & rule-based controls that contain the node
    if (actionCode == EN_CONDITIONAL)
    {
        actionCode = incontrols(p, NODE, index);
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].N1 == index ||
                net->Link[i].N2 == index)  actionCode += incontrols(p, LINK, i);
        }
        if (actionCode > 0) return 261;
    }

    // Get a reference to the node & its type
    node = &net->Node[index];
    EN_getnodetype(p, index, &nodeType);

    // Remove node from its hash table
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

    // Shift position of higher entries in Node & Coord arrays down one
    for (i = index; i <= net->Nnodes - 1; i++)
    {
        net->Node[i] = net->Node[i + 1];
        // ... update node's entry in the hash table
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
            net->Link[i].N2 == index)  EN_deletelink(p, i, EN_UNCONDITIONAL);
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
        if (net->Control[i].Node == index) EN_deletecontrol(p, i);
    }

    // Adjust higher numbered link indices in remaining controls
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (net->Control[i].Node > index) net->Control[i].Node--;
    }

    // Make necessary adjustments to rule-based controls
    adjustrules(p, EN_R_NODE, index);

    // Reduce counts of node types
    if (nodeType == EN_JUNCTION) net->Njuncs--;
    else net->Ntanks--;
    net->Nnodes--;
    return 0;
}

int DLLEXPORT EN_getnodeindex(EN_Project p, char *id, int *index)
/*----------------------------------------------------------------
**  Input:   id = node ID name
**  Output:  index = node index
**  Returns: error code
**  Purpose: retrieves the index of a node
**----------------------------------------------------------------
*/
{
    *index = 0;
    if (!p->Openflag) return 102;
    *index = findnode(&p->network, id);
    if (*index == 0) return 203;
    else return 0;
}

int DLLEXPORT EN_getnodeid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  id = node ID name
**  Returns: error code
**  Purpose: retrieves the name of a node
**----------------------------------------------------------------
*/
{
    strcpy(id, "");
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nnodes) return 203;
    strcpy(id, p->network.Node[index].ID);
    return 0;
}

int DLLEXPORT EN_setnodeid(EN_Project p, int index, char *newid)
/*----------------------------------------------------------------
**  Input:   index = node index
**           newid = new node ID name
**  Output:  none
**  Returns: error code
**  Purpose: sets the ID name of a node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    size_t n;

    // Check for valid arguments
    if (index <= 0 || index > net->Nnodes) return 203;
    n = strlen(newid);
    if (n < 1 || n > MAXID) return 209;
    if (strcspn(newid, " ;") < n) return 209;

    // Check if another node with same name exists
    if (hashtable_find(net->NodeHashTable, newid) > 0) return 215;

    // Replace the existing node ID with the new value
    hashtable_delete(net->NodeHashTable, net->Node[index].ID);
    strncpy(net->Node[index].ID, newid, MAXID);
    hashtable_insert(net->NodeHashTable, net->Node[index].ID, index);
    return 0;
}

int DLLEXPORT EN_getnodetype(EN_Project p, int index, int *code)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  code  = node type (see EN_NodeType)
**  Returns: error code
**  Purpose: retrieves the type of a node
**----------------------------------------------------------------
*/
{
    *code = -1;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nnodes) return 203;
    if (index <= p->network.Njuncs) *code = EN_JUNCTION;
    else
    {
        if (p->network.Tank[index - p->network.Njuncs].A == 0.0) *code = EN_RESERVOIR;
        else *code = EN_TANK;
    }
    return 0;
}

int DLLEXPORT EN_getnodevalue(EN_Project p, int index, int code, EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   index = node index
**           code = node property code (see EN_NodeProperty)
**  Output:  value = node property value
**  Returns: error code
**  Purpose: retrieves a property value for a node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    double v = 0.0;
    Pdemand demand;
    Psource source;

    Snode *Node = net->Node;
    Stank *Tank = net->Tank;

    int nJuncs = net->Njuncs;

    double *Ucf = p->Ucf;
    double *NodeDemand = hyd->NodeDemand;
    double *NodeHead = hyd->NodeHead;
    double *NodeQual = qual->NodeQual;

    // Check for valid arguments
    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nnodes) return 203;

    // Retrieve requested property
    switch (code)
    {
    case EN_ELEVATION:
        v = Node[index].El * Ucf[ELEV];
        break;

    case EN_BASEDEMAND:
        v = 0.0;
        // NOTE: primary demand category is last on demand list
        if (index <= nJuncs)
        {
            for (demand = Node[index].D; demand != NULL; demand = demand->next)
            {
                v = (demand->Base);
            }
        }
        v *= Ucf[FLOW];
        break;

    case EN_PATTERN:
        v = 0.0;
        // NOTE: primary demand category is last on demand list
        if (index <= nJuncs)
        {
            for (demand = Node[index].D; demand != NULL; demand = demand->next)
            {
                v = (double)(demand->Pat);
            }
        }
        else v = (double)(Tank[index - nJuncs].Pat);
        break;

    case EN_EMITTER:
        v = 0.0;
        if (Node[index].Ke > 0.0)
        {
            v = Ucf[FLOW] / pow((Ucf[PRESSURE] * Node[index].Ke), (1.0 / hyd->Qexp));
        }
        break;

    case EN_INITQUAL:
        v = Node[index].C0 * Ucf[QUALITY];
        break;

    case EN_SOURCEQUAL:
    case EN_SOURCETYPE:
    case EN_SOURCEMASS:
    case EN_SOURCEPAT:
        source = Node[index].S;
        if (source == NULL) return 240;
        if (code == EN_SOURCEQUAL) v = source->C0;
        else if (code == EN_SOURCEMASS) v = source->Smass * 60.0;
        else if (code == EN_SOURCEPAT)  v = source->Pat;
        else v = source->Type;
        break;

    case EN_TANKLEVEL:
        if (index <= nJuncs) return 251;
        v = (Tank[index - nJuncs].H0 - Node[index].El) * Ucf[ELEV];
        break;

    case EN_INITVOLUME:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].V0 * Ucf[VOLUME];
        break;

    case EN_MIXMODEL:
        v = MIX1;
        if (index > nJuncs) v = Tank[index - nJuncs].MixModel;
        break;

    case EN_MIXZONEVOL:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].V1max * Ucf[VOLUME];
        break;

    case EN_DEMAND:
        v = NodeDemand[index] * Ucf[FLOW];
        break;

    case EN_HEAD:
        v = NodeHead[index] * Ucf[HEAD];
        break;

    case EN_PRESSURE:
        v = (NodeHead[index] - Node[index].El) * Ucf[PRESSURE];
        break;

    case EN_QUALITY:
        v = NodeQual[index] * Ucf[QUALITY];
        break;

    case EN_TANKDIAM:
        v = 0.0;
        if (index > nJuncs)
        {
            v = sqrt(4.0 / PI * Tank[index - nJuncs].A) * Ucf[ELEV];
        }
        break;

    case EN_MINVOLUME:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].Vmin * Ucf[VOLUME];
        break;

    case EN_MAXVOLUME:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].Vmax * Ucf[VOLUME];
        break;

    case EN_VOLCURVE:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].Vcurve;
        break;

    case EN_MINLEVEL:
        v = 0.0;
        if (index > nJuncs)
        {
            v = (Tank[index - nJuncs].Hmin - Node[index].El) * Ucf[ELEV];
        }
        break;

    case EN_MAXLEVEL:
        v = 0.0;
        if (index > nJuncs)
        {
            v = (Tank[index - nJuncs].Hmax - Node[index].El) * Ucf[ELEV];
        }
        break;

    case EN_MIXFRACTION:
        v = 1.0;
        if (index > nJuncs && Tank[index - nJuncs].Vmax > 0.0)
        {
            v = Tank[index - nJuncs].V1max / Tank[index - nJuncs].Vmax;
        }
        break;

    case EN_TANK_KBULK:
        v = 0.0;
        if (index > nJuncs) v = Tank[index - nJuncs].Kb * SECperDAY;
        break;

    case EN_TANKVOLUME:
        if (index <= nJuncs) return 251;
        v = tankvolume(p, index - nJuncs, NodeHead[index]) * Ucf[VOLUME];
        break;

    default:
        return 251;
    }
    *value = (EN_API_FLOAT_TYPE)v;
    return 0;
}

int DLLEXPORT EN_setnodevalue(EN_Project p, int index, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
**  Input:   index = node index
**           code  = node property code (see EN_NodeProperty)
**           value = node property value
**  Output:  none
**  Returns: error code
**  Purpose: sets a property value for a node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    Snode *Node = net->Node;
    Stank *Tank = net->Tank;

    const int nNodes = net->Nnodes;
    const int nJuncs = net->Njuncs;
    const int nPats = net->Npats;

    double *Ucf = p->Ucf;

    int j;
    Pdemand demand;
    Psource source;
    double hTmp;
    double value = v;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > nNodes) return 203;
    switch (code)
    {
    case EN_ELEVATION:
        if (index <= nJuncs) Node[index].El = value / Ucf[ELEV];
        else
        {
            value = (value / Ucf[ELEV]) - Node[index].El;
            j = index - nJuncs;
            Tank[j].H0 += value;
            Tank[j].Hmin += value;
            Tank[j].Hmax += value;
            Node[index].El += value;
            hyd->NodeHead[index] += value;
        }
        break;

    case EN_BASEDEMAND:
        // NOTE: primary demand category is last on demand list
        if (index <= nJuncs)
        {
            for (demand = Node[index].D; demand != NULL; demand = demand->next)
            {
                if (demand->next == NULL) demand->Base = value / Ucf[FLOW];
            }
        }
        break;

    case EN_PATTERN:
        // NOTE: primary demand category is last on demand list
        j = ROUND(value);
        if (j < 0 || j > nPats) return 205;
        if (index <= nJuncs)
        {
            for (demand = Node[index].D; demand != NULL; demand = demand->next)
            {
                if (demand->next == NULL) demand->Pat = j;
            }
        }
        else Tank[index - nJuncs].Pat = j;
        break;

    case EN_EMITTER:
        if (index > nJuncs) return 203;
        if (value < 0.0) return 202;
        if (value > 0.0) value = pow((Ucf[FLOW] / value), hyd->Qexp) / Ucf[PRESSURE];
        Node[index].Ke = value;
        break;

    case EN_INITQUAL:
        if (value < 0.0) return 202;
        Node[index].C0 = value / Ucf[QUALITY];
        if (index > nJuncs) Tank[index - nJuncs].C = Node[index].C0;
        break;

    case EN_SOURCEQUAL:
    case EN_SOURCETYPE:
    case EN_SOURCEPAT:
        if (value < 0.0) return 202;
        source = Node[index].S;
        if (source == NULL)
        {
            source = (struct Ssource *)malloc(sizeof(struct Ssource));
            if (source == NULL) return 101;
            source->Type = CONCEN;
            source->C0 = 0.0;
            source->Pat = 0;
            Node[index].S = source;
        }
        if (code == EN_SOURCEQUAL) source->C0 = value;
        else if (code == EN_SOURCEPAT)
        {
            j = ROUND(value);
            if (j < 0 || j > nPats) return 205;
            source->Pat = j;
        }
        else // code == EN_SOURCETYPE
        {
            j = ROUND(value);
            if (j < CONCEN || j > FLOWPACED) return 251;
            else source->Type = (char)j;
        }
        return 0;

    case EN_TANKLEVEL:
        if (index <= nJuncs) return 251;
        j = index - nJuncs;
        if (Tank[j].A == 0.0) /* Tank is a reservoir */
        {
            Tank[j].H0 = value / Ucf[ELEV];
            Tank[j].Hmin = Tank[j].H0;
            Tank[j].Hmax = Tank[j].H0;
            Node[index].El = Tank[j].H0;
            hyd->NodeHead[index] = Tank[j].H0;
        }
        else
        {
            value = Node[index].El + value / Ucf[ELEV];
            if (value > Tank[j].Hmax || value < Tank[j].Hmin) return 202;
            Tank[j].H0 = value;
            Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
            // Resetting Volume in addition to initial volume
            Tank[j].V = Tank[j].V0;
            hyd->NodeHead[index] = Tank[j].H0;
        }
        break;

    case EN_TANKDIAM:
        if (value <= 0.0) return 202;
        if (index <= nJuncs) return 251;
        j = index - nJuncs;
        if (j > 0 && Tank[j].A > 0.0)
        {
            value /= Ucf[ELEV];
            Tank[j].A = PI * SQR(value) / 4.0;
            Tank[j].Vmin = tankvolume(p, j, Tank[j].Hmin);
            Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
            Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
        }
        else return 251;
        break;

    case EN_MINVOLUME:
        if (value < 0.0) return 202;
        if (index <= nJuncs) return 251;
        j = index - nJuncs;
        if (j > 0 && Tank[j].A > 0.0)
        {
            Tank[j].Vmin = value / Ucf[VOLUME];
            Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
            Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
        }
        else return 251;
        break;

    case EN_MINLEVEL:
        if (value < 0.0) return 202;
        if (index <= nJuncs) return 251; // not a tank or reservoir
        j = index - nJuncs;
        if (Tank[j].A == 0.0) return  251; // node is a reservoir
        hTmp = value / Ucf[ELEV] + Node[index].El;
        if (hTmp < Tank[j].Hmax && hTmp <= Tank[j].H0)
        {
            if (Tank[j].Vcurve > 0) return 202;
            Tank[j].Hmin = hTmp;
            Tank[j].Vmin = (Tank[j].Hmin - Node[index].El) * Tank[j].A;
        }
        else return 251;
        break;

    case EN_MAXLEVEL:
        if (value < 0.0) return 202;
        if (index <= nJuncs) return 251; // not a tank or reservoir
        j = index - nJuncs;
        if (Tank[j].A == 0.0) return 251; // node is a reservoir
        hTmp = value / Ucf[ELEV] + Node[index].El;
        if (hTmp > Tank[j].Hmin && hTmp >= Tank[j].H0)
        {
            if (Tank[j].Vcurve > 0) return 202;
            Tank[j].Hmax = hTmp;
            Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
        }
        else return 251;
        break;

    case EN_MIXMODEL:
        j = ROUND(value);
        if (index <= nJuncs) return 251;
        if (j < MIX1 || j > LIFO) return 202;
        if (index > nJuncs && Tank[index - nJuncs].A > 0.0)
        {
            Tank[index - nJuncs].MixModel = (char)j;
        }
        else return 251;
        break;

    case EN_MIXFRACTION:
        if (value < 0.0 || value > 1.0) return 202;
        if (index <= nJuncs) return 251;
        j = index - nJuncs;
        if (j > 0 && Tank[j].A > 0.0)
        {
            Tank[j].V1max = value * Tank[j].Vmax;
        }
        break;

    case EN_TANK_KBULK:
        if (index <= nJuncs) return 251;
        j = index - nJuncs;
        if (j > 0 && Tank[j].A > 0.0)
        {
            Tank[j].Kb = value / SECperDAY;
            qual->Reactflag = 1;
        }
        else return 251;
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_getcoord(EN_Project p, int index, EN_API_FLOAT_TYPE *x,
                          EN_API_FLOAT_TYPE *y)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  x = node x-coordinate
**           y = node y-coordinate
**  Returns: error code
**  Purpose: retrieves the coordinates of a node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Snode *node;

    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nnodes) return 203;

    // check if node have coords
    node = &net->Node[index];
    if (node->X == MISSING ||
        node->Y == MISSING) return 254;

    *x = (EN_API_FLOAT_TYPE)(node->X);
    *y = (EN_API_FLOAT_TYPE)(node->Y);
    return 0;
}

int DLLEXPORT EN_setcoord(EN_Project p, int index, EN_API_FLOAT_TYPE x,
                          EN_API_FLOAT_TYPE y)
/*----------------------------------------------------------------
**  Input:   index = node index
**           x = node x-coordinate
**           y = node y-coordinate
**  Output:  none
**  Returns: error code
**  Purpose: sets the coordinates of a node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Snode *node;

    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nnodes) return 203;
    node = &net->Node[index];
    node->X = x;
    node->Y = y;
    return 0;
}

/********************************************************************

    Nodal Demand Functions

********************************************************************/

int DLLEXPORT EN_getdemandmodel(EN_Project p, int *type, EN_API_FLOAT_TYPE *pmin,
                                EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  type = type of demand model (see EN_DemandModel)
**           pmin = minimum pressure for any demand
**           preq = required pressure for full demand
**           pexp = exponent in pressure dependent demand formula
**  Returns: error code
**  Purpose: retrieves the parameters of a project's demand model
**----------------------------------------------------------------
*/
{
    *type = p->hydraul.DemandModel;
    *pmin = (EN_API_FLOAT_TYPE)(p->hydraul.Pmin * p->Ucf[PRESSURE]);
    *preq = (EN_API_FLOAT_TYPE)(p->hydraul.Preq * p->Ucf[PRESSURE]);
    *pexp = (EN_API_FLOAT_TYPE)(p->hydraul.Pexp);
    return 0;
}

int DLLEXPORT EN_setdemandmodel(EN_Project p, int type, EN_API_FLOAT_TYPE pmin,
                                EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp)
/*----------------------------------------------------------------
**  Input:   type = type of demand model (see EN_DemandModel)
**           pmin = minimum pressure for any demand
**           preq = required pressure for full demand
**           pexp = exponent in pressure dependent demand formula
**  Output:  none
**  Returns: error code
**  Purpose: sets the parameters of a project's demand model
**----------------------------------------------------------------
*/
{
    if (type < 0 || type > EN_PDA) return 251;
    if (pmin > preq || pexp <= 0.0) return 202;
    p->hydraul.DemandModel = type;
    p->hydraul.Pmin = pmin / p->Ucf[PRESSURE];
    p->hydraul.Preq = preq / p->Ucf[PRESSURE];
    p->hydraul.Pexp = pexp;
    return 0;
}

int DLLEXPORT EN_getnumdemands(EN_Project p, int nodeIndex, int *numDemands)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**  Output:  numDemands  = number of demand categories
**  Returns: error code
**  Purpose: retrieves the number of demand categories for a node
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 0;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Count the number of demand categories
    for (d = p->network.Node[nodeIndex].D; d != NULL; d = d->next) n++;
    *numDemands = n;
    return 0;
}

int DLLEXPORT EN_getbasedemand(EN_Project p, int nodeIndex, int demandIdx,
                               EN_API_FLOAT_TYPE *baseDemand)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**  Output:  baseDemand = baseline demand value
**  Returns: error code
**  Purpose: retrieves the baseline value for a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Retrieve demand for specified category
    if (nodeIndex <= p->network.Njuncs)
    {
        for (d = p->network.Node[nodeIndex].D; n < demandIdx && d->next != NULL;
             d = d->next) n++;
        if (n != demandIdx) return 253;
        *baseDemand = (EN_API_FLOAT_TYPE)(d->Base * p->Ucf[FLOW]);
    }
    else *baseDemand = (EN_API_FLOAT_TYPE)(0.0);
    return 0;
}

int DLLEXPORT EN_setbasedemand(EN_Project p, int nodeIndex, int demandIdx,
                               EN_API_FLOAT_TYPE baseDemand)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**           baseDemand = baseline demand value
**  Output:  none
**  Returns: error code
**  Purpose: sets the baseline value for a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Set baseline demand for specified category
    if (nodeIndex <= p->network.Njuncs)
    {
        for (d = p->network.Node[nodeIndex].D; n < demandIdx && d->next != NULL;
             d = d->next) n++;
        if (n != demandIdx) return 253;
        d->Base = baseDemand / p->Ucf[FLOW];
    }
    return 0;
}

int DLLEXPORT EN_getdemandname(EN_Project p, int nodeIndex, int demandIdx, char *demandName)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**  Output:  demandname = demand category name
**  Returns: error code
**  Purpose: retrieves the name assigned to a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 1;

    strcpy(demandName, "");

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Njuncs) return 203;

    // Locate demand category record and retrieve its name
    for (d = p->network.Node[nodeIndex].D;
        n < demandIdx && d->next != NULL; d = d->next) n++;
    if (n != demandIdx) return 253;
    strcpy(demandName, d->Name);
    return 0;
}

int DLLEXPORT EN_setdemandname(EN_Project p, int nodeIndex, int demandIdx, char *demandName)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**           demandName = name of demand category
**  Output:  none
**  Returns: error code
**  Purpose: assigns a name to a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Njuncs) return 203;

    // Locate demand category record and assign demandName to it
    for (d = p->network.Node[nodeIndex].D;
         n < demandIdx && d->next != NULL; d = d->next) n++;
    if (n != demandIdx) return 253;
    strncpy(d->Name, demandName, MAXMSG);
    return 0;
}

int DLLEXPORT EN_getdemandpattern(EN_Project p, int nodeIndex, int demandIdx, int *patIdx)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**  Output:  patIdx = time pattern index
**  Returns: error code
**  Purpose: retrieves the time pattern assigned to a node's
**           demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;
    for (d = p->network.Node[nodeIndex].D;
         n < demandIdx && d->next != NULL; d = d->next) n++;
    if (n != demandIdx) return 253;
    *patIdx = d->Pat;
    return 0;
}

int  DLLEXPORT EN_setdemandpattern(EN_Project p, int nodeIndex, int demandIdx, int patIdx)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIdx = demand category index
**           patIdx = time pattern index
**  Output:  none
**  Returns: error code
**  Purpose: assigns a time pattern to a node's demand category
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Pdemand d;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > net->Nnodes) return 203;
    if (patIdx <= 0 || patIdx > net->Npats) return 205;

    // Locate demand category record and assign time pattern to it
    if (nodeIndex <= net->Njuncs)
    {
        for (d = net->Node[nodeIndex].D;
             n < demandIdx && d->next != NULL; d = d->next) n++;
        if (n != demandIdx) return 253;
        d->Pat = patIdx;
    }
    return 0;
}

/********************************************************************

    Link Functions

********************************************************************/

int DLLEXPORT EN_addlink(EN_Project p, char *id, EN_LinkType linkType,
                         char *fromNode, char *toNode)
/*----------------------------------------------------------------
**  Input:   id = link ID name
**           type = link type (see EN_LinkType)
**           fromNode = name of link's starting node
**           toNode = name of link's ending node
**  Output:  none
**  Returns: error code
**  Purpose: adds a new link to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;

    int i, n, size;
    int n1, n2;
    Slink *link;
    Spump *pump;

    // Check if a link with same id already exists
    if (!p->Openflag) return 102;
    if (EN_getlinkindex(p, id, &i) == 0) return 215;

    // Lookup the link's from and to nodes
    n1 = hashtable_find(net->NodeHashTable, fromNode);
    n2 = hashtable_find(net->NodeHashTable, toNode);
    if (n1 == 0 || n2 == 0) return 203;

    // Check that id name is not too long
    if (strlen(id) > MAXID) return 250;

    net->Nlinks++;
    n = net->Nlinks;

    // Grow link-related arrays to accomodate the new link
    size = (n + 1) * sizeof(Slink);
    net->Link = (Slink *)realloc(net->Link, size);
    size = (n + 1) * sizeof(double);
    hyd->LinkFlow = (double *)realloc(hyd->LinkFlow, size);
    hyd->LinkSetting = (double *)realloc(hyd->LinkSetting, size);
    size = (n + 1) * sizeof(StatusType);
    hyd->LinkStatus = (StatusType *)realloc(hyd->LinkStatus, size);

    link = &net->Link[n];
    strncpy(link->ID, id, MAXID);

    if (linkType <= EN_PIPE) net->Npipes++;
    else if (linkType == EN_PUMP)
    {
        // Grow pump array to accomodate the new link
        net->Npumps++;
        size = (net->Npumps + 1) * sizeof(Spump);
        net->Pump = (Spump *)realloc(net->Pump, size);
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
        pump->Energy.TotalCost = MISSING;
    }
    else
    {
        // Grow valve array to accomodate the new link
        net->Nvalves++;
        size = (net->Nvalves + 1) * sizeof(Svalve);
        net->Valve = (Svalve *)realloc(net->Valve, size);
        net->Valve[net->Nvalves].Link = n;
    }

    link->Type = linkType;
    link->N1 = n1;
    link->N2 = n2;
    link->Status = OPEN;

    if (linkType == EN_PUMP)
    {
        link->Kc = 1.0; // Speed factor
        link->Km = 0.0; // Horsepower
        link->Len = 0.0;
    }
    else if (linkType <= EN_PIPE)  // pipe or cvpipe
    {
        link->Diam = 10 / p->Ucf[DIAM];
        link->Kc = 100; // Rough. coeff
        link->Km = 0.0; // Loss coeff
        link->Len = 1000;
    }
    else  // Valve
    {
        link->Diam = 10 / p->Ucf[DIAM];
        link->Kc = 0.0; // Valve setting.
        link->Km = 0.0; // Loss coeff
        link->Len = 0.0;
        link->Status = ACTIVE;
    }
    link->Kb = 0;
    link->Kw = 0;
    link->R = 0;
    link->Rc = 0;
    link->Rpt = 0;
    strcpy(link->Comment, "");

    hashtable_insert(net->LinkHashTable, link->ID, n);
    return 0;
}

int DLLEXPORT EN_deletelink(EN_Project p, int index, int actionCode)
/*----------------------------------------------------------------
**  Input:   index  = index of the link to delete
**           actionCode = how to treat controls that contain the link:
**           EN_UNCONDITIONAL deletes all such controls plus the link,
**           EN_CONDITIONAL does not delete the link if it appears
**           in a control and returns an error code
**  Output:  none
**  Returns: error code
**  Purpose: deletes a link from a project.
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i;
    int pumpindex;
    int valveindex;
    EN_LinkType linkType;
    Slink *link;

    // Check that link exists
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) 204;
    if (actionCode < EN_UNCONDITIONAL || actionCode > EN_CONDITIONAL) return 251;

    // Deletion will be cancelled if link appears in any controls
    if (actionCode == EN_CONDITIONAL)
    {
        actionCode = incontrols(p, LINK, index);
        if (actionCode > 0) return 261;
    }

    // Get references to the link and its type
    link = &net->Link[index];
    EN_getlinktype(p, index, &linkType);

    // Remove link from its hash table
    hashtable_delete(net->LinkHashTable, link->ID);

    // Shift position of higher entries in Link array down one
    for (i = index; i <= net->Nlinks - 1; i++)
    {
        net->Link[i] = net->Link[i + 1];
        // ... update link's entry in the hash table
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
    if (linkType == PUMP)
    {
        pumpindex = findpump(net, index);
        for (i = pumpindex; i <= net->Npumps - 1; i++)
        {
            net->Pump[i] = net->Pump[i + 1];
        }
        net->Npumps--;
    }

    // Delete any valve (linkType > EN_PUMP) associated with the deleted link
    if (linkType > PUMP)
    {
        valveindex = findvalve(net, index);
        for (i = valveindex; i <= net->Nvalves - 1; i++)
        {
            net->Valve[i] = net->Valve[i + 1];
        }
        net->Nvalves--;
    }

    // Delete any control containing the link
    for (i = net->Ncontrols; i >= 1; i--)
    {
        if (net->Control[i].Link == index) EN_deletecontrol(p, i);
    }

    // Adjust higher numbered link indices in remaining controls
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (net->Control[i].Link > index) net->Control[i].Link--;
    }

    // Make necessary adjustments to rule-based controls
    adjustrules(p, EN_R_LINK, index);  // see RULES.C

    // Reduce link count by one
    net->Nlinks--;
    return 0;
}

int DLLEXPORT EN_getlinkindex(EN_Project p, char *id, int *index)
/*----------------------------------------------------------------
**  Input:   id = link ID name
**  Output:  index = link index
**  Returns: error code
**  Purpose: retrieves the index of a link
**----------------------------------------------------------------
*/
{
    *index = 0;
    if (!p->Openflag) return 102;
    *index = findlink(&p->network, id);
    if (*index == 0) return 204;
    else return 0;
}

int DLLEXPORT EN_getlinkid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = link index
**  Output:  id = link ID name
**  Returns: error code
**  Purpose: retrieves the ID name of a link
**----------------------------------------------------------------
*/
{
    strcpy(id, "");
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nlinks) return 204;
    strcpy(id, p->network.Link[index].ID);
    return 0;
}

int DLLEXPORT EN_setlinkid(EN_Project p, int index, char *newid)
/*----------------------------------------------------------------
**  Input:   index = link index
**           id = link ID name
**  Output:  none
**  Returns: error code
**  Purpose: sets the ID name of a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    size_t n;

    // Check for valid arguments
    if (index <= 0 || index > net->Nlinks) return 204;
    n = strlen(newid);
    if (n < 1 || n > MAXID) return 211;
    if (strcspn(newid, " ;") < n) return 211;

    // Check if another link with same name exists
    if (hashtable_find(net->LinkHashTable, newid) > 0) return 215;

    // Replace the existing link ID with the new value
    hashtable_delete(net->LinkHashTable, net->Link[index].ID);
    strncpy(net->Link[index].ID, newid, MAXID);
    hashtable_insert(net->LinkHashTable, net->Link[index].ID, index);
    return 0;
}

int DLLEXPORT EN_getlinktype(EN_Project p, int index, EN_LinkType *code)
/*----------------------------------------------------------------
**  Input:   index = link index
**  Output:  code = link type (see EN_LinkType)
**  Returns: error code
**  Purpose: retrieves the type code of a link
**----------------------------------------------------------------
*/
{
    *code = -1;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nlinks) return 204;
    *code = p->network.Link[index].Type;
    return 0;
}

int DLLEXPORT EN_setlinktype(EN_Project p, int *index, EN_LinkType type, int actionCode)
/*----------------------------------------------------------------
**  Input:   index = link index
**           type = link type (see EN_LinkType)
**           actionCode = how to treat controls that contain the link:
**           EN_UNCONDITIONAL deletes all such controls,
**           EN_CONDITIONAL cancels the type change if the link appears
**           in a control and returns an error code
**  Output:  none
**  Returns: error code
**  Purpose: retrieves the ID name of an indexed link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i = *index, n1, n2;
    char id[MAXID + 1];
    char id1[MAXID + 1];
    char id2[MAXID + 1];
    int errcode;
    EN_LinkType oldtype;

    // Check for valid input parameters
    if (!p->Openflag) return 102;
    if (type < 0 || type > GPV || actionCode < EN_UNCONDITIONAL ||
        actionCode > EN_CONDITIONAL)
    {
        return 251;
    }

    // Check for valid link index
    if (i <= 0 || i > net->Nlinks) return 204;

    // Check if current link type equals new type
    EN_getlinktype(p, i, &oldtype);
    if (oldtype == type) return 0;

    // Type change will be cancelled if link appears in any controls
    if (actionCode == EN_CONDITIONAL)
    {
        actionCode = incontrols(p, LINK, i);
        if (actionCode > 0) return 261;
    }

    // Pipe changing from or to having a check valve
    if (oldtype <= PIPE && type <= PIPE)
    {
        net->Link[i].Type = type;
        if (type == CVPIPE) net->Link[i].Status = OPEN;
        return 0;
    }

    // Get ID's of link & its end nodes
    EN_getlinkid(p, i, id);
    EN_getlinknodes(p, i, &n1, &n2);
    EN_getnodeid(p, n1, id1);
    EN_getnodeid(p, n2, id2);

    // Delete the original link (and any controls containing it)
    EN_deletelink(p, i, actionCode);

    // Create a new link of new type and old id
    errcode = EN_addlink(p, id, type, id1, id2);

    // Find the index of this new link
    EN_getlinkindex(p, id, index);
    return errcode;
}

int DLLEXPORT EN_getlinknodes(EN_Project p, int index, int *node1, int *node2)
/*----------------------------------------------------------------
**  Input:   index = link index
**  Output:  node1 = index of link's starting node
**           node2 = index of link's ending node
**  Returns: error code
**  Purpose: retrieves the start and end nodes of a link
**----------------------------------------------------------------
*/
{
    *node1 = 0;
    *node2 = 0;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nlinks) return 204;
    *node1 = p->network.Link[index].N1;
    *node2 = p->network.Link[index].N2;
    return 0;
}

int DLLEXPORT EN_setlinknodes(EN_Project p, int index, int node1, int node2)
/*----------------------------------------------------------------
**  Input:   index = link index
**           node1 = index of link's new starting node
**           node2 = index of link's new ending node
**  Returns: error code
**  Purpose: sets the start and end nodes of a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    int type;

    // Check that nodes exist
    if (node1 < 0 || node1 > net->Nnodes) return 203;
    if (node2 < 0 || node2 > net->Nnodes) return 203;

    // Check for illegal valve connection
    type = net->Link[index].Type;
    if (type == EN_PRV || type == EN_PSV || type == EN_FCV)
    {
        // Can't be connected to a fixed grade node
        if (node1 > net->Njuncs ||
            node2 > net->Njuncs) return 219;

        // Can't be connected to another pressure/flow control valve
        if (!valvecheck(p, type, node1, node2)) return 220;
    }

    // Assign new end nodes to link
    net->Link[index].N1 = node1;
    net->Link[index].N2 = node2;
    return 0;
}

int DLLEXPORT EN_getlinkvalue(EN_Project p, int index, EN_LinkProperty code,
                              EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   index = link index
**           code = link property code (see EN_LinkProperty)
**  Output:  value = link property value
**  Returns: error code
**  Purpose: retrieves a property value for a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;

    double a, h, q, v = 0.0;
    int returnValue = 0;
    int pmp;
    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
    double *Ucf = p->Ucf;
    double *LinkFlow = hyd->LinkFlow;
    double *LinkSetting = hyd->LinkSetting;

    // Check for valid arguments
    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;

    // Retrieve called-for property
    switch (code)
    {
    case EN_DIAMETER:
        if (Link[index].Type == EN_PUMP) v = 0.0;
        else v = Link[index].Diam * Ucf[DIAM];
        break;

    case EN_LENGTH:
        v = Link[index].Len * Ucf[ELEV];
        break;

    case EN_ROUGHNESS:
        if (Link[index].Type <= EN_PIPE)
        {
            if (hyd->Formflag == DW) v = Link[index].Kc * (1000.0 * Ucf[ELEV]);
            else v = Link[index].Kc;
        }
        else v = 0.0;
        break;

    case EN_MINORLOSS:
        if (Link[index].Type != EN_PUMP)
        {
            v = Link[index].Km;
            v *= (SQR(Link[index].Diam) * SQR(Link[index].Diam) / 0.02517);
        }
        else v = 0.0;
        break;

    case EN_INITSTATUS:
        if (Link[index].Status <= CLOSED) v = 0.0;
        else v = 1.0;
        break;

    case EN_INITSETTING:
        if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
        {
            return EN_getlinkvalue(p, index, EN_ROUGHNESS, value);
        }
        v = Link[index].Kc;
        switch (Link[index].Type)
        {
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
        if (hyd->LinkStatus[index] <= CLOSED) v = 0.0;
        else v = LinkFlow[index] * Ucf[FLOW];
        break;

    case EN_VELOCITY:
        if (Link[index].Type == EN_PUMP) v = 0.0;
        else if (hyd->LinkStatus[index] <= CLOSED) v = 0.0;
        else
        {
            q = ABS(LinkFlow[index]);
            a = PI * SQR(Link[index].Diam) / 4.0;
            v = q / a * Ucf[VELOCITY];
        }
        break;

    case EN_HEADLOSS:
        if (hyd->LinkStatus[index] <= CLOSED) v = 0.0;
        else
        {
            h = hyd->NodeHead[Link[index].N1] - hyd->NodeHead[Link[index].N2];
            if (Link[index].Type != EN_PUMP) h = ABS(h);
            v = h * Ucf[HEADLOSS];
        }
        break;

    case EN_STATUS:
        if (hyd->LinkStatus[index] <= CLOSED) v = 0.0;
        else v = 1.0;
        break;

    case EN_STATE:
        v = hyd->LinkStatus[index];

        if (Link[index].Type == EN_PUMP)
        {
            pmp = findpump(net, index);
            if (hyd->LinkStatus[index] >= OPEN)
            {
                if (hyd->LinkFlow[index] > hyd->LinkSetting[index] * Pump[pmp].Qmax)
                {
                    v = XFLOW;
                }
                if (hyd->LinkFlow[index] < 0.0) v = XHEAD;
            }
        }
        break;

    case EN_CONST_POWER:
        v = 0;
        if (Link[index].Type == EN_PUMP)
        {
            pmp = findpump(net, index);
            if (Pump[pmp].Ptype == CONST_HP) v = Link[index].Km; // Power in HP
        }
        break;

    case EN_SPEED:
        v = 0;
        if (Link[index].Type == EN_PUMP)
        {
            pmp = findpump(net, index);
            v = Link[index].Kc;
        }
        break;

    case EN_SETTING:
        if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
        {
            return EN_getlinkvalue(p, index, EN_ROUGHNESS, value);
        }
        if (LinkSetting[index] == MISSING) v = 0.0;
        else v = LinkSetting[index];
        switch (Link[index].Type)
        {
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
        {
            v = (double)Pump[findpump(&p->network, index)].Upat;
        }
        break;

    case EN_EFFICIENCY:
        getenergy(p, index, &a, &v);
        break;

    case EN_PRICEPATTERN:
        if (Link[index].Type == EN_PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Epat;
        }
        break;

    case EN_HEADCURVE:
        if (Link[index].Type == EN_PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Hcurve;
            if (v == 0) returnValue = 226;
        }
        else
        {
            v = 0;
            returnValue = 211;
        }
        break;

    case EN_EFFICIENCYCURVE:
        if (Link[index].Type == EN_PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Ecurve;
            if (v == 0) returnValue = 268;
        }
        else
        {
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

int DLLEXPORT EN_setlinkvalue(EN_Project p, int index, int code, EN_API_FLOAT_TYPE v)
/*----------------------------------------------------------------
**  Input:   index = link index
**           code  = link property code (see EN_LinkProperty)
**           v = property value
**  Output:  none
**  Returns: error code
**  Purpose: sets a property value for a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    Slink *Link = net->Link;
    double *Ucf = p->Ucf;
    double *LinkSetting = hyd->LinkSetting;
    char s;
    double r, value = v;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    switch (code)
    {
    case EN_DIAMETER:
        if (Link[index].Type != EN_PUMP)
        {
            if (value <= 0.0) return 202;
            value /= Ucf[DIAM];                // Convert to feet
            r = Link[index].Diam / value;      // Ratio of old to new diam
            Link[index].Km *= SQR(r) * SQR(r); // Adjust minor loss factor
            Link[index].Diam = value;          // Update diameter
            resistcoeff(p, index);             // Update resistance coeff.
        }
        break;

    case EN_LENGTH:
        if (Link[index].Type <= EN_PIPE)
        {
            if (value <= 0.0) return 202;
            Link[index].Len = value / Ucf[ELEV];
            resistcoeff(p, index);
        }
        break;

    case EN_ROUGHNESS:
        if (Link[index].Type <= EN_PIPE)
        {
            if (value <= 0.0) return 202;
            Link[index].Kc = value;
            if (hyd->Formflag == DW) Link[index].Kc /= (1000.0 * Ucf[ELEV]);
            resistcoeff(p, index);
        }
        break;

    case EN_MINORLOSS:
        if (Link[index].Type != EN_PUMP)
        {
            if (value <= 0.0) return 202;
            Link[index].Km = 0.02517 * value / SQR(Link[index].Diam) /
                             SQR(Link[index].Diam);
        }
        break;

    case EN_INITSTATUS:
    case EN_STATUS:
        // Cannot set status for a check valve
        if (Link[index].Type == EN_CVPIPE) return 207;
        s = (char)ROUND(value);
        if (s < 0 || s > 1) return 251;
        if (code == EN_INITSTATUS)
        {
            setlinkstatus(p, index, s, &Link[index].Status, &Link[index].Kc);
        }
        else
        {
            setlinkstatus(p, index, s, &hyd->LinkStatus[index], &LinkSetting[index]);
        }
        break;

    case EN_INITSETTING:
    case EN_SETTING:
        if (value < 0.0) return 202;
        if (Link[index].Type == EN_PIPE || Link[index].Type == EN_CVPIPE)
        {
            return EN_setlinkvalue(p, index, EN_ROUGHNESS, v);
        }
        else
        {
            switch (Link[index].Type)
            {
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
            case EN_GPV:
                return 202; // Cannot modify setting for GPV
            default:
                return 251;
            }
            if (code == EN_INITSETTING)
            {
                setlinksetting(p, index, value, &Link[index].Status, &Link[index].Kc);
            }
            else
            {
                setlinksetting(p, index, value, &hyd->LinkStatus[index],
                               &LinkSetting[index]);
            }
        }
        break;

    case EN_KBULK:
        if (Link[index].Type <= EN_PIPE)
        {
            Link[index].Kb = value / SECperDAY;
            qual->Reactflag = 1;
        }
        break;

    case EN_KWALL:
        if (Link[index].Type <= EN_PIPE)
        {
            Link[index].Kw = value / SECperDAY;
            qual->Reactflag = 1;
        }
        break;

    default:
        return 251;
    }
    return 0;
}

/********************************************************************

    Pump Functions

********************************************************************/

int DLLEXPORT EN_getpumptype(EN_Project p, int index, int *type)
/*----------------------------------------------------------------
**  Input:   index = index of a pump link
**  Output:  type = type of pump characteristic curve (see EN_PumpType)
**  Returns: error code
**  Purpose: retrieves the type of characteristic curve used by a pump
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
    const int Nlinks = net->Nlinks;

    *type = -1;
    if (!p->Openflag) return 102;
    if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type) return 204;
    *type = Pump[findpump(&p->network, index)].Ptype;
    return 0;
}

int DLLEXPORT EN_getheadcurveindex(EN_Project p, int index, int *curveindex)
/*----------------------------------------------------------------
**  Input:   index = index of a pump link
**  Output:  curveindex = index of a pump's characteristic curve
**  Returns: error code
**  Purpose: retrieves the index of a pump's characteristic curve
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
    const int Nlinks = net->Nlinks;

    if (!p->Openflag) return 102;
    if (index < 1 || index > Nlinks || EN_PUMP != Link[index].Type) return 204;
    *curveindex = Pump[findpump(net, index)].Hcurve;
    return 0;
}

int DLLEXPORT EN_setheadcurveindex(EN_Project p, int index, int curveindex)
/*----------------------------------------------------------------
**  Input:   index = index of a pump link
**           curveindex = index of a pump's characteristic curve
**  Output:  none
**  Returns: error code
**  Purpose: assigns a new characteristic curve to a pump
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    const int Nlinks = net->Nlinks;
    const int Ncurves = net->Ncurves;
    double *Ucf = p->Ucf;
    int pIdx;
    Spump *pump;

    // Check for valid parameters
    if (!p->Openflag) return 102;
    if (index < 1 || index > net->Nlinks || EN_PUMP != net->Link[index].Type) return 204;
    if (curveindex <= 0 || curveindex > net->Ncurves) return 206;

    // Assign the new curve to the pump
    pIdx = findpump(net, index);
    pump = &p->network.Pump[pIdx];
    pump->Ptype = NOCURVE;
    pump->Hcurve = curveindex;

    // Update the pump curve's parameters and convert their units
    updatepumpparams(p, pIdx);
    if (pump->Ptype == POWER_FUNC)
    {
        pump->H0 /= Ucf[HEAD];
        pump->R *= (pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
    }
    pump->Q0 /= Ucf[FLOW];
    pump->Qmax /= Ucf[FLOW];
    pump->Hmax /= Ucf[HEAD];

    // Designate the newly assigned curve as being a Pump Curve
    p->network.Curve[curveindex].Type = P_CURVE;
    return 0;
}

/********************************************************************

    Time Pattern Functions

********************************************************************/

int DLLEXPORT EN_addpattern(EN_Project p, char *id)
/*----------------------------------------------------------------
**  Input:   id = time pattern ID name
**  Output:  none
**  Returns: error code
**  Purpose: adds a new time pattern to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Parser  *parser = &p->parser;
    Hydraul *hyd = &p->hydraul;

    int i, n, err = 0;
    Spattern *pat;

    // Check if a pattern with same id already exists
    if (!p->Openflag) return 102;
    if (EN_getpatternindex(p, id, &i) == 0) return 215;

    // Check that id name is not too long
    if (strlen(id) > MAXID) return 250;

    // Expand the project's array of patterns
    n = net->Npats + 1;
    net->Pattern = (Spattern *)realloc(net->Pattern, (n + 1) * sizeof(Spattern));

    // Assign properties to the new pattern
    pat = &net->Pattern[n];
    strcpy(pat->ID, id);
    pat->Length = 1;
    pat->F = (double *)calloc(1, sizeof(double));
    if (pat->F == NULL) err = 1;
    else pat->F[0] = 1.0;

    // Abort if memory allocation error
    if (err)
    {
        free(pat->F);
        return 101;
    }

    // Update the number of patterns
    net->Npats = n;
    parser->MaxPats = n;

    // Make new pattern be default demand pattern if name matches
    if (strcmp(id, parser->DefPatID) == 0) hyd->DefPat = n;
    return 0;
}

int DLLEXPORT EN_getpatternindex(EN_Project p, char *id, int *index)
/*----------------------------------------------------------------
**  Input:   id = time pattern name
**  Output:  index = time pattern index
**  Returns: error code
**  Purpose: retrieves the index of a time pattern
**----------------------------------------------------------------
*/
{
    int i;

    *index = 0;
    if (!p->Openflag) return 102;
    for (i = 1; i <= p->network.Npats; i++)
    {
        if (strcmp(id, p->network.Pattern[i].ID) == 0)
        {
            *index = i;
            return 0;
        }
    }
    *index = 0;
    return 205;
}

int DLLEXPORT EN_getpatternid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**  Output:  id = time pattern ID name
**  Returns: error code
**  Purpose: retrieves the ID name of a time pattern
**----------------------------------------------------------------
*/
{
    strcpy(id, "");
    if (!p->Openflag)return 102;
    if (index < 1 || index > p->network.Npats) return 205;
    strcpy(id, p->network.Pattern[index].ID);
    return 0;
}

int DLLEXPORT EN_getpatternlen(EN_Project p, int index, int *len)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**  Output:  len = number of periods in a time pattern
**  Returns: error code
**  Purpose: retrieves the number of time periods in a time pattern
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Npats) return 205;
    *len = p->network.Pattern[index].Length;
    return 0;
}

int DLLEXPORT EN_getpatternvalue(EN_Project p, int index, int period, EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           period = time pattern period
**  Output:  value = multiplier for a particular time period
**  Returns: error code
**  Purpose: retrieves the multiplier for a specific time period
**           in a time pattern
**----------------------------------------------------------------
*/
{
    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Npats) return 205;
    if (period < 1 || period > p->network.Pattern[index].Length) return 251;
    *value = (EN_API_FLOAT_TYPE)p->network.Pattern[index].F[period - 1];
    return 0;
}

int DLLEXPORT EN_setpatternvalue(EN_Project p, int index, int period, EN_API_FLOAT_TYPE value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           period = time pattern period
**           value = multiplier for a particular time period
**  Output:  none
**  Returns: error code
**  Purpose: sets the multiplier for a specific time period
**           in a time pattern
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Spattern *Pattern = net->Pattern;

    if (!p->Openflag) return  102;
    if (index <= 0 || index > net->Npats) return 205;
    if (period <= 0 || period > Pattern[index].Length) return 251;
    Pattern[index].F[period - 1] = value;
    return 0;
}

int DLLEXPORT EN_getaveragepatternvalue(EN_Project p, int index, EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**  Output:  value = average of a time pattern's multipliers
**  Returns: error code
**  Purpose: retrieves the average multiplier value for a time pattern
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Spattern *Pattern = net->Pattern;
    int i;

    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index < 1 || index > net->Npats) return 205;
    for (i = 0; i < Pattern[index].Length; i++)
    {
        *value += (EN_API_FLOAT_TYPE)Pattern[index].F[i];
    }
    *value /= (EN_API_FLOAT_TYPE)Pattern[index].Length;
    return 0;
}

int DLLEXPORT EN_setpattern(EN_Project p, int index, EN_API_FLOAT_TYPE *f, int n)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           f = an array of multiplier values
**           n = number of values in f
**  Output:  none
**  Returns: error code
**  Purpose: replaces the multipliers in a time pattern
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int j;
    Spattern *Pattern = net->Pattern;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Npats) return 205;
    if (n <= 0) return 202;

    // Re-set number of time periods & reallocate memory for multipliers
    Pattern[index].Length = n;
    Pattern[index].F = (double *)realloc(Pattern[index].F, n * sizeof(double));
    if (Pattern[index].F == NULL) return 101;

    // Load multipliers into pattern
    for (j = 0; j < n; j++) Pattern[index].F[j] = f[j];
    return 0;
}

/********************************************************************

    Data Curve Functions

********************************************************************/

int DLLEXPORT EN_addcurve(EN_Project p, char *id)
/*----------------------------------------------------------------
**  Input:   id = data curve ID name
**  Output:  none
**  Returns: error code
**  Purpose: adds a new data curve to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i, n, err = 0;
    Scurve *curve;

    // Check if a curve with same id already exists
    if (!p->Openflag) return 102;
    if (EN_getcurveindex(p, id, &i) == 0) return 215;

    // Check that id name is not too long
    if (strlen(id) > MAXID) return 250;

    // Expand the array of curves
    n = net->Ncurves + 1;
    net->Curve = (Scurve *) realloc(net->Curve, (n + 1) * sizeof(Scurve));

    // Set the properties of the new curve
    curve = &net->Curve[n];
    strcpy(curve->ID, id);
    curve->Npts = 1;
    curve->Type = G_CURVE;
    curve->X = (double *)calloc(1, sizeof(double));
    curve->Y = (double *)calloc(1, sizeof(double));
    if (curve->X == NULL) err = 1;
    else if (curve->Y == NULL) err = 1;
    else
    {
        curve->X[0] = 1.0;
        curve->Y[0] = 1.0;
    }

    // Abort if memory allocation error
    if (err)
    {
        free(curve->X);
        free(curve->Y);
        return 101;
    }

    // Update the number of curves
    net->Ncurves = n;
    p->parser.MaxCurves = n;
    return 0;
}

int DLLEXPORT EN_getcurveindex(EN_Project p, char *id, int *index)
/*----------------------------------------------------------------
**  Input:   id = data curve name
**  Output:  index = data curve index
**  Returns: error code
**  Purpose: retrieves the index of a data curve
**----------------------------------------------------------------
*/
{
    int i;

    *index = 0;
    if (!p->Openflag) return 102;
    for (i = 1; i <= p->network.Ncurves; i++)
    {
        if (strcmp(id, p->network.Curve[i].ID) == 0)
        {
            *index = i;
            return 0;
        }
    }
    *index = 0;
    return 206;
}

int DLLEXPORT EN_getcurveid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**  Output:  id = data curve ID name
**  Returns: error code
**  Purpose: retrieves the name of a data curve
**----------------------------------------------------------------
*/
{
    strcpy(id, "");
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Ncurves) return 206;
    strcpy(id, p->network.Curve[index].ID);
    return 0;
}

int DLLEXPORT EN_getcurvelen(EN_Project p, int index, int *len)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**  Output:  len = number of points in a data curve
**  Returns: error code
**  Purpose: retrieves the number of points in a data curve
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Ncurves) return 206;
    *len = p->network.Curve[index].Npts;
    return 0;
}

int DLLEXPORT EN_getcurvetype(EN_Project p, int curveindex, int *type)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**  Output:  type = type of data curve (see EN_CurveType)
**  Returns: error code
**  Purpose: retrieves the type assigned to a data curve
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    if (!p->Openflag) return 102;
    if (curveindex < 1 || curveindex > net->Ncurves) return 206;
    *type = net->Curve[curveindex].Type;
    return 0;
}

int DLLEXPORT EN_getcurvevalue(EN_Project p, int index, int pnt,
                               EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**           pnt = index of data curve point
**  Output:  x = x-value of a point on the curve
**           y = y-value of a point on the curve
**  Returns: error code
**  Purpose: retrieves the value of a specific point on a data curve
**----------------------------------------------------------------
*/
{
    *x = 0.0;
    *y = 0.0;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Ncurves) return 206;
    if (pnt < 1 || pnt > p->network.Curve[index].Npts) return 251;
    *x = (EN_API_FLOAT_TYPE)p->network.Curve[index].X[pnt - 1];
    *y = (EN_API_FLOAT_TYPE)p->network.Curve[index].Y[pnt - 1];
    return 0;
}

int DLLEXPORT EN_setcurvevalue(EN_Project p, int index, int pnt,
                               EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           pnt = index of data curve point
**           x = new x-value for point on the curve
**           y = new y-value for point on the curve
**  Output:  none
**  Returns: error code
**  Purpose: sets the value of a specific point on a data curve
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Scurve *curve;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Ncurves) return 206;
    curve = &net->Curve[index];
    if (pnt <= 0 || pnt > curve->Npts) return 251;
    curve->X[pnt - 1] = x;
    curve->Y[pnt - 1] = y;
    return 0;
}

int DLLEXPORT EN_getcurve(EN_Project p, int index, char *id, int *nValues,
                          EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**  Output:  id = ID name of data curve
**           nValues = number of data points on the curve
**           xValues = array of x-values for each data point
**           yValues = array of y-values for each data point
**  Returns: error code
**  Purpose: retrieves the data associated with a data curve
**
**  The calling program is responsible for making xValues and
**  yValues large enough to hold nValues data points.
**----------------------------------------------------------------
*/
{
    int i;
    Scurve *curve;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > p->network.Ncurves) return 206;
    curve = &p->network.Curve[index];
    strncpy(id, curve->ID, MAXID);
    *nValues = curve->Npts;
    for (i = 0; i < curve->Npts; i++)
    {
        *xValues[i] = (EN_API_FLOAT_TYPE)curve->X[i];
        *yValues[i] = (EN_API_FLOAT_TYPE)curve->Y[i];
    }
    return 0;
}

int DLLEXPORT EN_setcurve(EN_Project p, int index, EN_API_FLOAT_TYPE *x,
                          EN_API_FLOAT_TYPE *y, int n)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**           x = array of x-values
**           y = array of y-values
**           n = number of data points in the x and y arrays
**  Returns: error code
**  Purpose: replaces a curve's set of data points
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Scurve *curve;
    int j;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Ncurves) return 206;
    if (n <= 0) return 202;

    // Re-set number of points & reallocate memory for values
    curve = &net->Curve[index];
    curve->Npts = n;
    curve->X = (double *)realloc(curve->X, n * sizeof(double));
    curve->Y = (double *)realloc(curve->Y, n * sizeof(double));
    if (curve->X == NULL) return 101;
    if (curve->Y == NULL) return 101;

    // Load values into curve
    for (j = 0; j < n; j++)
    {
        curve->X[j] = x[j];
        curve->Y[j] = y[j];
    }
    return 0;
}

/********************************************************************

    Simple Controls Functions

********************************************************************/

int DLLEXPORT EN_addcontrol(EN_Project p, int *index, int ctype, int lindex,
                            EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level)
/*----------------------------------------------------------------
**  Input:   ctype = type of control (see EN_ControlType)
**           lindex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nindex = index of node controlling a link (for level controls)
**           level = control activation level (pressure for junction nodes,
**                   water level for tank nodes or time value for time-based
**                   control)
**  Output:  index = the index of the new control
**  Returns: error code
**  Purpose: adds a new simple control to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Parser *parser = &p->parser;

    char status = ACTIVE;
    int  n;
    long t = 0;
    double s = setting, lvl = level;
    double *Ucf = p->Ucf;
    Scontrol *control;


    // Check that project exists
    if (!p->Openflag) return 102;

    // Check that controlled link exists
    if (lindex <= 0 || lindex > net->Nlinks) return 204;

    // Cannot control check valve
    if (net->Link[lindex].Type == EN_CVPIPE) return 207;

    // Check for valid parameters
    if (ctype < 0 || ctype > EN_TIMEOFDAY) return 251;
    if (ctype == EN_LOWLEVEL || ctype == EN_HILEVEL)
    {
        if (nindex < 1 || nindex > net->Nnodes) return 203;
    }
    else nindex = 0;
    if (s < 0.0 || lvl < 0.0) return 202;

    // Adjust units of control parameters
    switch (net->Link[lindex].Type)
    {
    case EN_PRV:
    case EN_PSV:
    case EN_PBV:
        s /= Ucf[PRESSURE];
        break;
    case EN_FCV:
        s /= Ucf[FLOW];
        break;
    case EN_GPV:
        if (s == 0.0) status = CLOSED;
        else if (s == 1.0) status = OPEN;
        else return 202;
        s = net->Link[lindex].Kc;
        break;
    case EN_PIPE:
    case EN_PUMP:
        status = OPEN;
        if (s == 0.0) status = CLOSED;
    default:
        break;
    }

    if (ctype == LOWLEVEL || ctype == HILEVEL)
    {
        if (nindex > net->Njuncs) lvl = net->Node[nindex].El + level / Ucf[ELEV];
        else lvl = net->Node[nindex].El + level / Ucf[PRESSURE];
    }
    if (ctype == TIMER) t = (long)ROUND(lvl);
    if (ctype == TIMEOFDAY) t = (long)ROUND(lvl) % SECperDAY;

    // Expand project's array of controls
    n = net->Ncontrols + 1;
    net->Control = (Scontrol *)realloc(net->Control, (n + 1) * sizeof(Scontrol));

    // Set properties of the new control
    control = &net->Control[n];
    control->Type = (char)ctype;
    control->Link = lindex;
    control->Node = nindex;
    control->Status = status;
    control->Setting = s;
    control->Grade = lvl;
    control->Time = t;

    // Update number of controls
    net->Ncontrols = n;
    parser->MaxControls = n;

    // Return the new control index
    *index = n;
    return 0;
}

int DLLEXPORT EN_deletecontrol(EN_Project p, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  none
**  Returns: error code
**  Purpose: deletes a simple control from a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    int i;

    if (index <= 0 || index > net->Ncontrols) return 241;
    for (i = index; i <= net->Ncontrols - 1; i++)
    {
        net->Control[i] = net->Control[i + 1];
    }
    net->Ncontrols--;
    return 0;
}

int DLLEXPORT EN_getcontrol(EN_Project p, int index, int *ctype, int *lindex,
                            EN_API_FLOAT_TYPE *setting, int *nindex,
                            EN_API_FLOAT_TYPE *level)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  ctype = type of control (see EN_ControlType)
**           lindex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nindex = index of node controlling a link (for level controls)
**           level = control activation level (pressure for junction nodes,
**                   water level for tank nodes or time value for time-based
**                   control)
**  Returns: error code
**  Purpose: Retrieves the properties of a simple control
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    double s, lvl;
    double *Ucf = p->Ucf;
    Scontrol *control;
    Snode *node;

    // Set default return values
    s = 0.0;
    lvl = 0.0;
    *ctype = 0;
    *lindex = 0;
    *nindex = 0;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Ncontrols) return 241;

    // Retrieve control's type and link index
    control = &net->Control[index];
    *ctype = control->Type;
    *lindex = control->Link;

    // Retrieve control's setting
    s = control->Setting;
    if (control->Setting != MISSING)
    {
        switch (net->Link[*lindex].Type)
        {
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
    }
    else if (control->Status == OPEN) s = 1.0;
    else s = 0.0;

    // Retrieve level value for a node level control
    *nindex = control->Node;
    if (*nindex > 0)
    {
        node = &net->Node[*nindex];
        if (*nindex > net->Njuncs)  // Node is a tank
        {
             lvl = (control->Grade - node->El) * Ucf[ELEV];
        }
        else  // Node is a junction
        {
            lvl = (control->Grade - node->El) * Ucf[PRESSURE];
        }
    }

    // Retrieve level value for a time-based control
    else
    {
        lvl = (EN_API_FLOAT_TYPE)control->Time;
    }
    *setting = (EN_API_FLOAT_TYPE)s;
    *level = (EN_API_FLOAT_TYPE)lvl;
    return 0;
}

int DLLEXPORT EN_setcontrol(EN_Project p, int index, int ctype, int lindex,
                            EN_API_FLOAT_TYPE setting, int nindex,
                            EN_API_FLOAT_TYPE level)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**           ctype = type of control (see EN_ControlType)
**           lindex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nindex = index of node controlling a link (for level controls)
**           level = control activation level (pressure for junction nodes,
**                   water level for tank nodes or time value for time-based
**                   control)
**  Output:  none
**  Returns: error code
**  Purpose: Sets the properties of a simple control
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    char status = ACTIVE;
    long t = 0;
    double s = setting, lvl = level;
    double *Ucf = p->Ucf;
    Slink *link;
    Scontrol *control;

    // Check that project exists
    if (!p->Openflag) return 102;

    // Check that control exists
    if (index <= 0 || index > net->Ncontrols) return 241;
    control = &net->Control[index];

    // Check that controlled link exists
    if (lindex == 0)
    {
        control->Link = 0;
        return 0;
    }
    if (lindex < 0 || lindex > net->Nlinks) return 204;

    // Cannot control check valve
    if (net->Link[lindex].Type == EN_CVPIPE) return 207;

    // Check for valid control properties
    if (ctype < 0 || ctype > EN_TIMEOFDAY) return 251;
    if (ctype == EN_LOWLEVEL || ctype == EN_HILEVEL)
    {
        if (nindex < 1 || nindex > net->Nnodes) return 203;
    }
    else nindex = 0;
    if (s < 0.0 || lvl < 0.0) return 202;

    // Adjust units of control's properties
    link = &net->Link[lindex];
    switch (link->Type)
    {
    case EN_PRV:
    case EN_PSV:
    case EN_PBV:
        s /= Ucf[PRESSURE];
        break;
    case EN_FCV:
        s /= Ucf[FLOW];
        break;
    case EN_GPV:
        if (s == 0.0)  status = CLOSED;
        else if (s == 1.0) status = OPEN;
        else return 202;
        s = link->Kc;
        break;
    case EN_PIPE:
    case EN_PUMP:
        status = OPEN;
        if (s == 0.0) status = CLOSED;
    default:
        break;
    }
    if (ctype == LOWLEVEL || ctype == HILEVEL)
    {
        if (nindex > net->Njuncs) lvl = net->Node[nindex].El + level / Ucf[ELEV];
        else lvl = net->Node[nindex].El + level / Ucf[PRESSURE];
    }
    if (ctype == TIMER) t = (long)ROUND(lvl);
    if (ctype == TIMEOFDAY) t = (long)ROUND(lvl) % SECperDAY;

    /* Reset control's parameters */
    control->Type = (char)ctype;
    control->Link = lindex;
    control->Node = nindex;
    control->Status = status;
    control->Setting = s;
    control->Grade = lvl;
    control->Time = t;
    return 0;
}

/********************************************************************

    Rule-Based Controls Functions

********************************************************************/

int DLLEXPORT EN_addrule(EN_Project p, char *rule)
/*----------------------------------------------------------------
**  Input:   rule = text of rule being added in the format
**           used for the [RULES] section of an EPANET input file
**  Output:  none
**  Returns: error code
**  Purpose: adds a new rule to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Parser  *parser = &p->parser;
    Rules   *rules = &p->rules;

    char *line;
    char *nextline;
    char line2[MAXLINE+1];

    // Resize rules array
    net->Rule = (Srule *)realloc(net->Rule, (net->Nrules + 2)*sizeof(Srule));
    rules->Errcode = 0;
    rules->RuleState = 6;  // = r_PRIORITY

    // Extract each line of the rule statement
    line = rule;
    while (line)
    {
        // Find where current line ends and next one begins
        nextline = strchr(line, '\n');
        if (nextline) *nextline = '\0';

        // Copy and tokenize the current line
        strcpy(line2, line);
        strcat(line2, "\n");  // Tokenizer won't work without this
        parser->Ntokens = gettokens(line2, parser->Tok, MAXTOKS, parser->Comment);

        // Process the line to build up the rule's contents
        if (parser->Ntokens > 0 && *parser->Tok[0] != ';')
        {
            ruledata(p);  // Nrules gets updated in ruledata()
            if (rules->Errcode) break;
        }

        // Extract next line from the rule statement
        if (nextline) *nextline = '\n';
        line = nextline ? (nextline + 1) : NULL;
    }

    // Delete new rule entry if there was an error
    if (rules->Errcode) deleterule(p, net->Nrules);

    // Re-assign error code 201 (syntax error) to 250 (invalid format)
    if (rules->Errcode == 201) rules->Errcode = 250;
    return rules->Errcode;
}

int DLLEXPORT EN_deleterule(EN_Project p, int index)
/*----------------------------------------------------------------
**  Input:   index = rule index
**  Output:  none
**  Returns: error code
**  Purpose: deletes a rule from a project.
**----------------------------------------------------------------
*/
{
    if (index < 1 || index > p->network.Nrules) return 257;
    deleterule(p, index);
    return 0;
}

int DLLEXPORT EN_getrule(EN_Project p, int index, int *nPremises,
                         int *nThenActions, int *nElseActions,
                         EN_API_FLOAT_TYPE *priority)
/*----------------------------------------------------------------
**  Input:   index  = rule index
**  Output:  nPremises  = number of premises conditions (IF AND OR)
**           nThenActions = number of actions in THEN portion of rule
**           nElseActions = number of actions in ELSE portion of rule
**           priority = rule priority
**  Returns: error code
**  Purpose: gets information about a particular rule
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int count;
    Spremise *premise;
    Saction *action;

    if (index < 1 || index > net->Nrules) return 257;
    *priority = (EN_API_FLOAT_TYPE)p->network.Rule[index].priority;

    count = 1;
    premise = net->Rule[index].Premises;
    while (premise->next != NULL)
    {
        count++;
        premise = premise->next;
    }
    *nPremises = count;

    count = 1;
    action = net->Rule[index].ThenActions;
    while (action->next != NULL)
    {
        count++;
        action = action->next;
    }
    *nThenActions = count;

    action = net->Rule[index].ElseActions;
    count = 0;
    if (action != NULL)
    {
        count = 1;
        while (action->next != NULL)
        {
            count++;
            action = action->next;
        }
    }
    *nElseActions = count;
    return 0;
}

int DLLEXPORT EN_getruleID(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = rule index
**  Output:  id = rule ID label
**  Returns: error code
**  Purpose: retrieves the ID label of a rule
**----------------------------------------------------------------
*/
{
    strcpy(id, "");
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nrules) return 257;
    strcpy(id, p->network.Rule[index].label);
    return 0;
}

int DLLEXPORT EN_getpremise(EN_Project p, int ruleIndex, int premiseIndex,
                           int *logop, int *object, int *objIndex, int *variable,
                           int *relop, int *status, EN_API_FLOAT_TYPE *value)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           premiseIndex = premise index
**  Output:  logop = logical operator (IF = 1, AND = 2, OR = 3)
**           object = type of object appearing in the premise (see EN_RuleObject)
**           objIndex = object's index in Node or Link array
**           variable = object variable being tested (see EN_RuleVariable)
**           relop = relational operator (see EN_RuleOperator)
**           status = object status being tested against (see EN_RuleStatus))
**           value = variable value being tested against
**  Returns: error code
**  Purpose: retrieves the properties of a rule's premise
**----------------------------------------------------------------
*/
{
    Spremise *premises;
    Spremise *premise;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    premises = p->network.Rule[ruleIndex].Premises;
    premise = getpremise(premises, premiseIndex);
    if (premise == NULL)  return 258;

    *logop = premise->logop;
    *object = premise->object;
    *objIndex = premise->index;
    *variable = premise->variable;
    *relop = premise->relop;
    *status = premise->status;
    *value = (EN_API_FLOAT_TYPE)premise->value;
    return 0;
}


int DLLEXPORT EN_setpremise(EN_Project p, int ruleIndex, int premiseIndex,
                            int logop, int object, int objIndex, int variable,
                            int relop, int status, EN_API_FLOAT_TYPE value)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           premiseIndex = premise index
**           logop = logical operator (IF = 1, AND = 2, OR = 3)
**           object = type of object appearing in the premise (see EN_RuleObject)
**           objIndex = object's index in Node or Link array
**           variable = object variable being tested (see EN_RuleVariable)
**           relop = relational operator (see EN_RuleOperator)
**           status = object status being tested against (see EN_RuleStatus))
**           value = variable value being tested against
**  Output:  none
**  Returns: error code
**  Purpose: sets the properties of a rule's premise
**----------------------------------------------------------------
*/
{
    Spremise *premises;
    Spremise *premise;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    premises = p->network.Rule[ruleIndex].Premises;
    premise = getpremise(premises, premiseIndex);
    if (premise == NULL)  return 258;

    premise->logop = logop;
    premise->object = object;
    premise->index = objIndex;
    premise->variable = variable;
    premise->relop = relop;
    premise->status = status;
    premise->value = value;
    return 0;
}

int DLLEXPORT EN_setpremiseindex(EN_Project p, int ruleIndex, int premiseIndex,
                                 int objIndex)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           premiseIndex = premise index
**           objIndex = object's index in Node or Link array
**  Output:  none
**  Returns: error code
**  Purpose: sets the index of an object referred to in a rule's premise
**----------------------------------------------------------------
*/
{
    Spremise *premises;
    Spremise *premise;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    premises = p->network.Rule[ruleIndex].Premises;
    premise = getpremise(premises, premiseIndex);
    if (premise == NULL)  return 258;

    premise->index = objIndex;
    return 0;
}

int DLLEXPORT EN_setpremisestatus(EN_Project p, int ruleIndex, int premiseIndex, int status)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           premiseIndex = premise index
**           status = object status being tested against
**                    (see EN_RuleStatus))
**  Output:  none
**  Returns: error code
**  Purpose: sets the status of an object being tested against
**           in a rule's premise
**----------------------------------------------------------------
*/
{
    Spremise *premises;
    Spremise *premise;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    premises = p->network.Rule[ruleIndex].Premises;
    premise = getpremise(premises, ruleIndex);
    if (premise == NULL) return 258;

    premise->status = status;
    return 0;
}

int DLLEXPORT EN_setpremisevalue(EN_Project p, int ruleIndex, int premiseIndex, EN_API_FLOAT_TYPE value)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           premiseIndex = premise index
**           value = value of object variable being tested against
**  Output:  none
**  Returns: error code
**  Purpose: sets the value of object's variable being tested against
**           in a rule's premise
**----------------------------------------------------------------
*/
{
    Spremise *premises;
    Spremise *premise;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    premises = p->network.Rule[ruleIndex].Premises;
    premise = getpremise(premises, premiseIndex);
    if (premise == NULL) return 258;

    premise->value = value;
    return 0;
}

int DLLEXPORT EN_getthenaction(EN_Project p, int ruleIndex, int actionIndex,
                               int *linkIndex, int *status, EN_API_FLOAT_TYPE *setting)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           actionIndex = index of a rule's THEN actions
**  Output:  linkIndex = index of link appearing in the action
**           status = status assigned to the link (see EN_RuleStatus))
**           setting = setting assigned to the link
**  Returns: error code
**  Purpose: retrieves the properties of a rule's THEN action
**----------------------------------------------------------------
*/
{
    Saction *actions;
    Saction *action;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    actions = p->network.Rule[ruleIndex].ThenActions;
    action = getaction(actions, actionIndex);
    if (action == NULL) return 258;

    *linkIndex = action->link;
    *status = action->status;
    *setting = (EN_API_FLOAT_TYPE)action->setting;
    return 0;
}

int DLLEXPORT EN_setthenaction(EN_Project p, int ruleIndex, int actionIndex,
                               int linkIndex,  int status, EN_API_FLOAT_TYPE setting)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           actionIndex = index of a rule's THEN actions
**           linkIndex = index of link appearing in the action
**           status = status assigned to the link (see EN_RuleStatus))
**           setting = setting assigned to the link
**  Returns: error code
**  Purpose: sets the properties of a rule's THEN action
**----------------------------------------------------------------
*/
{
    Saction *actions;
    Saction *action;

    if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

    actions = p->network.Rule[ruleIndex].ThenActions;
    action = getaction(actions, actionIndex);
    if (action == NULL) return 258;

    action->link = linkIndex;
    action->status = status;
    action->setting = setting;
    return 0;
}

int DLLEXPORT EN_getelseaction(EN_Project p, int ruleIndex, int actionIndex,
                               int *linkIndex, int *status, EN_API_FLOAT_TYPE *setting)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           actionIndex = index of a rule's ELSE actions
**  Output:  linkIndex = index of link appearing in the action
**           status = status assigned to the link (see EN_RuleStatus))
**           setting = setting assigned to the link
**  Returns: error code
**  Purpose: retrieves the properties of a rule's ELSE action
**----------------------------------------------------------------
*/
{

  Saction *actions;
  Saction *action;

  if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

  actions = p->network.Rule[ruleIndex].ThenActions;
  action = getaction(actions, actionIndex);
  if (action == NULL) return 258;

  *linkIndex = action->link;
  *status = action->status;
  *setting = (EN_API_FLOAT_TYPE)action->setting;
  return 0;
}

int DLLEXPORT EN_setelseaction(EN_Project p, int ruleIndex, int actionIndex,
                               int linkIndex,  int status, EN_API_FLOAT_TYPE setting)
/*----------------------------------------------------------------
**  Input:   ruleIndex = rule index
**           actionIndex = index of a rule's ELSE actions
**           linkIndex = index of link appearing in the action
**           status = status assigned to the link (see EN_RuleStatus))
**           setting = setting assigned to the link
**  Returns: error code
**  Purpose: sets the properties of a rule's ELSE action
**----------------------------------------------------------------
*/
{
  Saction *actions;
  Saction *action;

  if (ruleIndex < 1 || ruleIndex > p->network.Nrules) return 257;

  actions = p->network.Rule[ruleIndex].ThenActions;
  action = getaction(actions, actionIndex);
  if (action == NULL) return 258;

  action->link = linkIndex;
  action->status = status;
  action->setting = setting;
  return 0;
}

int DLLEXPORT EN_setrulepriority(EN_Project p, int index, EN_API_FLOAT_TYPE priority)
/*-----------------------------------------------------------------------------
**  Input:   index  = rule index
**           priority = rule priority level
**  Output:  none
**  Returns: error code
**  Purpose: sets the priority level for a rule
**-----------------------------------------------------------------------------
*/
{
    if (index <= 0 || index > p->network.Nrules)  return 257;
    p->network.Rule[index].priority = priority;
    return 0;
}
