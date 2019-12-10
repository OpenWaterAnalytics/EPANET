/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet.c
 Description:  implementation of EPANET's API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/15/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "epanet2_2.h"
#include "types.h"
#include "funcs.h"
#include "text.h"
#include "enumstxt.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

/********************************************************************

    Project Functions

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

int DLLEXPORT EN_deleteproject(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: deletes an EPANET project
**----------------------------------------------------------------
*/
{
    if (p == NULL) return -1;
    if (p->Openflag) {
      EN_close(p);
    }
    remove(p->TmpHydFname);
    remove(p->TmpOutFname);
    remove(p->TmpStatFname);
    free(p);
    return 0;
}

int DLLEXPORT EN_runproject(EN_Project p, const char *inpFile,
                            const char *rptFile, const char *outFile,
                            void (*pviewprog)(char *))
/*------------------------------------------------------------------------
**   Input:   inpFile = name of EPANET formatted input file
**            rptFile = name of report file
**            outFile = name of binary output file
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
    ERRCODE(EN_open(p, inpFile, rptFile, outFile));
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

int DLLEXPORT EN_init(EN_Project p, const char *rptFile, const char *outFile,
                      int unitsType, int headLossType)
/*----------------------------------------------------------------
 **  Input:   rptFile = name of report file
 **           outFile = name of binary output file
 **           unitsType = type of flow units (see FlowUnitsType)
 **           headLossType = type of head loss formula (see HeadLossType)
 **  Output:  none
 **  Returns: error code
 **  Purpose: initializes an EPANET project that isn't opened with
 **           an input file
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

    // Check for valid arguments
    if (unitsType < 0 || unitsType > CMD) return 251;
    if (headLossType < 0 || headLossType > CM) return 251;

    // Open files
    errcode = openfiles(p, "", rptFile, outFile);

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
    p->parser.MaxPats = 0;
    p->Openflag = TRUE;
    return errcode;
}

int DLLEXPORT EN_open(EN_Project p, const char *inpFile, const char *rptFile,
                      const char *outFile)
/*----------------------------------------------------------------
 **  Input:   inpFile = name of input file
 **           rptFile = name of report file
 **           outFile = name of binary output file
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
  ERRCODE(openfiles(p, inpFile, rptFile, outFile));
  if (errcode > 0)
  {
    errmsg(p, errcode);
    return errcode;
  }

  // Allocate memory for project's data arrays
  writewin(p->viewprog, FMT100);
  ERRCODE(netsize(p));
  ERRCODE(allocdata(p));

  // Read input data
  ERRCODE(getdata(p));

  // Close input file
  if (p->parser.InFile != NULL)
  {
      fclose(p->parser.InFile);
      p->parser.InFile = NULL;
  }

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

int DLLEXPORT EN_gettitle(EN_Project p, char *line1, char *line2, char *line3)
/*----------------------------------------------------------------
**  Input:   None
**  Output:  line1, line2, line3 = project's title lines
**  Returns: error code
**  Purpose: retrieves the title lines of the project
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    strncpy(line1, p->Title[0], TITLELEN);
    strncpy(line2, p->Title[1], TITLELEN);
    strncpy(line3, p->Title[2], TITLELEN);
    return 0;
}

int DLLEXPORT EN_settitle(EN_Project p, char *line1, char *line2, char *line3)
/*----------------------------------------------------------------
**  Input:  line1, line2, line3 = project's title lines
**  Returns: error code
**  Purpose: sets the title lines of the project
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    strncpy(p->Title[0], line1, TITLELEN);
    strncpy(p->Title[1], line2, TITLELEN);
    strncpy(p->Title[2], line3, TITLELEN);
    return 0;
}

int DLLEXPORT EN_getcomment(EN_Project p, int object, int index, char *comment)
/*----------------------------------------------------------------
**  Input:   object = a type of object (see EN_ObjectType)
**           index = the object's index
**  Output:  comment = the object's descriptive comment
**  Returns: error code
**  Purpose: Retrieves an object's descriptive comment
**----------------------------------------------------------------
*/
{
    return getcomment(&p->network, object, index, comment);
}

int  DLLEXPORT EN_setcomment(EN_Project p, int object, int index, char *comment)
/*----------------------------------------------------------------
**  Input:   object = a type of object (see EN_ObjectType)
**           index = the object's index
**           comment =  a descriptive comment to assign
**  Returns: error code
**  Purpose: Assigns a descriptive comment to an object
**----------------------------------------------------------------
*/
{
    return setcomment(&p->network, object, index, comment);
}

