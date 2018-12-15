/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       project.c
 Description:  project data management routines
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/15/2018
 ******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

//*** Need to define WINDOWS to use the getTmpName function ***//
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

#include "types.h"
#include "funcs.h"

int openfiles(Project *pr, const char *f1, const char *f2, const char *f3)
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
    // Initialize file pointers to NULL
    pr->parser.InFile = NULL;
    pr->report.RptFile = NULL;
    pr->outfile.OutFile = NULL;
    pr->outfile.HydFile = NULL;

    // Save file names
    strncpy(pr->parser.InpFname, f1, MAXFNAME);
    strncpy(pr->report.Rpt1Fname, f2, MAXFNAME);
    strncpy(pr->outfile.OutFname, f3, MAXFNAME);
    if (strlen(f3) > 0) pr->outfile.Outflag = SAVE;
    else                pr->outfile.Outflag = SCRATCH;

    // Check that file names are not identical
    if (strcomp(f1, f2) || strcomp(f1, f3) ||
        (strcomp(f2, f3) && (strlen(f2) > 0 || strlen(f3) > 0))) return 301;

    // Attempt to open input and report files
    if (strlen(f1) > 0)
    {
        if ((pr->parser.InFile = fopen(f1, "rt")) == NULL) return 302;
    }
    if (strlen(f2) == 0) pr->report.RptFile = stdout;
    else if ((pr->report.RptFile = fopen(f2, "wt")) == NULL) return 303;
    return 0;
}

int openhydfile(Project *pr)
/*----------------------------------------------------------------
** Input:   none
** Output:  none
** Returns: error code
** Purpose: opens file that saves hydraulics solution
**----------------------------------------------------------------
*/
{
    const int Nnodes = pr->network.Nnodes;
    const int Ntanks = pr->network.Ntanks;
    const int Nlinks = pr->network.Nlinks;
    const int Nvalves = pr->network.Nvalves;
    const int Npumps = pr->network.Npumps;

    INT4 nsize[6]; // Temporary array
    INT4 magic;
    INT4 version;
    int errcode = 0;

    // If HydFile currently open, then close it if its not a scratch file
    if (pr->outfile.HydFile != NULL)
    {
        if (pr->outfile.Hydflag == SCRATCH) return 0;
        fclose(pr->outfile.HydFile);
    }

    // Use Hydflag to determine the type of hydraulics file to use.
    // Write error message if the file cannot be opened.
    pr->outfile.HydFile = NULL;
    switch (pr->outfile.Hydflag)
    {
      case SCRATCH:
        strcpy(pr->outfile.HydFname, pr->TmpHydFname);
        pr->outfile.HydFile = fopen(pr->outfile.HydFname, "w+b");
        break;
      case SAVE:
        pr->outfile.HydFile = fopen(pr->outfile.HydFname, "w+b");
        break;
      case USE:
        pr->outfile.HydFile = fopen(pr->outfile.HydFname, "rb");
        break;
    }
    if (pr->outfile.HydFile == NULL) return 305;

    // If a previous hydraulics solution is not being used, then
    // save the current network size parameters to the file.
    if (pr->outfile.Hydflag != USE)
    {
        magic = MAGICNUMBER;
        version = ENGINE_VERSION;
        nsize[0] = Nnodes;
        nsize[1] = Nlinks;
        nsize[2] = Ntanks;
        nsize[3] = Npumps;
        nsize[4] = Nvalves;
        nsize[5] = (int)pr->times.Dur;
        fwrite(&magic, sizeof(INT4), 1, pr->outfile.HydFile);
        fwrite(&version, sizeof(INT4), 1, pr->outfile.HydFile);
        fwrite(nsize, sizeof(INT4), 6, pr->outfile.HydFile);
    }

    // If a previous hydraulics solution is being used, then
    // make sure its network size parameters match those of
    // the current network
    if (pr->outfile.Hydflag == USE)
    {
        fread(&magic, sizeof(INT4), 1, pr->outfile.HydFile);
        if (magic != MAGICNUMBER) return 306;
        fread(&version, sizeof(INT4), 1, pr->outfile.HydFile);
        if (version != ENGINE_VERSION) return 306;
        if (fread(nsize, sizeof(INT4), 6, pr->outfile.HydFile) < 6) return 306;
        if (nsize[0] != Nnodes || nsize[1] != Nlinks || nsize[2] != Ntanks ||
            nsize[3] != Npumps || nsize[4] != Nvalves || 
            nsize[5] != pr->times.Dur
           ) return 306;
        pr->outfile.SaveHflag = TRUE;
    }

    // Save current position in hydraulics file
    // where storage of hydraulic results begins
    pr->outfile.HydOffset = ftell(pr->outfile.HydFile);
    return errcode;
}