int DLLEXPORT EN_getcount(EN_Project p, int object, int *count)
/*----------------------------------------------------------------
**  Input:   object = type of object to count (see EN_CountType)
**  Output:  count = number of objects of the specified type
**  Returns: error code
**  Purpose: Retrieves number of network objects of a given type
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    *count = 0;
    if (!p->Openflag) return 102;
    switch (object)
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
 **  Purpose: frees all memory & files used by a project
 **----------------------------------------------------------------
 */
{
    // Free all project data
    if (p->Openflag) writetime(p, FMT105);
    freedata(p);

    // Close output file
    closeoutfile(p);

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
    if (p->outfile.HydFile != NULL)
    {
        fclose(p->outfile.HydFile);
        p->outfile.HydFile = NULL;
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
    int tmpflag;
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
 **  Purpose: opens a project's hydraulic solver
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

int DLLEXPORT EN_initH(EN_Project p, int initFlag)
/*----------------------------------------------------------------
 **  Input:   initFlag = 2-digit flag where 1st (left) digit indicates
 **                      if link flows should be re-initialized (1) or
 **                      not (0) and 2nd digit indicates if hydraulic
 **                      results should be saved to file (1) or not (0)
 **  Output:  none
 **  Returns: error code
 **  Purpose: initializes a project's hydraulic solver
 **----------------------------------------------------------------
 */
{
    int errcode = 0;
    int sflag, fflag;

    // Reset status flags
    p->outfile.SaveHflag = FALSE;
    p->Warnflag = FALSE;

    // Get values of save-to-file flag and reinitialize-flows flag
    fflag = initFlag / EN_INITFLOW;
    sflag = initFlag - fflag * EN_INITFLOW;

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

int DLLEXPORT EN_runH(EN_Project p, long *currentTime)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  currentTime = current elapsed time (sec)
**  Returns: error code
**  Purpose: solves network hydraulics at current time point
**----------------------------------------------------------------
*/
{
    int errcode;

    *currentTime = 0;
    if (!p->hydraul.OpenHflag) return 103;
    errcode = runhyd(p, currentTime);
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_nextH(EN_Project p, long *tStep)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  tStep = next hydraulic time step to take (sec)
**  Returns: error code
**  Purpose: determines the time step until the next hydraulic event
**----------------------------------------------------------------
*/
{
    int errcode;

    *tStep = 0;
    if (!p->hydraul.OpenHflag) return 103;
    errcode = nexthyd(p, tStep);
    if (errcode) errmsg(p, errcode);
    else if (p->outfile.Saveflag && *tStep == 0) p->outfile.SaveHflag = TRUE;
    return errcode;
}

int DLLEXPORT EN_closeH(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: closes a project's hydraulic solver
**----------------------------------------------------------------
*/
{
  if (!p->Openflag) return 102;
  if (p->hydraul.OpenHflag) closehyd(p);
  p->hydraul.OpenHflag = FALSE;
  return 0;
}

int DLLEXPORT EN_savehydfile(EN_Project p, const char *filename)
/*----------------------------------------------------------------
**  Input:   filename = name of file to which hydraulic results are saved
**  Output:  none
**  Returns: error code
**  Purpose: saves results from a scratch hydraulics file to a
**           permanent one
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

int DLLEXPORT EN_usehydfile(EN_Project p, const char *filename)
/*----------------------------------------------------------------
**  Input:   filename = name of previously saved hydraulics file
**  Output:  none
**  Returns: error code
**  Purpose: uses contents of a previously saved hydraulics file to
**           run a water quality analysis
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
**  Purpose: opens a project's water quality solver
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

int DLLEXPORT EN_initQ(EN_Project p, int saveFlag)
/*----------------------------------------------------------------
**  Input:   saveFlag = flag indicating if results should be saved
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
    if (saveFlag)
    {
        errcode = openoutfile(p);
        if (!errcode) p->outfile.Saveflag = TRUE;
    }
    return errcode;
}

int DLLEXPORT EN_runQ(EN_Project p, long *currentTime)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  currentTime = current simulation time (sec)
**  Returns: error code
**  Purpose: retrieves current hydraulic results and saves current
**           results to file.
**----------------------------------------------------------------
*/
{
    int errcode;

    *currentTime = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = runqual(p, currentTime);
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_nextQ(EN_Project p, long *tStep)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  tStep = time step over which water quality is updated (sec)
**  Returns: error code
**  Purpose: updates water quality throughout the network until
**           next hydraulic event occurs
**----------------------------------------------------------------
*/
{
    int errcode;

    *tStep = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = nextqual(p, tStep);
    if (!errcode && p->outfile.Saveflag && *tStep == 0)
    {
        p->outfile.SaveQflag = TRUE;
    }
    if (errcode) errmsg(p, errcode);
    return errcode;
}

int DLLEXPORT EN_stepQ(EN_Project p, long *timeLeft)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  timeLeft = amount of simulation time remaining (sec)
**  Returns: error code
**  Purpose: updates water quality throughout the network over
**           fixed time step
**----------------------------------------------------------------
*/
{
    int errcode;

    *timeLeft = 0;
    if (!p->quality.OpenQflag) return 105;
    errcode = stepqual(p, timeLeft);
    if (!errcode && p->outfile.Saveflag && *timeLeft == 0)
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
**  Purpose: closes a project's water quality solver
**----------------------------------------------------------------
*/
{
    if (!p->Openflag) return 102;
    closequal(p);
    p->quality.OpenQflag = FALSE;
    closeoutfile(p);
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
**  Purpose: write a line of text to a project's report file
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
**  Purpose: writes formatted simulation results to a project's
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

int  DLLEXPORT EN_copyreport(EN_Project p, char *filename)
/*----------------------------------------------------------------
**  Input:   filename = name of file to receive copy of report
**  Output:  none
**  Returns: error code
**  Purpose: copies the contents of a project's report file to
**           another file
**----------------------------------------------------------------
*/
{
    return copyreport(p, filename);
}

int DLLEXPORT EN_clearreport(EN_Project p)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: clears the contents of a project's report file
**----------------------------------------------------------------
*/
{
    return clearreport(p);
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

int DLLEXPORT EN_setreport(EN_Project p, char *format)
/*----------------------------------------------------------------
**  Input:   format = a report formatting command
**  Output:  none
**  Returns: error code
**  Purpose: sets a specific set of reporting options
**----------------------------------------------------------------
*/
{
    char s1[MAXLINE + 1];

    if (!p->Openflag) return 102;
    if (strlen(format) >= MAXLINE) return 250;
    strcpy(s1, format);
    strcat(s1, "\n");
    if (setreport(p, s1) > 0) return 250;
    else return 0;
}

int DLLEXPORT EN_setstatusreport(EN_Project p, int level)
/*----------------------------------------------------------------
**  Input:   level = level of reporting to use (see EN_StatusReport)
**  Output:  none
**  Returns: error code
**  Purpose: sets the level of hydraulic status reporting
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    if (level >= EN_NO_REPORT && level <= EN_FULL_REPORT)
    {
        p->report.Statflag = (char)level;
    }
    else errcode = 251;
    return errcode;
}

int DLLEXPORT EN_getversion(int *version)
/*----------------------------------------------------------------
**  Input:    none
**  Output:   version = version number of the source code
**  Returns:  error code (should always be 0)
**  Purpose:  retrieves the toolkit API version number
**
**  The version number is set by the constant CODEVERSION found in
**  TYPES.H and is to be interpreted with implied decimals, i.e.,
**  "20100" == "2(.)01(.)00".
**----------------------------------------------------------------
*/
{
    *version = CODEVERSION;
    return 0;
}

int DLLEXPORT EN_geterror(int errcode, char *errmsg, int maxLen)
/*----------------------------------------------------------------
**  Input:   errcode = an error or warnng code
**           maxLen = maximum characters that errmsg can hold
**  Output:  errmsg = text of error/warning message
**  Returns: error code
**  Purpose: retrieves the text of the message associated with
**           a particular error/warning code
**----------------------------------------------------------------
*/
{
    char msg1[MAXMSG+1] = "";
    char msg2[MAXMSG+1] = "";

    switch (errcode)
    {
    case 1:
        strncpy(errmsg, WARN1, maxLen);
        break;
    case 2:
        strncpy(errmsg, WARN2, maxLen);
        break;
    case 3:
        strncpy(errmsg, WARN3, maxLen);
        break;
    case 4:
        strncpy(errmsg, WARN4, maxLen);
        break;
    case 5:
        strncpy(errmsg, WARN5, maxLen);
        break;
    case 6:
        strncpy(errmsg, WARN6, maxLen);
        break;
    default:
        sprintf(msg1, "Error %d: ", errcode);
        if ((errcode >= 202 && errcode <= 222) ||
            (errcode >= 240 && errcode <= 261)) strcat(msg1, "function call contains ");
        snprintf(errmsg, maxLen, "%s%s", msg1, geterrmsg(errcode, msg2));
    }
    if (strlen(errmsg) == 0)
        return 251;
    else
        return 0;
}

int DLLEXPORT EN_getstatistic(EN_Project p, int type, double *value)
/*----------------------------------------------------------------
**  Input:   type = type of simulation statistic (see EN_AnalysisStatistic)
**  Output:  value = simulation analysis statistic value
**  Returns: error code
**  Purpose: retrieves the value of a simulation analysis statistic
**----------------------------------------------------------------
*/
{
    switch (type)
    {
    case EN_ITERATIONS:
        *value = p->hydraul.Iterations;
        break;
    case EN_RELATIVEERROR:
        *value = p->hydraul.RelativeError;
        break;
    case EN_MAXHEADERROR:
        *value = p->hydraul.MaxHeadError * p->Ucf[HEAD];
        break;
    case EN_MAXFLOWCHANGE:
        *value = p->hydraul.MaxFlowChange * p->Ucf[FLOW];
        break;
    case EN_DEFICIENTNODES:
        *value = p->hydraul.DeficientNodes;
        break;
    case EN_DEMANDREDUCTION:
        *value = p->hydraul.DemandReduction;
        break;
    case EN_MASSBALANCE:
        *value = p->quality.MassBalance.ratio;
        break;
    default:
        *value = 0.0;
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_getresultindex(EN_Project p, int type, int index, int *value)
/*----------------------------------------------------------------
**  Input:   type = type of object (either EN_NODE or EN_LINK)
**           index = the object's index
**  Output:  value = the order in which the object's results were saved
**  Returns: error code
**  Purpose: retrieves the order in which a node's or link's results
**           were saved to an output file.
**----------------------------------------------------------------
*/
{
    *value = 0;
    if (!p->Openflag) return 102;
    if (type == EN_NODE)
    {
        if (index <= 0 || index > p->network.Nnodes) return 203;
        *value = p->network.Node[index].ResultIndex;
    }
    else if (type == EN_LINK)
    {
        if (index <= 0 || index > p->network.Nlinks) return 204;
        *value = p->network.Link[index].ResultIndex;
    }
    else return 251;
    return 0;
}

/********************************************************************

    Analysis Options Functions

********************************************************************/

int DLLEXPORT EN_getoption(EN_Project p, int option, double *value)
/*----------------------------------------------------------------
**  Input:   option = an analysis option code (see EN_Option)
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
    switch (option)
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
        if (hyd->Qexp > 0.0) v = 1.0 / hyd->Qexp;
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
    case EN_HEADLOSSFORM:
        v = hyd->Formflag;
        break;
    case EN_GLOBALEFFIC:
        v = hyd->Epump;
        break;
    case EN_GLOBALPRICE:
        v = hyd->Ecost;
        break;
    case EN_GLOBALPATTERN:
        v = hyd->Epat;
        break;
    case EN_DEMANDCHARGE:
        v = hyd->Dcost;
        break;
    case EN_SP_GRAVITY:
        v = hyd->SpGrav;
        break;
    case EN_SP_VISCOS:
        v = hyd->Viscos / VISCOS;
        break;
    case EN_UNBALANCED:
        v = hyd->ExtraIter;
        break;
    case EN_CHECKFREQ:
        v = hyd->CheckFreq;
        break;
    case EN_MAXCHECK:
        v = hyd->MaxCheck;
        break;
    case EN_DAMPLIMIT:
        v = hyd->DampLimit;
        break;
    case EN_SP_DIFFUS:
        v = qual->Diffus / DIFFUS;
        break;
    case EN_BULKORDER:
        v = qual->BulkOrder;
        break;
    case EN_WALLORDER:
        v = qual->WallOrder;
        break;
    case EN_TANKORDER:
        v = qual->TankOrder;
        break;
    case EN_CONCENLIMIT:
        v = qual->Climit * p->Ucf[QUALITY];
        break;

    default:
        return 251;
    }
    *value = (double)v;
    return 0;
}

int DLLEXPORT EN_setoption(EN_Project p, int option, double value)
/*----------------------------------------------------------------
**  Input:   option  = analysis option code (see EN_Option)
**           value = analysis option value
**  Output:  none
**  Returns: error code
**  Purpose: sets the value for an analysis option
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;
    Quality *qual = &p->quality;

    int Njuncs = net->Njuncs;
    double *Ucf = p->Ucf;
    int i, j, pat;
    double Ke, n, ucf;

    if (!p->Openflag) return 102;

    // The EN_UNBALANCED option can be < 0 indicating that the simulation
    // should be halted if no convergence is reached in EN_TRIALS. Other
    // values set the number of additional trials to use with no more
    // link status changes to achieve convergence.
    if (option == EN_UNBALANCED)
    {
        hyd->ExtraIter = (int)value;
        if (hyd->ExtraIter < 0) hyd->ExtraIter = -1;
        return 0;
    }

    // All other option values must be non-negative
    if (value < 0.0) return 213;

    // Process the speficied option
    switch (option)
    {
    case EN_TRIALS:
        if (value < 1.0) return 213;
        hyd->MaxIter = (int)value;
        break;

    case EN_ACCURACY:
        if (value < 1.e-8 || value > 1.e-1) return 213;
        hyd->Hacc = value;
        break;

    case EN_TOLERANCE:
        qual->Ctol = value / Ucf[QUALITY];
        break;

    case EN_EMITEXPON:
        if (value <= 0.0) return 213;
        n = 1.0 / value;
        ucf = pow(Ucf[FLOW], n) / Ucf[PRESSURE];
        for (i = 1; i <= Njuncs; i++)
        {
            j = EN_getnodevalue(p, i, EN_EMITTER, &Ke);
            if (j == 0 && Ke > 0.0) net->Node[i].Ke = ucf / pow(Ke, n);
        }
        hyd->Qexp = n;
        break;

    case EN_DEMANDMULT:
        hyd->Dmult = value;
        break;

    case EN_HEADERROR:
        hyd->HeadErrorLimit = value / Ucf[HEAD];
        break;

    case EN_FLOWCHANGE:
        hyd->FlowChangeLimit = value / Ucf[FLOW];
        break;

    case EN_HEADLOSSFORM:
        // Can't change if hydraulic solver is open
        if (p->hydraul.OpenHflag) return 262;
        i = ROUND(value);
        if (i < HW || i > CM) return 213;
        hyd->Formflag = i;
        if (hyd->Formflag == HW) hyd->Hexp = 1.852;
        else hyd->Hexp = 2.0;
        break;

    case EN_GLOBALEFFIC:
        if (value <= 1.0 || value > 100.0) return 213;
        hyd->Epump = value;
        break;

    case EN_GLOBALPRICE:
        hyd->Ecost = value;
        break;

    case EN_GLOBALPATTERN:
        pat = ROUND(value);
        if (pat < 0 || pat > net->Npats) return 205;
        hyd->Epat = pat;
        break;

    case EN_DEMANDCHARGE:
        hyd->Dcost = value;
        break;

    case EN_SP_GRAVITY:
        if (value <= 0.0) return 213;
        Ucf[PRESSURE] *= (value / hyd->SpGrav);
        hyd->SpGrav = value;
        break;

    case EN_SP_VISCOS:
        if (value <= 0.0) return 213;
        hyd->Viscos = value * VISCOS;
        break;

    case EN_CHECKFREQ:
        hyd->CheckFreq = (int)value;
        break;

    case EN_MAXCHECK:
        hyd->MaxCheck = (int)value;
        break;

    case EN_DAMPLIMIT:
        hyd->DampLimit = value;
        break;

    case EN_SP_DIFFUS:
        qual->Diffus = value * DIFFUS;
        break;

    case EN_BULKORDER:
        qual->BulkOrder = value;
        break;

    case EN_WALLORDER:
        if (value == 0.0 || value == 1.0) qual->WallOrder = value;
        else return 213;
        break;

    case EN_TANKORDER:
        qual->TankOrder = value;
        break;

    case EN_CONCENLIMIT:
        qual->Climit = value / p->Ucf[QUALITY];
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_getflowunits(EN_Project p, int *units)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  units = flow units code (see EN_FlowUnits)
**  Returns: error code
**  Purpose: retrieves the flow units used by a project
**----------------------------------------------------------------
*/
{
    *units = -1;
    if (!p->Openflag) return 102;
    *units = p->parser.Flowflag;
    return 0;
}

int DLLEXPORT EN_setflowunits(EN_Project p, int units)
/*----------------------------------------------------------------
**  Input:   units = flow units code (see EN_FlowUnits)
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

    p->parser.Flowflag = units;
    switch (units)
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
        case VOLUME_CURVE:
            xfactor = efactor / Ucf[ELEV];
            yfactor = vfactor / Ucf[VOLUME];
            break;
        case HLOSS_CURVE:
        case PUMP_CURVE:
            xfactor = qfactor / Ucf[FLOW];
            yfactor = hfactor / Ucf[HEAD];
            break;
        case EFFIC_CURVE:
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

int DLLEXPORT EN_gettimeparam(EN_Project p, int param, long *value)
/*----------------------------------------------------------------
**  Input:   param = time parameter code (see EN_TimeParameter)
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
    if (param < EN_DURATION || param > EN_NEXTEVENTTANK) return 251;
    switch (param)
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
    case EN_RULESTEP:
        *value = time->Rulestep;
        break;
    case EN_STATISTIC:
        *value = rpt->Tstatflag;
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
    case EN_QTIME:
        *value = time->Qtime;
    case EN_HALTFLAG:
        break;
    case EN_NEXTEVENT:
        *value = time->Hstep; // find the lesser of the hydraulic time step length,
                              // or the time to next full/empty tank
        tanktimestep(p, value);
        break;
    case EN_NEXTEVENTTANK:
        *value = time->Hstep;
        i = tanktimestep(p, value);
        *value = i;
        break;
    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_settimeparam(EN_Project p, int param, long value)
/*----------------------------------------------------------------
**  Input:   param = time parameter code (see EN_TimeParameter)
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
    if (value < 0) return 213;
    switch (param)
    {
    case EN_DURATION:
        time->Dur = value;
        if (time->Rstart > time->Dur) time->Rstart = 0;
        break;

    case EN_HYDSTEP:
        if (value == 0) return 213;
        time->Hstep = value;
        time->Hstep = MIN(time->Pstep, time->Hstep);
        time->Hstep = MIN(time->Rstep, time->Hstep);
        time->Qstep = MIN(time->Qstep, time->Hstep);
        break;

    case EN_QUALSTEP:
        if (value == 0) return 213;
        time->Qstep = value;
        time->Qstep = MIN(time->Qstep, time->Hstep);
        break;

    case EN_PATTERNSTEP:
        if (value == 0) return 213;
        time->Pstep = value;
        if (time->Hstep > time->Pstep) time->Hstep = time->Pstep;
        break;

    case EN_PATTERNSTART:
        time->Pstart = value;
        break;

    case EN_REPORTSTEP:
        if (value == 0) return 213;
        time->Rstep = value;
        if (time->Hstep > time->Rstep) time->Hstep = time->Rstep;
        break;

    case EN_REPORTSTART:
        if (time->Rstart > time->Dur) return 213;
        time->Rstart = value;
        break;

    case EN_RULESTEP:
        if (value == 0) return 213;
        time->Rulestep = value;
        time->Rulestep = MIN(time->Rulestep, time->Hstep);
        break;

    case EN_STATISTIC:
        if (value > RANGE) return 213;
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

int DLLEXPORT EN_getqualinfo(EN_Project p, int *qualType, char *chemName,
                             char *chemUnits, int *traceNode)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  qualType = type of quality analysis to run (see EN_QualityType)
**           chemName = name of chemical constituent
**           chemUnits = concentration units of constituent
**           traceNode = index of node being traced (if applicable)
**  Returns: error code
**  Purpose: retrieves water quality analysis options
**----------------------------------------------------------------
*/
{
    EN_getqualtype(p, qualType, traceNode);
    if (p->quality.Qualflag == CHEM)
    {
        strncpy(chemName, p->quality.ChemName, MAXID);
        strncpy(chemUnits, p->quality.ChemUnits, MAXID);
    }
    else if (p->quality.Qualflag == TRACE)
    {
        strncpy(chemName, w_TRACE, MAXID);
        strncpy(chemUnits, u_PERCENT, MAXID);
    }
    else if (p->quality.Qualflag == AGE)
    {
        strncpy(chemName, w_AGE, MAXID);
        strncpy(chemUnits, u_HOURS, MAXID);
    }
    else
    {
        strncpy(chemName, "", MAXID);
        strncpy(chemUnits, "", MAXID);
    }
    return 0;
}

int DLLEXPORT EN_getqualtype(EN_Project p, int *qualType, int *traceNode)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  qualType = type of quality analysis to run (see EN_QualityType)
**           traceNode = index of node being traced (for qualType = EN_TRACE)
**  Output:  none
**  Returns: error code
**  Purpose: retrieves type of quality analysis being made
**----------------------------------------------------------------
*/
{
    *traceNode = 0;
    if (!p->Openflag) return 102;
    *qualType = p->quality.Qualflag;
    if (p->quality.Qualflag == TRACE) *traceNode = p->quality.TraceNode;
    return 0;
}

int DLLEXPORT EN_setqualtype(EN_Project p, int qualType, char *chemName,
                             char *chemUnits, char *traceNode)
/*----------------------------------------------------------------
**  Input:   qualType = type of quality analysis to run (see EN_QualityType)
**           chemname = name of chemical constituent
**           chemunits = concentration units of constituent
**           tracenode = ID name of node being traced (if applicable)
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
    int i, oldQualFlag, traceNodeIndex;
    double ccf = 1.0;

    if (!p->Openflag) return 102;
    if (qual->OpenQflag) return 262;
    if (qualType < NONE || qualType > TRACE) return 251;
    if (qualType == TRACE)
    {
        traceNodeIndex = findnode(net, traceNode);
        if (traceNodeIndex == 0) return 212;
    }

    oldQualFlag = qual->Qualflag;
    qual->Qualflag = qualType;
    qual->Ctol *= Ucf[QUALITY];
    if (qual->Qualflag == CHEM) // Chemical analysis
    {
        strncpy(qual->ChemName, chemName, MAXID);
        strncpy(qual->ChemUnits, chemUnits, MAXID);
        strncpy(rpt->Field[QUALITY].Units, qual->ChemUnits, MAXID);
        strncpy(rpt->Field[REACTRATE].Units, qual->ChemUnits, MAXID);
        strcat(rpt->Field[REACTRATE].Units, t_PERDAY);
        ccf = 1.0 / LperFT3;
    }
    if (qual->Qualflag == TRACE) // Source trace analysis
    {
        qual->TraceNode = findnode(net, traceNode);
        if (qual->TraceNode == 0) return 212;
        strncpy(qual->ChemName, w_TRACE, MAXID);
        strncpy(qual->ChemUnits, u_PERCENT, MAXID);
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
    if ((qual->Qualflag == AGE || qual->Qualflag == TRACE) && oldQualFlag == CHEM)
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

int DLLEXPORT EN_addnode(EN_Project p, char *id, int nodeType, int *index)
/*----------------------------------------------------------------
**  Input:   id = node ID name
**           nodeType = type of node (see EN_NodeType)
**  Output:  index = index of newly added node
**  Returns: error code
**  Purpose: adds a new node to a project
**----------------------------------------------------------------
*/
{
    Network  *net = &p->network;
    Hydraul  *hyd = &p->hydraul;
    Quality  *qual = &p->quality;

    int i, nIdx, size;
    Stank *tank;
    Snode *node;
    Scontrol *control;

    // Cannot modify network structure while solvers are active
    *index = 0;
    if (!p->Openflag) return 102;
    if (hyd->OpenHflag || qual->OpenQflag) return 262;

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    // Check if a node with same id already exists
    if (EN_getnodeindex(p, id, &i) == 0) return 215;
    
    // Check for valid node type
    if (nodeType < EN_JUNCTION || nodeType > EN_TANK) return 251; 

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
        // shift indices of non-Junction nodes at end of Node array
        for (i = net->Nnodes; i > net->Njuncs; i--)
        {
            hashtable_update(net->NodeHashTable, net->Node[i].ID, i + 1);
            net->Node[i + 1] = net->Node[i];
        }
    
        // set index of new Junction node
        net->Njuncs++;
        nIdx = net->Njuncs;
        node = &net->Node[nIdx];
        node->D = NULL;
        adddemand(node, 0.0, 0, NULL);

        // shift indices of Tank array
        for (i = 1; i <= net->Ntanks; i++)
        {
            net->Tank[i].Node += 1;
        }
        // shift indices of Links, if necessary
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].N1 > net->Njuncs - 1) net->Link[i].N1 += 1;
            if (net->Link[i].N2 > net->Njuncs - 1) net->Link[i].N2 += 1;
        }
        // shift indices of tanks/reservoir nodes in controls
        for (i = 1; i <= net->Ncontrols; ++i)
        {
            control = &net->Control[i];
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
        node->D = NULL;
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
        tank->CanOverflow = FALSE;
    }
    net->Nnodes++;
    p->parser.MaxNodes = net->Nnodes;
    strncpy(node->ID, id, MAXID);

    // set default values for new node
    node->Type = nodeType;
    node->El = 0;
    node->S = NULL;
    node->C0 = 0;
    node->Ke = 0;
    node->Rpt = 0;
    node->ResultIndex = 0;
    node->X = MISSING;
    node->Y = MISSING;
    node->Comment = NULL;

    // Insert new node into hash table
    hashtable_insert(net->NodeHashTable, node->ID, nIdx);
    *index = nIdx;
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
**  Purpose: deletes a node from a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i, nodeType, tankindex;
    Snode *node;

    // Cannot modify network structure while solvers are active
    if (!p->Openflag) return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check that node exists
    if (index <= 0 || index > net->Nnodes) return 203;
    if (actionCode < EN_UNCONDITIONAL || actionCode > EN_CONDITIONAL) return 251;

    // Can't delete a water quality trace node
    if (index == p->quality.TraceNode) return 260;

    // Do not delete a node contained in a control or is connected to a link
    if (actionCode == EN_CONDITIONAL)
    {
        if (incontrols(p, NODE, index)) return 261;
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].N1 == index ||
                net->Link[i].N2 == index)  return 259;
        }
    }

    // Get a reference to the node & its type
    node = &net->Node[index];
    EN_getnodetype(p, index, &nodeType);

    // Remove node from its hash table
    hashtable_delete(net->NodeHashTable, node->ID);

    // Free memory allocated to node's demands, WQ source & comment
    freedemands(node);
    free(node->S);
    free(node->Comment);

    // Shift position of higher entries in Node & Coord arrays down one
    for (i = index; i <= net->Nnodes - 1; i++)
    {
        net->Node[i] = net->Node[i + 1];
        // ... update node's entry in the hash table
        hashtable_update(net->NodeHashTable, net->Node[i].ID, i);
    }

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

    // Check for valid arguments
    if (index <= 0 || index > net->Nnodes) return 203;
    if (!namevalid(newid)) return 252;

    // Check if another node with same name exists
    if (hashtable_find(net->NodeHashTable, newid) > 0) return 215;

    // Replace the existing node ID with the new value
    hashtable_delete(net->NodeHashTable, net->Node[index].ID);
    strncpy(net->Node[index].ID, newid, MAXID);
    hashtable_insert(net->NodeHashTable, net->Node[index].ID, index);
    return 0;
}

int DLLEXPORT EN_getnodetype(EN_Project p, int index, int *nodeType)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  nodeType  = node type (see EN_NodeType)
**  Returns: error code
**  Purpose: retrieves the type of a node
**----------------------------------------------------------------
*/
{
    *nodeType = -1;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nnodes) return 203;
    if (index <= p->network.Njuncs) *nodeType = EN_JUNCTION;
    else
    {
        if (p->network.Tank[index - p->network.Njuncs].A == 0.0)
        {
            *nodeType = EN_RESERVOIR;
        }
        else *nodeType = EN_TANK;
    }
    return 0;
}

int DLLEXPORT EN_getnodevalue(EN_Project p, int index, int property, double *value)
/*----------------------------------------------------------------
**  Input:   index = node index
**           property = node property code (see EN_NodeProperty)
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
    Psource source;

    Snode *Node = net->Node;
    Stank *Tank = net->Tank;

    int nJuncs = net->Njuncs;

    double *Ucf = p->Ucf;
    double *NodeHead = hyd->NodeHead;
    double *NodeQual = qual->NodeQual;

    // Check for valid arguments
    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nnodes) return 203;

    // Retrieve requested property
    switch (property)
    {
    case EN_ELEVATION:
        v = Node[index].El * Ucf[ELEV];
        break;

    case EN_BASEDEMAND:
        // NOTE: primary demand category is first on demand list
        if (index <= nJuncs)
        {
            if (Node[index].D) v = Node[index].D->Base * Ucf[FLOW];
        }
        break;

    case EN_PATTERN:
        // NOTE: primary demand category is first on demand list
        if (index <= nJuncs)
        {
            if (Node[index].D) v = (double)(Node[index].D->Pat);
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
        if (property == EN_SOURCEQUAL) v = source->C0;
        else if (property == EN_SOURCEMASS) v = source->Smass * 60.0;
        else if (property == EN_SOURCEPAT)  v = source->Pat;
        else v = source->Type;
        break;

    case EN_TANKLEVEL:
        if (index <= nJuncs) return 0;
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
        v = hyd->NodeDemand[index] * Ucf[FLOW];
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
        if (index <= nJuncs) return 0;
        v = tankvolume(p, index - nJuncs, NodeHead[index]) * Ucf[VOLUME];
        break;

    case EN_CANOVERFLOW:
        if (Node[index].Type != TANK) return 0;
        v = Tank[index - nJuncs].CanOverflow;
        break;
        
    case EN_DEMANDDEFICIT:
        if (index > nJuncs) return 0;
        // After an analysis, DemandFlow contains node's required demand
        // while NodeDemand contains delivered demand + emitter flow
        if (hyd->DemandFlow[index] < 0.0) return 0;
        v = (hyd->DemandFlow[index] - 
            (hyd->NodeDemand[index] - hyd->EmitterFlow[index])) * Ucf[FLOW];
        break;
        
    default:
        return 251;
    }
    *value = v;
    return 0;
}

int DLLEXPORT EN_setnodevalue(EN_Project p, int index, int property, double value)
/*----------------------------------------------------------------
**  Input:   index = node index
**           property  = node property code (see EN_NodeProperty)
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
    Scurve *curve;

    const int nNodes = net->Nnodes;
    const int nJuncs = net->Njuncs;
    const int nPats = net->Npats;

    double *Ucf = p->Ucf;

    int i, j, n;
    Psource source;
    double hTmp;
    double vTmp;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > nNodes) return 203;
    switch (property)
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
        // NOTE: primary demand category is first on demand list
        if (index <= nJuncs)
        {
            if (Node[index].D) Node[index].D->Base = value / Ucf[FLOW];
        }
        break;

    case EN_PATTERN:
        // NOTE: primary demand category is first on demand list
        j = ROUND(value);
        if (j < 0 || j > nPats) return 205;
        if (index <= nJuncs)
        {
            if (Node[index].D) Node[index].D->Pat = j;
        }
        else Tank[index - nJuncs].Pat = j;
        break;

    case EN_EMITTER:
        if (index > nJuncs) return 0;
        if (value < 0.0) return 209;
        if (value > 0.0) value = pow((Ucf[FLOW] / value), hyd->Qexp) / Ucf[PRESSURE];
        Node[index].Ke = value;
        break;

    case EN_INITQUAL:
        if (value < 0.0) return 209;
        Node[index].C0 = value / Ucf[QUALITY];
        if (index > nJuncs) Tank[index - nJuncs].C = Node[index].C0;
        break;

    case EN_SOURCEQUAL:
    case EN_SOURCETYPE:
    case EN_SOURCEPAT:
        if (value < 0.0) return 209;
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
        if (property == EN_SOURCEQUAL) source->C0 = value;
        else if (property == EN_SOURCEPAT)
        {
            j = ROUND(value);
            if (j < 0 || j > nPats) return 205;
            source->Pat = j;
        }
        else // property == EN_SOURCETYPE
        {
            j = ROUND(value);
            if (j < CONCEN || j > FLOWPACED) return 251;
            else source->Type = (char)j;
        }
        break;

    case EN_TANKLEVEL:
        if (index <= nJuncs) return 0;
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
            if (value > Tank[j].Hmax || value < Tank[j].Hmin) return 225;
            Tank[j].H0 = value;
            Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
            // Resetting Volume in addition to initial volume
            Tank[j].V = Tank[j].V0;
            hyd->NodeHead[index] = Tank[j].H0;
        }
        break;

    case EN_TANKDIAM:
        if (value <= 0.0) return 209;                  // invalid diameter
        if (index <= nJuncs) return 0;                 // node is not a tank
        j = index - nJuncs;                            // tank index
        if (Tank[j].A == 0.0) return 0;                // tank is a reservoir
        value /= Ucf[ELEV];                            // diameter in feet
        Tank[j].A = PI * SQR(value) / 4.0;             // new tank area
        if (Tank[j].Vcurve > 0)                        // tank has a volume curve
        {
            Tank[j].Vcurve = 0;                        // remove volume curve

            // Since the volume curve no longer applies we assume that the tank's
            // shape below Hmin is cylindrical and Vmin equals area times Hmin
            Tank[j].Vmin = Tank[j].A * Tank[j].Hmin;
        }
        // Since tank's area has changed its volumes must be updated
        // NOTE: For a non-volume curve tank we can't change the Vmin
        //       associated with a Hmin since we don't know the tank's
        //       shape below Hmin. Vmin can always be changed by setting
        //       EN_MINVOLUME in a subsequent function call.
        Tank[j].V0 = tankvolume(p, j, Tank[j].H0);     // new init. volume
        vTmp = Tank[j].Vmax;                           // old max. volume
        Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax); // new max. volume
        Tank[j].V1max *= Tank[j].Vmax / vTmp;          // new mix zone volume
        break;

    case EN_MINVOLUME:
        if (value < 0.0) return 209;               // invalid volume
        if (index <= nJuncs) return 0;             // node is not a tank
        j = index - nJuncs;                        // tank index
        if (Tank[j].A == 0.0) return 0;            // tank is a reservoir
        i = Tank[j].Vcurve;                        // volume curve index
        if (i > 0)                                 // tank has a volume curve
        {
            curve = &net->Curve[i];                // curve object
            if (value < curve->Y[0]) return 225;   // volume off of curve
            value /= Ucf[VOLUME];                  // volume in ft3
            hTmp = tankgrade(p, j, value);         // head at given volume
            if (hTmp > Tank[j].H0 ||
                hTmp > Tank[j].Hmax) return 225;   // invalid water levels
            Tank[j].Hmin = hTmp;                   // new min. head
            Tank[j].Vmin = value;                  // new min. volume
        }
        else                                       // tank has no volume curve
        {
            // If the volume supplied by the function is 0 then the tank shape
            // below Hmin is assumed to be cylindrical and a new Vmin value is
            // computed. Otherwise Vmin is set to the supplied value.
            if (value == 0.0) Tank[j].Vmin = Tank[j].A * Tank[j].Hmin;
            else Tank[j].Vmin = value / Ucf[VOLUME];

            // Since Vmin changes the other volumes need updating
            Tank[j].V0 = tankvolume(p, j, Tank[j].H0);     // new init. volume
            vTmp = Tank[j].Vmax;                           // old max. volume
            Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax); // new max. volume
            Tank[j].V1max *= Tank[j].Vmax / vTmp;          // new mix zone volume
        }
        break;

    case EN_VOLCURVE:
        // NOTE: Setting EN_VOLCURVE to 0 to remove a volume curve is not valid.
        //       One should instead set a value for EN_TANKDIAM.
        i = ROUND(value);                          // curve index
        if (i <= 0 ||
            i > net->Ncurves) return 205;          // invalid curve index
        if (index <= nJuncs) return 0;             // node not a tank
        j = index - nJuncs;                        // tank index
        if (Tank[j].A == 0.0) return 0;            // tank is a reservoir
        curve = &net->Curve[i];                    // curve object

        // Check that tank's min/max levels lie within curve
        value = (Tank[j].Hmin - Node[index].El) * Ucf[ELEV];
        if (value < curve->X[0]) return 225;
        value = (Tank[j].Hmax - Node[index].El) * Ucf[ELEV];
        n = curve->Npts - 1;
        if (value > curve->X[n]) return 225;

        Tank[j].Vcurve = i;                            // assign curve to tank
        Tank[j].Vmin = tankvolume(p, j, Tank[j].Hmin); // new min. volume
        Tank[j].V0 = tankvolume(p, j, Tank[j].H0);     // new init. volume
        vTmp = Tank[j].Vmax;                           // old max. volume
        Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax); // new max. volume
        Tank[j].V1max *= Tank[j].Vmax / vTmp;          // new mix zone volume
        Tank[j].A = (curve->Y[n] - curve->Y[0]) /      // nominal area 
            (curve->X[n] - curve->X[0]);
        break;

    case EN_MINLEVEL:
        if (value < 0.0) return 209;               // invalid water level
        if (index <= nJuncs) return 0;             // node not a tank
        j = index - nJuncs;                        // tank index
        if (Tank[j].A == 0.0) return  0;           // tank is a reservoir
        hTmp = value / Ucf[ELEV] + Node[index].El; // convert level to head
        if (hTmp >= Tank[j].Hmax ||
            hTmp > Tank[j].H0) return 225;         // invalid water levels
        i = Tank[j].Vcurve;                        // volume curve index
        if (i > 0)                                 // tank has a volume curve
        {
            curve = &net->Curve[i];
            if (value < curve->X[0]) return 225;   // new level is off curve
            Tank[j].Vmin = tankvolume(p, j, hTmp); // new min. volume
        }
        Tank[j].Hmin = hTmp;
        // NOTE: We assume that for non-volume curve tanks Vmin doesn't change
        //       with Hmin. If not the case then a subsequent call setting
        //       EN_MINVOLUME must be made.
        break;

    case EN_MAXLEVEL:
        if (value <= 0.0) return 209;              // invalid water level
        if (index <= nJuncs) return 0;             // node not a tank
        j = index - nJuncs;                        // tank index
        if (Tank[j].A == 0.0) return 0;            // tank is a reservoir
        hTmp = value / Ucf[ELEV] + Node[index].El; // convert level to head
        if (hTmp < Tank[j].Hmin ||
            hTmp < Tank[j].H0) return 225;         // invalid water levels
        i = Tank[j].Vcurve;                        // volume curve index
        if (i > 0)                                 // tank has a volume curve
        {
            curve = &net->Curve[i];
            n = curve->Npts - 1;                   // last point on curve
            if (value > curve->X[n]) return 225;   // new level is off curve
        }
        Tank[j].Hmax = hTmp;                       // new max. head
        vTmp = Tank[j].Vmax;                       // old max. volume
        Tank[j].Vmax = tankvolume(p, j, hTmp);     // new max. volume
        Tank[j].V1max *= Tank[j].Vmax / vTmp;      // new mix zone volume
        break;

    case EN_MIXMODEL:
        j = ROUND(value);
        if (index <= nJuncs) return 0;
        if (j < MIX1 || j > LIFO) return 251;
        if (Tank[index - nJuncs].A > 0.0)
        {
            Tank[index - nJuncs].MixModel = (char)j;
        }
        break;

    case EN_MIXFRACTION:
        if (index <= nJuncs) return 0;
        if (value < 0.0 || value > 1.0) return 209;
        j = index - nJuncs;
        if (Tank[j].A > 0.0)
        {
            Tank[j].V1max = value * Tank[j].Vmax;
        }
        break;

    case EN_TANK_KBULK:
        if (index <= nJuncs) return 0;
        j = index - nJuncs;
        if (Tank[j].A > 0.0)
        {
            Tank[j].Kb = value / SECperDAY;
            qual->Reactflag = 1;
        }
        break;

    case EN_CANOVERFLOW:
        if (Node[index].Type != TANK) return 0;
        Tank[index - nJuncs].CanOverflow = (value != 0.0);
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_setjuncdata(EN_Project p, int index, double elev,
                             double dmnd, char *dmndpat)
/*----------------------------------------------------------------
**  Input:   index = junction node index
**           elev = junction elevation
**           dmnd = junction primary base demand
**           dmndpat = name of primary demand time pattern
**  Output:  none
**  Returns: error code
**  Purpose: sets several properties for a junction node
**----------------------------------------------------------------
*/
{
    int patIndex = 0;
    Snode *node;

    // Check that junction exists
    if (!p->Openflag) return 102;
    if (index <= 0 || index > p->network.Njuncs) return 203;

    // Check that demand pattern exists
    if (dmndpat && strlen(dmndpat) > 0)
    {
        if (EN_getpatternindex(p, dmndpat, &patIndex) > 0) return 205;
    }

    // Assign demand parameters to junction's primary demand category
    node = &(p->network.Node[index]);
    dmnd /= p->Ucf[FLOW];
    // Category exists - update its properties
    if (node->D)
    {
        (node->D)->Base = dmnd;
        (node->D)->Pat = patIndex;
    }
    // No demand categories exist -- create a new one
    else if (!adddemand(node, dmnd, patIndex, NULL)) return 101;

    // Assign new elevation value to junction
    node->El = elev / p->Ucf[ELEV];
    return 0;
}

int DLLEXPORT EN_settankdata(EN_Project p, int index, double elev,
                             double initlvl, double minlvl,
                             double maxlvl, double diam,
                             double minvol, char *volcurve)
/*----------------------------------------------------------------
**  Input:   index = tank node index
**           elev = tank bottom elevation
**           initlvl = initial water depth
**           minlvl = minimum water depth
**           maxlvl = maximum water depth
**           diam = tank diameter
**           minvol = tank volume at minimum level
**           volCurve = name of curve for volume v. level
**  Output:  none
**  Returns: error code
**  Purpose: sets several properties for a tank node
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i, j, n, curveIndex = 0;
    double area, elevation = elev;
    double *Ucf = p->Ucf;
    Stank *Tank = net->Tank;
    Scurve *curve;

    // Check that tank exists
    if (!p->Openflag) return 102;
    if (index <= net->Njuncs || index > net->Nnodes) return 203;
    j = index - net->Njuncs;
    if (Tank[j].A == 0) return 0;  // Tank is a Reservoir

    // Check for valid parameter values
    if (initlvl < 0.0 || minlvl < 0.0 || maxlvl < 0.0) return 209;
    if (minlvl > initlvl || minlvl > maxlvl || initlvl > maxlvl) return 225;
    if (diam < 0.0 || minvol < 0.0) return 209;

    // volume curve supplied
    if (strlen(volcurve) > 0)
    {
        for (i = 1; i <= net->Ncurves; i++)
        {
            if (strcmp(volcurve, net->Curve[i].ID) == 0)
            {
                curveIndex = i;
                break;
            }
        }
        if (curveIndex == 0) return 206;
        curve = &net->Curve[curveIndex];
        n = curve->Npts - 1;
        if (minlvl < curve->X[0] || maxlvl > curve->X[n]) return 225;
        area = (curve->Y[n] - curve->Y[0]) / (curve->X[n] - curve->X[0]);
    }

    // Tank diameter supplied
    else area = PI * diam * diam / 4.0;

    // Assign parameters to tank object
    net->Node[Tank[j].Node].El = elevation;
    Tank[j].A = area / Ucf[ELEV] / Ucf[ELEV];
    Tank[j].H0 = elevation + initlvl / Ucf[ELEV];
    Tank[j].Hmin = elevation + minlvl / Ucf[ELEV];
    Tank[j].Hmax = elevation + maxlvl / Ucf[ELEV];
    Tank[j].Vcurve = curveIndex;
    if (curveIndex == 0)
    {
        if (minvol > 0.0) Tank[j].Vmin = minvol / Ucf[VOLUME];
        else Tank[j].Vmin = Tank[j].A * Tank[j].Hmin;
    }
    else Tank[j].Vmin = tankvolume(p, j, Tank[j].Hmin);
    Tank[j].V0 = tankvolume(p, j, Tank[j].H0);
    Tank[j].Vmax = tankvolume(p, j, Tank[j].Hmax);
    return 0;
}

int DLLEXPORT EN_getcoord(EN_Project p, int index, double *x, double *y)
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

    // check if node has coords
    node = &net->Node[index];
    if (node->X == MISSING ||
        node->Y == MISSING) return 254;

    *x = (double)(node->X);
    *y = (double)(node->Y);
    return 0;
}

int DLLEXPORT EN_setcoord(EN_Project p, int index, double x, double y)
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

int DLLEXPORT EN_getdemandmodel(EN_Project p, int *model, double *pmin,
                                double *preq, double *pexp)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  model = type of demand model (see EN_DemandModel)
**           pmin = minimum pressure for any demand
**           preq = required pressure for full demand
**           pexp = exponent in pressure dependent demand formula
**  Returns: error code
**  Purpose: retrieves the parameters of a project's demand model
**----------------------------------------------------------------
*/
{
    *model = p->hydraul.DemandModel;
    *pmin = p->hydraul.Pmin * p->Ucf[PRESSURE];
    *preq = p->hydraul.Preq * p->Ucf[PRESSURE];
    *pexp = p->hydraul.Pexp;
    return 0;
}

int DLLEXPORT EN_setdemandmodel(EN_Project p, int model, double pmin,
                                double preq, double pexp)
/*----------------------------------------------------------------
**  Input:   model = type of demand model (see EN_DemandModel)
**           pmin = minimum pressure for any demand
**           preq = required pressure for full demand
**           pexp = exponent in pressure dependent demand formula
**  Output:  none
**  Returns: error code
**  Purpose: sets the parameters of a project's demand model
**----------------------------------------------------------------
*/
{
    if (model < 0 || model > EN_PDA) return 251;
    if (model == EN_PDA)
    {
        if (pexp <= 0.0) return 208;
        if (pmin < 0.0) return 208;
        if (preq - pmin < MINPDIFF) return 208;
    }
    p->hydraul.DemandModel = model;
    p->hydraul.Pmin = pmin / p->Ucf[PRESSURE];
    p->hydraul.Preq = preq / p->Ucf[PRESSURE];
    p->hydraul.Pexp = pexp;
    return 0;
}

int  DLLEXPORT EN_adddemand(EN_Project p, int nodeIndex, double baseDemand,
                            char *demandPattern, char *demandName)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           baseDemand = baseline demand value
**           demandPattern = name of demand's time pattern (can be NULL or empty)
**           demandName = name of demand's category (can be NULL or empty)
**  Returns: error code
**  Purpose: adds a new demand category to a junction node
**----------------------------------------------------------------
*/
{
    int patIndex = 0;
    Snode *node;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;
    if (demandPattern && strlen(demandPattern) > 0)
    {
        if (EN_getpatternindex(p, demandPattern, &patIndex) > 0) return 205;
    }

    // Do nothing if node is not a junction
    if (nodeIndex > p->network.Njuncs) return 0;

    // Add the new demand to the node's demands list
    node = &(p->network.Node[nodeIndex]);
    if (!adddemand(node, baseDemand / p->Ucf[FLOW], patIndex, demandName)) return 101;
    return 0;
}

int DLLEXPORT EN_deletedemand(EN_Project p, int nodeIndex, int demandIndex)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = index of node's demand to be deleted
**  Returns: error code
**  Purpose: deletes an existing demand category from a junction node
**----------------------------------------------------------------
*/
{
    Pdemand d, dprev;
    Snode *node;
    int n = 1;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Only junctions have demands
    if (nodeIndex <= p->network.Njuncs)
    {
        // Find head of node's list of demands
        node = &p->network.Node[nodeIndex];
        d = node->D;
        if (d == NULL) return 253;
        dprev = d;

        // Check if target demand is head of demand list
        if (demandIndex == 1)
        {
            node->D = d->next;
            free(d->Name);
            free(d);
            return 0;
        }

        // Otherwise locate target demand in demand list
        while (d != NULL && n < demandIndex)
        {
            dprev = d;
            d = d->next;
            n++;
        }

        // Return error if target demand not found
        if (d == NULL) return 253;

        // Link the demands that precede and follow the target
        dprev->next = d->next;

        // Delete the target demand
        free(d->Name);
        free(d);
    }
    return 0;
}

int DLLEXPORT EN_getdemandindex(EN_Project p, int nodeIndex, char *demandName,
                                int *demandIndex)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandName = name of demand being sought
**  Output:  demandIndex  = index of demand being sought
**  Returns: error code
**  Purpose: retrieves the position of a named demand category
**           in a node's list of demands
**----------------------------------------------------------------
*/
{
    Pdemand d;
    int n = 0;
    int nameEmpty = FALSE;
    int found = FALSE;

    // Check for valid arguments
    *demandIndex = 0;
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;
    if (demandName == NULL) return 253;

    // Check if target name is empty
    if (strlen(demandName) == 0) nameEmpty = TRUE;

    // Locate target demand in node's demands list
    for (d = p->network.Node[nodeIndex].D; d != NULL; d = d->next)
    {
        n++;
        if (d->Name == NULL)
        {
            if (nameEmpty) found = TRUE;;
        }
        else if (strcmp(d->Name, demandName) == 0) found = TRUE;
        if (found) break;
    }

    // Return target demand's index
    if (!found) return 253;
    *demandIndex = n;
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

    // Count the number of demand categories assigned to node
    for (d = p->network.Node[nodeIndex].D; d != NULL; d = d->next) n++;
    *numDemands = n;
    return 0;
}

int DLLEXPORT EN_getbasedemand(EN_Project p, int nodeIndex, int demandIndex,
                               double *baseDemand)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**  Output:  baseDemand = baseline demand value
**  Returns: error code
**  Purpose: retrieves the baseline value for a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;

    // Check for valid arguments
    *baseDemand = 0.0;
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Locate target demand in node's demands list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Retrieve target demand's base value
    *baseDemand = d->Base * p->Ucf[FLOW];
    return 0;
}

int DLLEXPORT EN_setbasedemand(EN_Project p, int nodeIndex, int demandIndex,
                               double baseDemand)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**           baseDemand = baseline demand value
**  Output:  none
**  Returns: error code
**  Purpose: sets the baseline value for a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Locate target demand in node's demands list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Assign new base value to target demand
    d->Base = baseDemand / p->Ucf[FLOW];
    return 0;
}

int DLLEXPORT EN_getdemandname(EN_Project p, int nodeIndex, int demandIndex,
                               char *demandName)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**  Output:  demandname = demand category name
**  Returns: error code
**  Purpose: retrieves the name assigned to a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;

    strcpy(demandName, "");

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Njuncs) return 203;

    // Locate target demand in node's demands list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Retrieve target demand's category name
    if (d->Name) strcpy(demandName, d->Name);
    return 0;
}

int DLLEXPORT EN_setdemandname(EN_Project p, int nodeIndex, int demandIndex,
                               char *demandName)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**           demandName = name of demand category
**  Output:  none
**  Returns: error code
**  Purpose: assigns a name to a node's demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Njuncs) return 203;

    // Locate target demand in node's demands list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Assign category name to target demand
    d->Name = xstrcpy(&d->Name, demandName, MAXID);
    return 0;
}

int DLLEXPORT EN_getdemandpattern(EN_Project p, int nodeIndex, int demandIndex,
                                  int *patIndex)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**  Output:  patIndex = time pattern index
**  Returns: error code
**  Purpose: retrieves the time pattern assigned to a node's
**           demand category
**----------------------------------------------------------------
*/
{
    Pdemand d;

    // Check for valid arguments
    *patIndex = 0;
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > p->network.Nnodes) return 203;

    // Locate target demand in node's demand list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Retrieve that demand's pattern index
    *patIndex = d->Pat;
    return 0;
}

int  DLLEXPORT EN_setdemandpattern(EN_Project p, int nodeIndex, int demandIndex,
                                   int patIndex)
/*----------------------------------------------------------------
**  Input:   nodeIndex = node index
**           demandIndex = demand category index
**           patIndex = time pattern index
**  Output:  none
**  Returns: error code
**  Purpose: assigns a time pattern to a node's demand category
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Pdemand d;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (nodeIndex <= 0 || nodeIndex > net->Nnodes) return 203;
    if (patIndex < 0 || patIndex > net->Npats) return 205;

    // Locate target demand in node's demand list
    d = finddemand(p->network.Node[nodeIndex].D, demandIndex);
    if (d == NULL) return 253;

    // Assign new time pattern to target demand
    d->Pat = patIndex;
    return 0;
}

/********************************************************************

    Link Functions

********************************************************************/