int openoutfile(Project *pr)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: opens binary output file.
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    // Close output file if already opened
    if (pr->outfile.OutFile != NULL)
    {
        fclose(pr->outfile.OutFile);
        pr->outfile.OutFile = NULL;
    }
    if (pr->outfile.TmpOutFile != NULL)
    {
        fclose(pr->outfile.TmpOutFile);
        pr->outfile.TmpOutFile = NULL;
    }

    // If output file name was supplied, then attempt to
    // open it. Otherwise open a temporary output file. 
    if (pr->outfile.Outflag == SAVE)
    {
        pr->outfile.OutFile = fopen(pr->outfile.OutFname, "w+b");
        if (pr->outfile.OutFile == NULL) errcode = 304;
    }
    else
    {
        strcpy(pr->outfile.OutFname, pr->TmpOutFname);
        pr->outfile.OutFile = fopen(pr->outfile.OutFname, "w+b");
        if (pr->outfile.OutFile == NULL) errcode = 304;
    }

    // Save basic network data & energy usage results
    ERRCODE(savenetdata(pr));
    pr->outfile.OutOffset1 = ftell(pr->outfile.OutFile);
    ERRCODE(saveenergy(pr));
    pr->outfile.OutOffset2 = ftell(pr->outfile.OutFile);

    // Open temporary file if computing time series statistic
    if (!errcode)
    {
        if (pr->report.Tstatflag != SERIES)
        {
            pr->outfile.TmpOutFile = fopen(pr->TmpStatFname, "w+b");
            if (pr->outfile.TmpOutFile == NULL) errcode = 304;
        }
        else pr->outfile.TmpOutFile = pr->outfile.OutFile;
    }
    return errcode;
}

void initpointers(Project *pr)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes data array pointers to NULL
**----------------------------------------------------------------
*/
{
    pr->hydraul.NodeDemand = NULL;
    pr->hydraul.NodeHead = NULL;
    pr->hydraul.LinkFlow = NULL;
    pr->hydraul.LinkStatus = NULL;
    pr->hydraul.LinkSetting = NULL;
    pr->hydraul.OldStatus = NULL;
    pr->hydraul.P = NULL;
    pr->hydraul.Y = NULL;
    pr->hydraul.Xflow = NULL;

    pr->quality.NodeQual = NULL;
    pr->quality.PipeRateCoeff = NULL;

    pr->network.Node = NULL;
    pr->network.Link = NULL;
    pr->network.Tank = NULL;
    pr->network.Pump = NULL;
    pr->network.Valve = NULL;
    pr->network.Pattern = NULL;
    pr->network.Curve = NULL;
    pr->network.Control = NULL;
    pr->network.Adjlist = NULL;
    pr->network.NodeHashTable = NULL;
    pr->network.LinkHashTable = NULL;

    pr->parser.Patlist = NULL;
    pr->parser.Curvelist = NULL;

    pr->hydraul.smatrix.Aii = NULL;
    pr->hydraul.smatrix.Aij = NULL;
    pr->hydraul.smatrix.F = NULL;
    pr->hydraul.smatrix.Order = NULL;
    pr->hydraul.smatrix.Row = NULL;
    pr->hydraul.smatrix.Ndx = NULL;
    pr->hydraul.smatrix.XLNZ = NULL;
    pr->hydraul.smatrix.NZSUB = NULL;
    pr->hydraul.smatrix.LNZ = NULL;

    initrules(pr);
}