int DLLEXPORT EN_addlink(EN_Project p, char *id, int linkType,
                         char *fromNode, char *toNode, int *index)
/*----------------------------------------------------------------
**  Input:   id = link ID name
**           type = link type (see EN_LinkType)
**           fromNode = name of link's starting node
**           toNode = name of link's ending node
**  Output:  index = position of new link in Link array
**  Returns: error code
**  Purpose: adds a new link to a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;

    int i, n, size, errcode;
    int n1, n2;
    Slink *link;
    Spump *pump;

    // Cannot modify network structure while solvers are active
    *index = 0;
    if (!p->Openflag) return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    // Check if a link with same id already exists
    if (EN_getlinkindex(p, id, &i) == 0) return 215;

    // Check for valid link type
    if (linkType < CVPIPE || linkType > GPV) return 251;

    // Lookup the link's from and to nodes
    n1 = hashtable_find(net->NodeHashTable, fromNode);
    n2 = hashtable_find(net->NodeHashTable, toNode);
    if (n1 == 0 || n2 == 0) return 203;

    // Check that valve link has legal connections
    if (linkType > PUMP)
    {
        errcode = valvecheck(p, 0, linkType, n1, n2);
        if (errcode) return errcode;
    }

    // Grow link-related arrays to accomodate the new link
    net->Nlinks++;
    p->parser.MaxLinks = net->Nlinks;
    n = net->Nlinks;
    size = (n + 1) * sizeof(Slink);
    net->Link = (Slink *)realloc(net->Link, size);
    size = (n + 1) * sizeof(double);
    hyd->LinkFlow = (double *)realloc(hyd->LinkFlow, size);
    hyd->LinkSetting = (double *)realloc(hyd->LinkSetting, size);
    size = (n + 1) * sizeof(StatusType);
    hyd->LinkStatus = (StatusType *)realloc(hyd->LinkStatus, size);

    // Set properties for the new link
    link = &net->Link[n];
    strncpy(link->ID, id, MAXID);

    if (linkType <= PIPE) net->Npipes++;
    else if (linkType == PUMP)
    {
        // Grow pump array to accomodate the new link
        net->Npumps++;
        size = (net->Npumps + 1) * sizeof(Spump);
        net->Pump = (Spump *)realloc(net->Pump, size);
        pump = &net->Pump[net->Npumps];
        pump->Link = n;
        pump->Ptype = NOCURVE;
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

    if (linkType == PUMP)
    {
        link->Kc = 1.0; // Speed factor
        link->Km = 0.0; // Horsepower
        link->Len = 0.0;
    }
    else if (linkType <= PIPE)  // pipe or cvpipe
    {
        // 10" diameter new ductile iron pipe with
        // length of average city block
        link->Diam = 10 / p->Ucf[DIAM];
        switch (hyd->Formflag)
        {
        case HW: link->Kc = 130;    break;
        case DW: link->Kc = 0.0005; break;
        case CM: link->Kc = 0.01;   break;
        default: link->Kc = 1.0;
        }
        link->Km = 0.0; // Loss coeff
        link->Len = 330.0;
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
    link->ResultIndex = 0;
    link->Comment = NULL;
    link->Vertices = NULL;

    hashtable_insert(net->LinkHashTable, link->ID, n);
    *index = n;
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
**  Purpose: deletes a link from a project
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i;
    int pumpindex;
    int valveindex;
    int linkType;
    Slink *link;

    // Cannot modify network structure while solvers are active
    if (!p->Openflag) return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check that link exists
    if (index <= 0 || index > net->Nlinks) return 204;
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

    // Remove link's comment and vertices
    free(link->Comment);
    freelinkvertices(link);

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

    // Delete any valve (linkType > PUMP) associated with the deleted link
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

    // Check for valid arguments
    if (index <= 0 || index > net->Nlinks) return 204;
    if (!namevalid(newid)) return 252;

    // Check if another link with same name exists
    if (hashtable_find(net->LinkHashTable, newid) > 0) return 215;

    // Replace the existing link ID with the new value
    hashtable_delete(net->LinkHashTable, net->Link[index].ID);
    strncpy(net->Link[index].ID, newid, MAXID);
    hashtable_insert(net->LinkHashTable, net->Link[index].ID, index);
    return 0;
}

int DLLEXPORT EN_getlinktype(EN_Project p, int index, int *linkType)
/*----------------------------------------------------------------
**  Input:   index = link index
**  Output:  linkType = link type (see EN_LinkType)
**  Returns: error code
**  Purpose: retrieves the type code of a link
**----------------------------------------------------------------
*/
{
    *linkType = -1;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Nlinks) return 204;
    *linkType = p->network.Link[index].Type;
    return 0;
}

int DLLEXPORT EN_setlinktype(EN_Project p, int *index, int linkType, int actionCode)
/*----------------------------------------------------------------
**  Input:   index = link index
**           linkType = new link type (see EN_LinkType)
**           actionCode = how to treat controls that contain the link:
**           EN_UNCONDITIONAL deletes all such controls,
**           EN_CONDITIONAL cancels the type change if the link appears
**           in a control and returns an error code
**  Output:  none
**  Returns: error code
**  Purpose: changes the type of a particular link (e.g. pipe to pump)
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int i = *index, n1, n2;
    char id[MAXID + 1];
    char id1[MAXID + 1];
    char id2[MAXID + 1];
    int errcode;
    int oldType;

    // Cannot modify network structure while solvers are active
    if (!p->Openflag) return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check for valid input parameters
    if (linkType < 0 || linkType > GPV || actionCode < EN_UNCONDITIONAL ||
        actionCode > EN_CONDITIONAL)
    {
        return 251;
    }

    // Check for valid link index
    if (i <= 0 || i > net->Nlinks) return 204;

    // Check if current link type equals new type
    EN_getlinktype(p, i, &oldType);
    if (oldType == linkType) return 0;

    // Type change will be cancelled if link appears in any controls
    if (actionCode == EN_CONDITIONAL)
    {
        actionCode = incontrols(p, LINK, i);
        if (actionCode > 0) return 261;
    }

    // Pipe changing from or to having a check valve
    if (oldType <= PIPE && linkType <= PIPE)
    {
        net->Link[i].Type = linkType;
        if (linkType == CVPIPE) net->Link[i].Status = OPEN;
        return 0;
    }

    // Get ID's of link & its end nodes
    EN_getlinkid(p, i, id);
    EN_getlinknodes(p, i, &n1, &n2);
    EN_getnodeid(p, n1, id1);
    EN_getnodeid(p, n2, id2);

    // Check for illegal valve connections
    errcode = valvecheck(p, i, linkType, n1, n2);
    if (errcode) return errcode;

    // Delete the original link (and any controls containing it)
    EN_deletelink(p, i, actionCode);

    // Create a new link of new type and old id
    errcode = EN_addlink(p, id, linkType, id1, id2, index);
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
    int type, errcode;

    // Cannot modify network structure while solvers are active
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check for valid link index
    if (index <= 0 || index > net->Nlinks) return 204;

    // Check that nodes exist
    if (node1 < 0 || node1 > net->Nnodes) return 203;
    if (node2 < 0 || node2 > net->Nnodes) return 203;

    // Check that nodes are not the same
    if (node1 == node2) return 222;

    // Do nothing if the new nodes are the same as the old ones
    if (node1 == net->Link[index].N1 && node2 == net->Link[index].N2)
        return 0;

    // Check for illegal valve connection
    type = net->Link[index].Type;
    if (type > PUMP)
    {
        errcode = valvecheck(p, index, type, node1, node2);
        if (errcode) return errcode;
    }

    // Assign new end nodes to link
    net->Link[index].N1 = node1;
    net->Link[index].N2 = node2;
    return 0;
}

int DLLEXPORT EN_getlinkvalue(EN_Project p, int index, int property, double *value)
/*----------------------------------------------------------------
**  Input:   index = link index
**           property = link property code (see EN_LinkProperty)
**  Output:  value = link property value
**  Returns: error code
**  Purpose: retrieves a property value for a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Hydraul *hyd = &p->hydraul;

    double a, h, q, v = 0.0;
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
    switch (property)
    {
    case EN_DIAMETER:
        if (Link[index].Type == PUMP) v = 0.0;
        else v = Link[index].Diam * Ucf[DIAM];
        break;

    case EN_LENGTH:
        v = Link[index].Len * Ucf[ELEV];
        break;

    case EN_ROUGHNESS:
        if (Link[index].Type <= PIPE)
        {
            if (hyd->Formflag == DW) v = Link[index].Kc * (1000.0 * Ucf[ELEV]);
            else v = Link[index].Kc;
        }
        else v = 0.0;
        break;

    case EN_MINORLOSS:
        if (Link[index].Type != PUMP)
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
        if (Link[index].Type == PIPE || Link[index].Type == CVPIPE)
        {
            return EN_getlinkvalue(p, index, EN_ROUGHNESS, value);
        }
        v = Link[index].Kc;
        switch (Link[index].Type)
        {
        case PRV:
        case PSV:
        case PBV:
            v *= Ucf[PRESSURE];
            break;
        case FCV:
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
        if (Link[index].Type == PUMP) v = 0.0;
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
            if (Link[index].Type != PUMP) h = ABS(h);
            v = h * Ucf[HEADLOSS];
        }
        break;

    case EN_STATUS:
        if (hyd->LinkStatus[index] <= CLOSED) v = 0.0;
        else v = 1.0;
        break;

    case EN_SETTING:
        if (Link[index].Type == PIPE || Link[index].Type == CVPIPE)
        {
            return EN_getlinkvalue(p, index, EN_ROUGHNESS, value);
        }
        if (LinkSetting[index] == MISSING) v = 0.0;
        else v = LinkSetting[index];
        switch (Link[index].Type)
        {
        case PRV:
        case PSV:
        case PBV:
            v *= Ucf[PRESSURE];
            break;
        case FCV:
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
        if (Link[index].Type == PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Upat;
        }
        break;

    case EN_PUMP_STATE:
        v = hyd->LinkStatus[index];

        if (Link[index].Type == PUMP)
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

    case EN_PUMP_EFFIC:
        getenergy(p, index, &a, &v);
        break;

    case EN_PUMP_POWER:
        v = 0;
        if (Link[index].Type == PUMP)
        {
            pmp = findpump(net, index);
            if (Pump[pmp].Ptype == CONST_HP) v = Link[index].Km; // Power in HP or KW
        }
        break;

    case EN_PUMP_HCURVE:
        if (Link[index].Type == PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Hcurve;
        }
        break;

    case EN_PUMP_ECURVE:
        if (Link[index].Type == PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Ecurve;
        }
        break;

    case EN_PUMP_ECOST:
        if  (Link[index].Type == PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Ecost;
        }
        break;

    case EN_PUMP_EPAT:
        if (Link[index].Type == PUMP)
        {
            v = (double)Pump[findpump(&p->network, index)].Epat;
        }
        break;

    default:
        return 251;
    }
    *value = (double)v;
    return 0;
}

int DLLEXPORT EN_setlinkvalue(EN_Project p, int index, int property, double value)
/*----------------------------------------------------------------
**  Input:   index = link index
**           property  = link property code (see EN_LinkProperty)
**           value = property value
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
    double r;
    int pumpIndex, patIndex, curveIndex;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    switch (property)
    {
    case EN_DIAMETER:
        if (Link[index].Type != PUMP)
        {
            if (value <= 0.0) return 211;
            value /= Ucf[DIAM];                // Convert to feet
            r = Link[index].Diam / value;      // Ratio of old to new diam
            Link[index].Km *= SQR(r) * SQR(r); // Adjust minor loss factor
            Link[index].Diam = value;          // Update diameter
            resistcoeff(p, index);             // Update resistance coeff.
        }
        break;

    case EN_LENGTH:
        if (Link[index].Type <= PIPE)
        {
            if (value <= 0.0) return 211;
            Link[index].Len = value / Ucf[ELEV];
            resistcoeff(p, index);
        }
        break;

    case EN_ROUGHNESS:
        if (Link[index].Type <= PIPE)
        {
            if (value <= 0.0) return 211;
            Link[index].Kc = value;
            if (hyd->Formflag == DW) Link[index].Kc /= (1000.0 * Ucf[ELEV]);
            resistcoeff(p, index);
        }
        break;

    case EN_MINORLOSS:
        if (Link[index].Type != PUMP)
        {
            if (value <= 0.0) return 211;
            Link[index].Km = 0.02517 * value / SQR(Link[index].Diam) /
                             SQR(Link[index].Diam);
        }
        break;

    case EN_INITSTATUS:
    case EN_STATUS:
        // Cannot set status for a check valve
        if (Link[index].Type == CVPIPE) return 207;
        s = (char)ROUND(value);
        if (s < 0 || s > 1) return 211;
        if (property == EN_INITSTATUS)
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
        if (value < 0.0) return 211;
        if (Link[index].Type == PIPE || Link[index].Type == CVPIPE)
        {
            return EN_setlinkvalue(p, index, EN_ROUGHNESS, value);
        }
        else
        {
            switch (Link[index].Type)
            {
            case PUMP:
                break;
            case PRV:
            case PSV:
            case PBV:
                value /= Ucf[PRESSURE];
                break;
            case FCV:
                value /= Ucf[FLOW];
                break;
            case TCV:
                break;
            case GPV:
                return 207; // Cannot modify setting for GPV
            default:
                return 0;
            }
            if (property == EN_INITSETTING)
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
        if (Link[index].Type <= PIPE)
        {
            Link[index].Kb = value / SECperDAY;
            qual->Reactflag = 1;
        }
        break;

    case EN_KWALL:
        if (Link[index].Type <= PIPE)
        {
            Link[index].Kw = value / SECperDAY;
            qual->Reactflag = 1;
        }
        break;

    case EN_LINKPATTERN:
        if (Link[index].Type == PUMP)
        {
            patIndex = ROUND(value);
            if (patIndex < 0 || patIndex > net->Npats) return 205;
            pumpIndex = findpump(&p->network, index);
            net->Pump[pumpIndex].Upat = patIndex;
        }
        break;

    case EN_PUMP_POWER:
        if (Link[index].Type == PUMP)
        {
            if (value <= 0.0) return 211;
            pumpIndex = findpump(&p->network, index);
            net->Pump[pumpIndex].Ptype = CONST_HP;
            net->Pump[pumpIndex].Hcurve = 0;
            net->Link[index].Km = value;
            updatepumpparams(p, pumpIndex);
            net->Pump[pumpIndex].R /= Ucf[POWER];
            net->Pump[pumpIndex].Q0 /= Ucf[FLOW];
            net->Pump[pumpIndex].Qmax /= Ucf[FLOW];
            net->Pump[pumpIndex].Hmax /= Ucf[HEAD];
        }
        break;

    case EN_PUMP_HCURVE:
        if (Link[index].Type == PUMP)
        {
            return EN_setheadcurveindex(p, index, ROUND(value));
        }
        break;

    case EN_PUMP_ECURVE:
        if (Link[index].Type == PUMP)
        {
            curveIndex = ROUND(value);
            if (curveIndex < 0 || curveIndex > net->Ncurves) return 205;
            pumpIndex = findpump(&p->network, index);
            net->Pump[pumpIndex].Ecurve = curveIndex;
        }
        break;

    case EN_PUMP_ECOST:
        if (Link[index].Type == PUMP)
        {
            if (value < 0.0) return 211;
            pumpIndex = findpump(&p->network, index);
            net->Pump[pumpIndex].Ecost = value;
        }
        break;

    case EN_PUMP_EPAT:
        if (Link[index].Type == PUMP)
        {
            patIndex = ROUND(value);
            if (patIndex < 0 || patIndex > net->Npats) return 205;
            pumpIndex = findpump(&p->network, index);
            net->Pump[pumpIndex].Epat = patIndex;
        }
        break;

    default:
        return 251;
    }
    return 0;
}

int DLLEXPORT EN_setpipedata(EN_Project p, int index, double length,
                             double diam, double rough, double mloss)
/*----------------------------------------------------------------
**  Input:   index = pipe link index
**           length = pipe length
**           diam = pipe diameter
**           rough = pipe roughness coefficient
**           mloss = minor loss coefficient
**  Output:  none
**  Returns: error code
**  Purpose: sets several properties for a pipe link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    double *Ucf = p->Ucf;
    double diameter = diam;

    // Check that pipe exists
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    if (Link[index].Type > PIPE) return 0;

    // Check for valid parameters
    if (length <= 0.0 || diam <= 0.0 || rough <= 0.0 || mloss < 0.0) return 211;

    // Assign parameters to pipe
    Link[index].Len = length / Ucf[ELEV];
    diameter /= Ucf[DIAM];
    Link[index].Diam = diameter;
    Link[index].Kc = rough;
    if (p->hydraul.Formflag == DW) Link[index].Kc /= (1000.0 * Ucf[ELEV]);

    // Update minor loss factor & pipe flow resistance
    Link[index].Km = 0.02517 * mloss / SQR(Link[index].Diam) / SQR(Link[index].Diam);
    resistcoeff(p, index);
    return 0;
}

int DLLEXPORT EN_getvertexcount(EN_Project p, int index, int *count)
/*----------------------------------------------------------------
**  Input:   index = link index
**  Output:  count = number of link's vertex points
**  Returns: error code
**  Purpose: retrieves number of vertex points in a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    
    Slink *Link = net->Link;
    Pvertices vertices;
    
    // Check that link exists
    *count = 0;
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    
    // Set count to number of vertices
    vertices = Link[index].Vertices;
    if (vertices) *count = vertices->Npts;
    return 0;
}    

int DLLEXPORT EN_getvertex(EN_Project p, int index, int vertex, double *x, double *y)
/*----------------------------------------------------------------
**  Input:   index = link index
**           vertex = index of a link vertex point
**  Output:  x = vertex point's X-coordinate
**           y = vertex point's Y-coordinate
**  Returns: error code
**  Purpose: retrieves the coordinates of a vertex point in a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    
    Slink *Link = net->Link;
    Pvertices vertices;
    
    // Check that link exists
    *x = MISSING;
    *y = MISSING;
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    
    // Check that vertex exists
    vertices = Link[index].Vertices;
    if (vertices == NULL) return 255;
    if (vertex <= 0 || vertex > vertices->Npts) return 255;
    *x = vertices->X[vertex - 1];
    *y = vertices->Y[vertex - 1];    
    return 0;
}
    
int DLLEXPORT EN_setvertices(EN_Project p, int index, double *x, double *y, int count)
/*----------------------------------------------------------------
**  Input:   index = link index
**           x = array of X-coordinates for vertex points
**           y = array of Y-coordinates for vertex points
**           count = number of vertex points
**  Returns: error code
**  Purpose: assigns a set of vertex points to a link
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    
    Slink *link;
    int i;
    int err = 0;
    
    // Check that link exists
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Nlinks) return 204;
    link = &net->Link[index];

    // Delete existing set of vertices
    freelinkvertices(link);
    
    // Add each new vertex to the link
    for (i = 0; i < count; i++)
    {
        err = addlinkvertex(link, x[i], y[i]);
        if (err) break;
    }
    if (err) freelinkvertices(link);
    return err;
}    

/********************************************************************

    Pump Functions

********************************************************************/