int allocdata(Project *pr)
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

    // Allocate node & link ID hash tables
    pr->network.NodeHashTable = hashtable_create();
    pr->network.LinkHashTable = hashtable_create();
    ERRCODE(MEMCHECK(pr->network.NodeHashTable));
    ERRCODE(MEMCHECK(pr->network.LinkHashTable));

    // Allocate memory for network nodes
    //*************************************************************
    // NOTE: Because network components of a given type are indexed
    // starting from 1, their arrays must be sized 1
    // element larger than the number of components.
    //*************************************************************
    if (!errcode)
    {
        n = pr->parser.MaxNodes + 1;
        pr->network.Node          = (Snode *)calloc(n, sizeof(Snode));
        pr->hydraul.NodeDemand = (double *)calloc(n, sizeof(double));
        pr->hydraul.NodeHead   = (double *)calloc(n, sizeof(double));
        pr->quality.NodeQual      = (double *)calloc(n, sizeof(double));
        ERRCODE(MEMCHECK(pr->network.Node));
        ERRCODE(MEMCHECK(pr->hydraul.NodeDemand));
        ERRCODE(MEMCHECK(pr->hydraul.NodeHead));
        ERRCODE(MEMCHECK(pr->quality.NodeQual));
    }

    // Allocate memory for network links 
    if (!errcode)
    {
        n = pr->parser.MaxLinks + 1;
        pr->network.Link           = (Slink *)calloc(n, sizeof(Slink));
        pr->hydraul.LinkFlow   = (double *)calloc(n, sizeof(double));
        pr->hydraul.LinkSetting = (double *)calloc(n, sizeof(double));
        pr->hydraul.LinkStatus  = (StatusType *)calloc(n, sizeof(StatusType));
        ERRCODE(MEMCHECK(pr->network.Link));
        ERRCODE(MEMCHECK(pr->hydraul.LinkFlow));
        ERRCODE(MEMCHECK(pr->hydraul.LinkSetting));
        ERRCODE(MEMCHECK(pr->hydraul.LinkStatus));
    }

    // Allocate memory for tanks, sources, pumps, valves,
    // controls, demands, time patterns, & operating curves
    if (!errcode)
    {
        pr->network.Tank =
            (Stank *)calloc(pr->parser.MaxTanks + 1, sizeof(Stank));
        pr->network.Pump =
            (Spump *)calloc(pr->parser.MaxPumps + 1, sizeof(Spump));
        pr->network.Valve =
            (Svalve *)calloc(pr->parser.MaxValves + 1, sizeof(Svalve));
        pr->network.Control =
            (Scontrol *)calloc(pr->parser.MaxControls + 1, sizeof(Scontrol));
        pr->network.Pattern =
            (Spattern *)calloc(pr->parser.MaxPats + 1, sizeof(Spattern));
        pr->network.Curve =
            (Scurve *)calloc(pr->parser.MaxCurves + 1, sizeof(Scurve));
        ERRCODE(MEMCHECK(pr->network.Tank));
        ERRCODE(MEMCHECK(pr->network.Pump));
        ERRCODE(MEMCHECK(pr->network.Valve));
        ERRCODE(MEMCHECK(pr->network.Control));
        ERRCODE(MEMCHECK(pr->network.Pattern));
        ERRCODE(MEMCHECK(pr->network.Curve));
    }

    // Initialize pointers used in patterns, curves, and demand category lists
    if (!errcode)
    {
        for (n = 0; n <= pr->parser.MaxPats; n++)
        {
            pr->network.Pattern[n].Length = 0;
            pr->network.Pattern[n].F = NULL;
        }
        for (n = 0; n <= pr->parser.MaxCurves; n++)
        {
            pr->network.Curve[n].Npts = 0;
            pr->network.Curve[n].Type = G_CURVE;
            pr->network.Curve[n].X = NULL;
            pr->network.Curve[n].Y = NULL;
        }
        for (n = 0; n <= pr->parser.MaxNodes; n++)
        {
            pr->network.Node[n].D = NULL;    // node demand
        }
    }

    // Allocate memory for rule base (see RULES.C)
    if (!errcode) errcode = allocrules(pr);
    return errcode;
}

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
    while (t != NULL)
    {
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
    while (f != NULL)
    {
        fnext = f->next;
        free(f);
        f = fnext;
    }
}

void freedata(Project *pr)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: frees memory allocated for network data structures.
**----------------------------------------------------------------
*/
{
    int j;
    Pdemand demand, nextdemand;
    Psource source;

    // Free memory for computed results
    free(pr->hydraul.NodeDemand);
    free(pr->hydraul.NodeHead);
    free(pr->hydraul.LinkFlow);
    free(pr->hydraul.LinkSetting);
    free(pr->hydraul.LinkStatus);
    free(pr->quality.NodeQual);

    // Free memory used for nodal adjacency lists
    freeadjlists(&pr->network);

    // Free memory for node data
    if (pr->network.Node != NULL)
    {
        for (j = 0; j <= pr->parser.MaxNodes; j++)
        {
            // Free memory used for demand category list
            demand = pr->network.Node[j].D;
            while (demand != NULL)
            {
                nextdemand = demand->next;
                free(demand);
                demand = nextdemand;
            }
            // Free memory used for WQ source data 
            source = pr->network.Node[j].S;
            if (source != NULL) free(source);
        }
        free(pr->network.Node);
    }

    // Free memory for other network objects
    free(pr->network.Link);
    free(pr->network.Tank);
    free(pr->network.Pump);
    free(pr->network.Valve);
    free(pr->network.Control);

    // Free memory for time patterns
    if (pr->network.Pattern != NULL)
    {
        for (j = 0; j <= pr->parser.MaxPats; j++) free(pr->network.Pattern[j].F);
        free(pr->network.Pattern);
    }

    // Free memory for curves
    if (pr->network.Curve != NULL)
    {
        for (j = 0; j <= pr->parser.MaxCurves; j++)
        {
            free(pr->network.Curve[j].X);
            free(pr->network.Curve[j].Y);
        }
        free(pr->network.Curve);
    }

    // Free memory for rule base (see RULES.C)
    freerules(pr);

    // Free hash table memory
    if (pr->network.NodeHashTable != NULL)
    {
        hashtable_free(pr->network.NodeHashTable);
    }
    if (pr->network.LinkHashTable != NULL)
    {
        hashtable_free(pr->network.LinkHashTable);
    }
}