int DLLEXPORT EN_getpumptype(EN_Project p, int linkIndex, int *pumpType)
/*----------------------------------------------------------------
**  Input:   linkIndex = index of a pump link
**  Output:  pumpType = type of pump head curve (see EN_PumpType)
**  Returns: error code
**  Purpose: retrieves the type of head curve used by a pump
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
    const int Nlinks = net->Nlinks;

    *pumpType = -1;
    if (!p->Openflag) return 102;
    if (linkIndex < 1 || linkIndex > Nlinks) return 204;
    if (PUMP != Link[linkIndex].Type) return 216;
    *pumpType = Pump[findpump(&p->network, linkIndex)].Ptype;
    return 0;
}

int DLLEXPORT EN_getheadcurveindex(EN_Project p, int linkIndex, int *curveIndex)
/*----------------------------------------------------------------
**  Input:   linkIndex = index of a pump link
**  Output:  curveIndex = index of a pump's head curve
**  Returns: error code
**  Purpose: retrieves the index of a pump's head curve
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
    const int Nlinks = net->Nlinks;

    *curveIndex = 0;
    if (!p->Openflag) return 102;
    if (linkIndex < 1 || linkIndex > Nlinks) return 204;
    if (PUMP != Link[linkIndex].Type) return 216;
    *curveIndex = Pump[findpump(net, linkIndex)].Hcurve;
    return 0;
}

int DLLEXPORT EN_setheadcurveindex(EN_Project p, int linkIndex, int curveIndex)
/*----------------------------------------------------------------
**  Input:   linkIndex = index of a pump link
**           curveIndex = index of a curve
**  Output:  none
**  Returns: error code
**  Purpose: assigns a new head curve to a pump
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    double *Ucf = p->Ucf;
    int pumpIndex;
    int oldCurveIndex;
    int newCurveType;
    int err = 0;
    Spump *pump;

    // Check for valid parameters
    if (!p->Openflag) return 102;
    if (linkIndex < 1 || linkIndex > net->Nlinks) return 204;
    if (PUMP != net->Link[linkIndex].Type) return 0;
    if (curveIndex < 0 || curveIndex > net->Ncurves) return 206;

    // Save values that need to be restored in case new curve is invalid
    pumpIndex = findpump(net, linkIndex);
    pump = &p->network.Pump[pumpIndex];
    oldCurveIndex = pump->Hcurve;
    newCurveType = p->network.Curve[curveIndex].Type;
    
    // Assign the new curve to the pump
    pump->Ptype = NOCURVE;
    pump->Hcurve = curveIndex;
    if (curveIndex == 0) return 0;
    
    // Update the pump's head curve parameters (which also changes
    // the new curve's Type to PUMP_CURVE)
    err = updatepumpparams(p, pumpIndex);
    
    // If the parameter updating failed (new curve was not a valid pump curve)
    // restore the pump's original curve and its parameters
    if (err > 0)
    {
        p->network.Curve[curveIndex].Type = newCurveType;
        pump->Ptype = NOCURVE;
        pump->Hcurve = oldCurveIndex;
        if (oldCurveIndex == 0) return err;
        updatepumpparams(p, pumpIndex);
    }    
    
    // Convert the units of the updated pump parameters to feet and cfs
    if (pump->Ptype == POWER_FUNC)
    {
        pump->H0 /= Ucf[HEAD];
        pump->R *= (pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
    }
    pump->Q0 /= Ucf[FLOW];
    pump->Qmax /= Ucf[FLOW];
    pump->Hmax /= Ucf[HEAD];

    return err;
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

    int i, n, err = 0;
    Spattern *pat;

    // Check if a pattern with same id already exists
    if (!p->Openflag) return 102;
    if (EN_getpatternindex(p, id, &i) == 0) return 215;

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    // Expand the project's array of patterns
    n = net->Npats + 1;
    net->Pattern = (Spattern *)realloc(net->Pattern, (n + 1) * sizeof(Spattern));

    // Assign properties to the new pattern
    pat = &net->Pattern[n];
    strcpy(pat->ID, id);
    pat->Comment = NULL;
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
    return 0;
}

int  DLLEXPORT EN_deletepattern(EN_Project p, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the pattern to delete
**  Output:  none
**  Returns: error code
**  Purpose: deletes a time pattern from a project
**----------------------------------------------------------------
*/
{
    int i;

    Network *net = &p->network;
    Parser  *parser = &p->parser;
    Hydraul *hyd = &p->hydraul;

    // Can't delete a pattern while a solver is active
    if (!p->Openflag)return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check that pattern exists
    if (index < 1 || index > p->network.Npats) return 205;

    // Adjust references by other objects to patterns
    adjustpatterns(net, index);

    // Modify global energy price pattern
    if (hyd->Epat == index)  hyd->Epat = 0;
    else if (hyd->Epat > index) hyd->Epat--;

    // Free the pattern's factor array
    FREE(net->Pattern[index].F);
    FREE(net->Pattern[index].Comment);

    // Shift the entries in the network's Pattern array
    for (i = index; i < net->Npats; i++) net->Pattern[i] = net->Pattern[i+1];
    net->Npats--;
    parser->MaxPats--;
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

int DLLEXPORT EN_setpatternid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           id = time pattern ID name
**  Returns: error code
**  Purpose: changes the ID name of a time pattern
**----------------------------------------------------------------
*/
{
    int i;

    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Npats) return 205;

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    for (i = 1; i <= p->network.Npats; i++)
    {
        if (i != index && strcmp(id, p->network.Pattern[i].ID) == 0) return 215;
    }
    strcpy(p->network.Pattern[index].ID, id);
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

int DLLEXPORT EN_getpatternvalue(EN_Project p, int index, int period, double *value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           period = time pattern period
**  Output:  value = pattern factor for a particular time period
**  Returns: error code
**  Purpose: retrieves the pattern factor for a specific time period
**           in a time pattern
**----------------------------------------------------------------
*/
{
    *value = 0.0;
    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Npats) return 205;
    if (period < 1 || period > p->network.Pattern[index].Length) return 251;
    *value = (double)p->network.Pattern[index].F[period - 1];
    return 0;
}

int DLLEXPORT EN_setpatternvalue(EN_Project p, int index, int period, double value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           period = time pattern period
**           value = pattern factor for a particular time period
**  Output:  none
**  Returns: error code
**  Purpose: sets the pattern factor for a specific time period
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

int DLLEXPORT EN_getaveragepatternvalue(EN_Project p, int index, double *value)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**  Output:  value = average of a time pattern's factors
**  Returns: error code
**  Purpose: retrieves the average of all pattern factors for a time pattern
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
        *value += (double)Pattern[index].F[i];
    }
    *value /= (double)Pattern[index].Length;
    return 0;
}

int DLLEXPORT EN_setpattern(EN_Project p, int index, double *values, int len)
/*----------------------------------------------------------------
**  Input:   index = time pattern index
**           values = an array of pattern factor values
**           len = number of time periods contained in f
**  Output:  none
**  Returns: error code
**  Purpose: replaces the pattern factors in a time pattern
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;

    int j;
    Spattern *Pattern = net->Pattern;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Npats) return 205;
    if (values == NULL) return 205;
    if (len <= 0) return 202;

    // Re-set number of time periods & reallocate memory for multipliers
    Pattern[index].Length = len;
    Pattern[index].F = (double *)realloc(Pattern[index].F, len * sizeof(double));
    if (Pattern[index].F == NULL) return 101;

    // Load multipliers into pattern
    for (j = 0; j < len; j++) Pattern[index].F[j] = values[j];
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

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    // Expand the array of curves
    n = net->Ncurves + 1;
    net->Curve = (Scurve *) realloc(net->Curve, (n + 1) * sizeof(Scurve));

    // Set the properties of the new curve
    curve = &net->Curve[n];
    strcpy(curve->ID, id);
    curve->Comment = NULL;
    curve->Capacity = 1;
    curve->Npts = 1;
    curve->Type = GENERIC_CURVE;
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

int  DLLEXPORT EN_deletecurve(EN_Project p, int index)
/*----------------------------------------------------------------
**  Input:   index  = index of the curve to delete
**  Output:  none
**  Returns: error code
**  Purpose: deletes a data curve from a project
**----------------------------------------------------------------
*/
{
    int i;

    Network *net = &p->network;
    Parser  *parser = &p->parser;

    // Can't delete a curve while a solver is active
    if (!p->Openflag)return 102;
    if (p->hydraul.OpenHflag || p->quality.OpenQflag) return 262;

    // Check that curve exists
    if (index < 1 || index > p->network.Ncurves) return 205;

    // Adjust references by other objects to curves
    adjustcurves(net, index);

    // Free the curve's data arrays
    FREE(net->Curve[index].X);
    FREE(net->Curve[index].Y);
    FREE(net->Curve[index].Comment);

    // Shift the entries in the network's Curve array
    for (i = index; i < net->Ncurves; i++) net->Curve[i] = net->Curve[i + 1];
    net->Ncurves--;
    parser->MaxCurves--;
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
    *index = 0;
    if (!p->Openflag) return 102;
    *index = findcurve(&p->network, id);
    if (*index == 0) return 206;
    return 0;
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

int DLLEXPORT EN_setcurveid(EN_Project p, int index, char *id)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**           id = data curve ID name
**  Returns: error code
**  Purpose: changes the ID name of a data curve
**----------------------------------------------------------------
*/
{
    int i;

    if (!p->Openflag) return 102;
    if (index < 1 || index > p->network.Ncurves) return 205;

    // Check if id name contains invalid characters
    if (!namevalid(id)) return 252;

    for (i = 1; i <= p->network.Ncurves; i++)
    {
        if (i != index && strcmp(id, p->network.Curve[i].ID) == 0) return 215;
    }
    strcpy(p->network.Curve[index].ID, id);
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

int DLLEXPORT EN_getcurvetype(EN_Project p, int index, int *type)
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
    if (index < 1 || index > net->Ncurves) return 206;
    *type = net->Curve[index].Type;
    return 0;
}

int DLLEXPORT EN_getcurvevalue(EN_Project p, int curveIndex, int pointIndex,
                               double *x, double *y)
/*----------------------------------------------------------------
**  Input:   curveIndex = data curve index
**           pointIndex = index of a data point on the curve
**  Output:  x = x-value of the point on the curve
**           y = y-value of the point on the curve
**  Returns: error code
**  Purpose: retrieves the value of a specific point on a data curve
**----------------------------------------------------------------
*/
{
    *x = 0.0;
    *y = 0.0;
    if (!p->Openflag) return 102;
    if (curveIndex < 1 || curveIndex > p->network.Ncurves) return 206;
    if (pointIndex < 1 || pointIndex > p->network.Curve[curveIndex].Npts) return 251;
    *x = p->network.Curve[curveIndex].X[pointIndex - 1];
    *y = p->network.Curve[curveIndex].Y[pointIndex - 1];
    return 0;
}

int DLLEXPORT EN_setcurvevalue(EN_Project p, int curveIndex, int pointIndex,
                               double x, double y)
/*----------------------------------------------------------------
**  Input:   curveIndex = data curve index
**           pointIndex = index of a data point on the curve
**           x = new x-value for the point on the curve
**           y = new y-value for the point on the curve
**  Output:  none
**  Returns: error code
**  Purpose: sets the value of a specific point on a data curve
**  Note:    if pointIndex exceeds the curve's length a new point is added.
**----------------------------------------------------------------
*/
{
    Network *net = &p->network;
    Scurve *curve;
    double x1 = -1.e37, x2 = 1.e37;
    int n = pointIndex - 1;

    // Check for valid input
    if (!p->Openflag) return 102;
    if (curveIndex <= 0 || curveIndex > net->Ncurves) return 206;
    curve = &net->Curve[curveIndex];
    if (pointIndex <= 0) return 251;

    // Check that new point maintains increasing x values
    if (n - 1 >= 0) x1 = curve->X[n-1];
    if (n + 1 < curve->Npts) x2 = curve->X[n+1];
    if (x <= x1 || x >= x2) return 230;

    // Expand curve if need be
    if (pointIndex > curve->Npts) pointIndex = curve->Npts + 1;
    if (pointIndex >= curve->Capacity)
    {
        if (resizecurve(curve, curve->Capacity + 10) > 0) return 101;
    }

    // Increase curve's number of points if need be
    if (pointIndex > curve->Npts)
    {
        curve->Npts++;
        n = curve->Npts - 1;
    }

    // Insert new point into curve
    curve->X[n] = x;
    curve->Y[n] = y;
    
    // Adjust parameters for pumps using curve as a head curve
    return adjustpumpparams(p, curveIndex);
}

int DLLEXPORT EN_getcurve(EN_Project p, int index, char *id, int *nPoints,
                          double *xValues, double *yValues)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**  Output:  id = ID name of data curve
**           nPoints = number of data points on the curve
**           xValues = array of x-values for each data point
**           yValues = array of y-values for each data point
**  Returns: error code
**  Purpose: retrieves the data associated with a data curve
**
**  The calling program is responsible for making xValues and
**  yValues large enough to hold nPoints data points.
**----------------------------------------------------------------
*/
{
    int i;
    Scurve *curve;

    if (!p->Openflag) return 102;
    if (index <= 0 || index > p->network.Ncurves) return 206;
    if (xValues == NULL || yValues == NULL) return 206;
    curve = &p->network.Curve[index];
    strncpy(id, curve->ID, MAXID);
    *nPoints = curve->Npts;
    for (i = 0; i < curve->Npts; i++)
    {
        xValues[i] = curve->X[i];
        yValues[i] = curve->Y[i];
    }
    return 0;
}

int DLLEXPORT EN_setcurve(EN_Project p, int index, double *xValues,
                          double *yValues, int nPoints)
/*----------------------------------------------------------------
**  Input:   index = data curve index
**           xValues = array of x-values
**           yValues = array of y-values
**           nPoints = number of data points in the x and y arrays
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
    if (xValues == NULL || yValues == NULL) return 206;
    if (nPoints <= 0) return 202;

    // Check that x values are increasing
    for (j = 1; j < nPoints; j++) if (xValues[j-1] >= xValues[j]) return 230;

    // Expand size of curve's data arrays if need be
    curve = &net->Curve[index];
    if (resizecurve(curve, nPoints) > 0) return 101;

    // Load values into curve
    curve->Npts = nPoints;
    for (j = 0; j < nPoints; j++)
    {
        curve->X[j] = xValues[j];
        curve->Y[j] = yValues[j];
    }
    
    // Adjust parameters for pumps using curve as a head curve
    return adjustpumpparams(p, index);
}

/********************************************************************

    Simple Controls Functions

********************************************************************/

int DLLEXPORT EN_addcontrol(EN_Project p, int type, int linkIndex, double setting,
              int nodeIndex, double level, int *index)
/*----------------------------------------------------------------
**  Input:   type = type of control (see EN_ControlType)
**           linkIndex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nodeIndex = index of node controlling a link (for level controls)
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
    if (linkIndex <= 0 || linkIndex > net->Nlinks) return 204;

    // Cannot control check valve
    if (net->Link[linkIndex].Type == CVPIPE) return 207;

    // Check for valid parameters
    if (type < 0 || type > EN_TIMEOFDAY) return 251;
    if (type == EN_LOWLEVEL || type == EN_HILEVEL)
    {
        if (nodeIndex < 1 || nodeIndex > net->Nnodes) return 203;
    }
    else nodeIndex = 0;
    if (s < 0.0 || lvl < 0.0) return 202;

    // Adjust units of control parameters
    switch (net->Link[linkIndex].Type)
    {
    case PRV:
    case PSV:
    case PBV:
        s /= Ucf[PRESSURE];
        break;
    case FCV:
        s /= Ucf[FLOW];
        break;
    case GPV:
        if (s == 0.0) status = CLOSED;
        else if (s == 1.0) status = OPEN;
        else return 202;
        s = net->Link[linkIndex].Kc;
        break;
    case PIPE:
    case PUMP:
        status = OPEN;
        if (s == 0.0) status = CLOSED;
    default:
        break;
    }

    if (type == LOWLEVEL || type == HILEVEL)
    {
        if (nodeIndex > net->Njuncs) lvl = net->Node[nodeIndex].El + level / Ucf[ELEV];
        else lvl = net->Node[nodeIndex].El + level / Ucf[PRESSURE];
    }
    if (type == TIMER) t = (long)ROUND(lvl);
    if (type == TIMEOFDAY) t = (long)ROUND(lvl) % SECperDAY;

    // Expand project's array of controls
    n = net->Ncontrols + 1;
    net->Control = (Scontrol *)realloc(net->Control, (n + 1) * sizeof(Scontrol));

    // Set properties of the new control
    control = &net->Control[n];
    control->Type = (char)type;
    control->Link = linkIndex;
    control->Node = nodeIndex;
    control->Status = status;
    control->Setting = s;
    control->Grade = lvl;
    control->Time = t;

    // Update number of controls
    net->Ncontrols = n;
    parser->MaxControls = n;

    // Replace the control's index
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

int DLLEXPORT EN_getcontrol(EN_Project p, int index, int *type, int *linkIndex,
                            double *setting, int *nodeIndex, double *level)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**  Output:  type = type of control (see EN_ControlType)
**           linkIndex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nodeIndex = index of node that triggers a level control
**           level = trigger level that activates the control (pressure for junction nodes,
**                   water level for tank nodes or time value for time-based control)
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
    *type = 0;
    *linkIndex = 0;
    *nodeIndex = 0;

    // Check for valid arguments
    if (!p->Openflag) return 102;
    if (index <= 0 || index > net->Ncontrols) return 241;

    // Retrieve control's type and link index
    control = &net->Control[index];
    *type = control->Type;
    *linkIndex = control->Link;

    // Retrieve control's setting
    s = control->Setting;
    if (control->Setting != MISSING)
    {
        switch (net->Link[*linkIndex].Type)
        {
        case PRV:
        case PSV:
        case PBV:
            s *= Ucf[PRESSURE];
            break;
        case FCV:
            s *= Ucf[FLOW];
        default:
            break;
        }
    }
    else if (control->Status == OPEN) s = 1.0;
    else s = 0.0;

    // Retrieve level value for a node level control
    *nodeIndex = control->Node;
    if (*nodeIndex > 0)
    {
        node = &net->Node[*nodeIndex];
        if (*nodeIndex > net->Njuncs)  // Node is a tank
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
        lvl = (double)control->Time;
    }
    *setting = (double)s;
    *level = (double)lvl;
    return 0;
}

int DLLEXPORT EN_setcontrol(EN_Project p, int index, int type, int linkIndex,
                            double setting, int nodeIndex, double level)
/*----------------------------------------------------------------
**  Input:   index  = index of the control
**           type = type of control (see EN_ControlType)
**           linkIndex = index of link being controlled
**           setting = link control setting (e.g., pump speed)
**           nodeIndex = index of node that triggers the control (for level controls)
**           level = trigger level that activates the control (pressure for junction nodes,
**                   water level for tank nodes or time value for time-based control)
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

    // Check that controlled link exists (0 index de-activates the control)
    if (linkIndex == 0)
    {
        control->Link = 0;
        return 0;
    }
    if (linkIndex < 0 || linkIndex > net->Nlinks) return 204;

    // Cannot control check valve
    if (net->Link[linkIndex].Type == CVPIPE) return 207;

    // Check for valid control properties
    if (type < 0 || type > EN_TIMEOFDAY) return 251;
    if (type == EN_LOWLEVEL || type == EN_HILEVEL)
    {
        if (nodeIndex < 1 || nodeIndex > net->Nnodes) return 203;
    }
    else nodeIndex = 0;
    if (s < 0.0 || lvl < 0.0) return 202;

    // Adjust units of control's properties
    link = &net->Link[linkIndex];
    switch (link->Type)
    {
    case PRV:
    case PSV:
    case PBV:
        s /= Ucf[PRESSURE];
        break;
    case FCV:
        s /= Ucf[FLOW];
        break;
    case GPV:
        if (s == 0.0)  status = CLOSED;
        else if (s == 1.0) status = OPEN;
        else return 202;
        s = link->Kc;
        break;
    case PIPE:
    case PUMP:
        status = OPEN;
        if (s == 0.0) status = CLOSED;
    default:
        break;
    }
    if (type == LOWLEVEL || type == HILEVEL)
    {
        if (nodeIndex > net->Njuncs) lvl = net->Node[nodeIndex].El + level / Ucf[ELEV];
        else lvl = net->Node[nodeIndex].El + level / Ucf[PRESSURE];
    }
    if (type == TIMER) t = (long)ROUND(lvl);
    if (type == TIMEOFDAY) t = (long)ROUND(lvl) % SECperDAY;

    /* Reset control's parameters */
    control->Type = (char)type;
    control->Link = linkIndex;
    control->Node = nodeIndex;
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
**  Purpose: deletes a rule from a project
**----------------------------------------------------------------
*/
{
    if (index < 1 || index > p->network.Nrules) return 257;
    deleterule(p, index);
    return 0;
}

int DLLEXPORT EN_getrule(EN_Project p, int index, int *nPremises,
                         int *nThenActions, int *nElseActions,
                         double *priority)
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
    *priority = (double)p->network.Rule[index].priority;

    count = 0;
    premise = net->Rule[index].Premises;
    while (premise != NULL)
    {
        count++;
        premise = premise->next;
    }
    *nPremises = count;

    count = 0;
    action = net->Rule[index].ThenActions;
    while (action != NULL)
    {
        count++;
        action = action->next;
    }
    *nThenActions = count;

    count = 0;
    action = net->Rule[index].ElseActions;
    while (action != NULL)
    {
        count++;
        action = action->next;
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
                           int *relop, int *status, double *value)
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
    *value = (double)premise->value;
    return 0;
}