int  buildadjlists(Network *net)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: builds linked list of links adjacent to each node
**--------------------------------------------------------------
*/
{
    int       i, j, k;
    int       errcode = 0;
    Padjlist  alink;

    // Create an array of adjacency lists
    freeadjlists(net);
    net->Adjlist = (Padjlist *)calloc(net->Nnodes + 1, sizeof(Padjlist));
    if (net->Adjlist == NULL) return 101;
    for (i = 0; i <= net->Nnodes; i++) net->Adjlist[i] = NULL;

    // For each link, update adjacency lists of its end nodes
    for (k = 1; k <= net->Nlinks; k++)
    {
        i = net->Link[k].N1;
        j = net->Link[k].N2;

        // Include link in start node i's list
        alink = (struct Sadjlist *) malloc(sizeof(struct Sadjlist));
        if (alink == NULL)
        {
            errcode = 101;
            break;
        }
        alink->node = j;
        alink->link = k;
        alink->next = net->Adjlist[i];
        net->Adjlist[i] = alink;

        // Include link in end node j's list
        alink = (struct Sadjlist *) malloc(sizeof(struct Sadjlist));
        if (alink == NULL)
        {
            errcode = 101;
            break;
        }
        alink->node = i;
        alink->link = k;
        alink->next = net->Adjlist[j];
        net->Adjlist[j] = alink;
    }
    if (errcode) freeadjlists(net);
    return errcode;
}


void  freeadjlists(Network *net)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  none
** Purpose: frees memory used for nodal adjacency lists
**--------------------------------------------------------------
*/
{
    int   i;
    Padjlist alink;

    if (net->Adjlist == NULL) return;
    for (i = 0; i <= net->Nnodes; i++)
    {
        for (alink = net->Adjlist[i]; alink != NULL; alink = net->Adjlist[i])
        {
            net->Adjlist[i] = alink->next;
            free(alink);
        }
    }
    FREE(net->Adjlist);
}

int incontrols(Project *pr, int objType, int index)
/*----------------------------------------------------------------
**  Input:   objType = type of object (either NODE or LINK)
**           index  = the object's index
**  Output:  none
**  Returns: 1 if any controls contain the object; 0 if not
**  Purpose: determines if any simple or rule-based controls
**           contain a particular node or link.
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Rules *rules = &pr->rules;

    int i, ruleObject;
    Spremise *premise;
    Saction *action;

    // Check simple controls
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (objType == NODE && net->Control[i].Node == index) return 1;
        if (objType == LINK && net->Control[i].Link == index) return 1;
    }

    // Check rule-based controls
    for (i = 1; i <= net->Nrules; i++)
    {
        // Convert objType to a rule object type
        if (objType == NODE) ruleObject = 6;
        else                 ruleObject = 7;

        // Check rule's premises
        premise = net->Rule[i].Premises;
        while (premise != NULL)
        {
            if (ruleObject == premise->object && premise->index == index) return 1;
            premise = premise->next;
        }

        // Rule actions only need to be checked for link objects
        if (objType == LINK)
        {
            // Check rule's THEN actions
            action = net->Rule[i].ThenActions;
            while (action != NULL)
            {
                if (action->link == index) return 1;
                action = action->next;
            }

            // Check rule's ELSE actions
            action = net->Rule[i].ElseActions;
            while (action != NULL)
            {
                if (action->link == index) return 1;
                action = action->next;
            }
        }
    }
    return 0;
}

int findnode(Network *network, char *id)
/*----------------------------------------------------------------
**  Input:   id = node ID
**  Output:  none
**  Returns: index of node with given ID, or 0 if ID not found
**  Purpose: uses hash table to find index of node with given ID
**----------------------------------------------------------------
*/
{
    return (hashtable_find(network->NodeHashTable, id));
}