int DLLEXPORT EN_setpremise(EN_Project p, int ruleIndex, int premiseIndex,
                            int logop, int object, int objIndex, int variable,
                            int relop, int status, double value)
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
    premise = getpremise(premises, premiseIndex);
    if (premise == NULL) return 258;

    premise->status = status;
    return 0;
}

int DLLEXPORT EN_setpremisevalue(EN_Project p, int ruleIndex, int premiseIndex, double value)
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
                               int *linkIndex, int *status, double *setting)
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
    *setting = (double)action->setting;
    return 0;
}

int DLLEXPORT EN_setthenaction(EN_Project p, int ruleIndex, int actionIndex,
                               int linkIndex,  int status, double setting)
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
                               int *linkIndex, int *status, double *setting)
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

  actions = p->network.Rule[ruleIndex].ElseActions;
  action = getaction(actions, actionIndex);
  if (action == NULL) return 258;

  *linkIndex = action->link;
  *status = action->status;
  *setting = (double)action->setting;
  return 0;
}

int DLLEXPORT EN_setelseaction(EN_Project p, int ruleIndex, int actionIndex,
                               int linkIndex,  int status, double setting)
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

  actions = p->network.Rule[ruleIndex].ElseActions;
  action = getaction(actions, actionIndex);
  if (action == NULL) return 258;

  action->link = linkIndex;
  action->status = status;
  action->setting = setting;
  return 0;
}

int DLLEXPORT EN_setrulepriority(EN_Project p, int index, double priority)
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