int findlink(Network *network, char *id)
/*----------------------------------------------------------------
**  Input:   id = link ID
**  Output:  none
**  Returns: index of link with given ID, or 0 if ID not found
**  Purpose: uses hash table to find index of link with given ID
**----------------------------------------------------------------
*/
{
    return (hashtable_find(network->LinkHashTable, id));
}

int findtank(Network *network, int index)
/*----------------------------------------------------------------
**  Input:   index = node index
**  Output:  none
**  Returns: index of tank with given node id, or NOTFOUND if tank not found
**  Purpose: for use in the deletenode function
**----------------------------------------------------------------
*/
{
    int i;
    for (i = 1; i <= network->Ntanks; i++)
    {
        if (network->Tank[i].Node == index) return i;
    }
    return NOTFOUND;
}

int findpump(Network *network, int index)
/*----------------------------------------------------------------
**  Input:   index = link ID
**  Output:  none
**  Returns: index of pump with given link id, or NOTFOUND if pump not found
**  Purpose: for use in the deletelink function
**----------------------------------------------------------------
*/
{
    int i;
    for (i = 1; i <= network->Npumps; i++)
    {
        if (network->Pump[i].Link == index) return i;
    }
    return NOTFOUND;
}

int findvalve(Network *network, int index)
/*----------------------------------------------------------------
**  Input:   index = link ID
**  Output:  none
**  Returns: index of valve with given link id, or NOTFOUND if valve not found
**  Purpose: for use in the deletelink function
**----------------------------------------------------------------
*/
{
    int i;
    for (i = 1; i <= network->Nvalves; i++)
    {
        if (network->Valve[i].Link == index) return i;
    }
    return NOTFOUND;
}

char *getTmpName(char *fname)
//----------------------------------------------------------------
//  Input:   fname = file name string
//  Output:  returns pointer to file name
//  Purpose: creates a temporary file name with path prepended to it.
//----------------------------------------------------------------
{
#ifdef _WIN32

    char* name = NULL;

    // --- use Windows _tempnam function to get a pointer to an
    //     unused file name that begins with "en"
    name = _tempnam(NULL, "en");
    if (name == NULL) return NULL;

    // --- copy the file name to fname
    if (strlen(name) < MAXFNAME) strncpy(fname, name, MAXFNAME);
    else fname = NULL;

    // --- free the pointer returned by _tempnam
    if (name) free(name);

    // --- for non-Windows systems:
#else
    // --- use system function mkstemp() to create a temporary file name
    strcpy(fname, "enXXXXXX");
    mkstemp(fname);
#endif
    return fname;
}

int strcomp(const char *s1, const char *s2)
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
    {
        if (!s1[i + 1] && !s2[i + 1]) return 1;
    }
    return 0;
}

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

    m = n - 1;                        // Highest data index
    if (xx <= x[0]) return (y[0]);    // xx off low end of curve 
    for (k = 1; k <= m; k++)          // Bracket xx on curve
    {
        if (x[k] >= xx)               // Interp. over interval
        {
            dx = x[k] - x[k - 1];
            dy = y[k] - y[k - 1];
            if (ABS(dx) < TINY) return (y[k]);
            else return (y[k] - (x[k] - xx) * dy / dx);
        }
    }
    return (y[m]); // xx off high end of curve
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
    switch (errcode)
    {

//#define DAT(code,string) case code: sprintf(msg, "%s", string); break;
#define DAT(code,string) case code: strcpy(msg, string); break;
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
    char errmsg[MAXMSG + 1] = "";
    if (errcode == 309) /* Report file write error -  */
    {                   /* Do not write msg to file.  */

    }
    else if (pr->report.RptFile != NULL && pr->report.Messageflag)
    {
        sprintf(pr->Msg, "Error %d: %s", errcode, geterrmsg(errcode, errmsg));
        writeline(pr, pr->Msg);
    }
}

void writewin(void(*vp)(char *), char *s)
/*----------------------------------------------------------------
**  Input:   text string
**  Output:  none
**  Purpose: passes character string to viewprog() in
**           application which calls the EPANET DLL
**----------------------------------------------------------------
*/
{
    char progmsg[MAXMSG + 1];
    if (vp != NULL)
    {
        strncpy(progmsg, s, MAXMSG);
        vp(progmsg);
    }
}
