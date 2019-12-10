/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       project.c
 Description:  project data management routines
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/15/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> 

//*** For the Windows SDK _tempnam function ***//
#ifdef _WIN32
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
    pr->outfile.TmpOutFile = NULL;

    // Save file names
    strncpy(pr->parser.InpFname, f1, MAXFNAME);
    strncpy(pr->report.Rpt1Fname, f2, MAXFNAME);
    strncpy(pr->outfile.OutFname, f3, MAXFNAME);
    if (strlen(f3) > 0) pr->outfile.Outflag = SAVE;
    else
    {
        pr->outfile.Outflag = SCRATCH;
        strcpy(pr->outfile.OutFname, pr->TmpOutFname);
    }

    // Check that file names are not identical
    if (strlen(f1) > 0 && (strcomp(f1, f2) || strcomp(f1, f3))) return 301;
    if (strlen(f3) > 0 && strcomp(f2, f3)) return 301;

    // Attempt to open input and report files
    if (strlen(f1) > 0)
    {
        if ((pr->parser.InFile = fopen(f1, "rt")) == NULL) return 302;
    }
    if (strlen(f2) == 0) pr->report.RptFile = stdout;
    else
    {
        pr->report.RptFile = fopen(f2, "wt");
        if (pr->report.RptFile == NULL) return 303;
    }
    writelogo(pr);
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
        pr->outfile.HydFile = NULL;
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
    closeoutfile(pr);

    // Try to open binary output file
    pr->outfile.OutFile = fopen(pr->outfile.OutFname, "w+b");
    if (pr->outfile.OutFile == NULL) return 304;

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

void closeoutfile(Project *pr)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: closes binary output file.
**----------------------------------------------------------------
*/
{
    if (pr->outfile.TmpOutFile != pr->outfile.OutFile)
    {
        if (pr->outfile.TmpOutFile != NULL)
        {
            fclose(pr->outfile.TmpOutFile);
            pr->outfile.TmpOutFile = NULL;
        }
    }
    if (pr->outfile.OutFile != NULL)
    {
        if (pr->outfile.OutFile == pr->outfile.TmpOutFile)
        {
            pr->outfile.TmpOutFile = NULL;
        }
        fclose(pr->outfile.OutFile);
        pr->outfile.OutFile = NULL;
    }
}

void initpointers(Project *pr)
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes data array pointers to NULL
**----------------------------------------------------------------
*/
{
    Network* nw = &pr->network;
    nw->Nnodes = 0;
    nw->Ntanks = 0;
    nw->Njuncs = 0;
    nw->Nlinks = 0;
    nw->Npipes = 0;
    nw->Npumps = 0;
    nw->Nvalves = 0;
    nw->Ncontrols = 0;
    nw->Nrules = 0;
    nw->Npats = 0;
    nw->Ncurves = 0;

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
        pr->network.Node       = (Snode *)calloc(n, sizeof(Snode));
        pr->hydraul.NodeDemand = (double *)calloc(n, sizeof(double));
        pr->hydraul.NodeHead   = (double *)calloc(n, sizeof(double));
        pr->quality.NodeQual   = (double *)calloc(n, sizeof(double));
        ERRCODE(MEMCHECK(pr->network.Node));
        ERRCODE(MEMCHECK(pr->hydraul.NodeDemand));
        ERRCODE(MEMCHECK(pr->hydraul.NodeHead));
        ERRCODE(MEMCHECK(pr->quality.NodeQual));
    }

    // Allocate memory for network links
    if (!errcode)
    {
        n = pr->parser.MaxLinks + 1;
        pr->network.Link        = (Slink *)calloc(n, sizeof(Slink));
        pr->hydraul.LinkFlow    = (double *)calloc(n, sizeof(double));
        pr->hydraul.LinkSetting = (double *)calloc(n, sizeof(double));
        pr->hydraul.LinkStatus  = (StatusType *)calloc(n, sizeof(StatusType));
        ERRCODE(MEMCHECK(pr->network.Link));
        ERRCODE(MEMCHECK(pr->hydraul.LinkFlow));
        ERRCODE(MEMCHECK(pr->hydraul.LinkSetting));
        ERRCODE(MEMCHECK(pr->hydraul.LinkStatus));
    }

    // Allocate memory for tanks, sources, pumps, valves, and controls
    // (memory for Patterns and Curves arrays expanded as each is added)
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
        ERRCODE(MEMCHECK(pr->network.Tank));
        ERRCODE(MEMCHECK(pr->network.Pump));
        ERRCODE(MEMCHECK(pr->network.Valve));
        ERRCODE(MEMCHECK(pr->network.Control));
    }

    // Initialize pointers used in nodes and links
    if (!errcode)
    {
        for (n = 0; n <= pr->parser.MaxNodes; n++)
        {
            pr->network.Node[n].D = NULL;    // node demand
            pr->network.Node[n].S = NULL;    // node source
            pr->network.Node[n].Comment = NULL;
        }
        for (n = 0; n <= pr->parser.MaxLinks; n++)
        {
            pr->network.Link[n].Vertices = NULL;
            pr->network.Link[n].Comment = NULL;
        }
    }

    // Allocate memory for rule base (see RULES.C)
    if (!errcode) errcode = allocrules(pr);
    return errcode;
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
        for (j = 1; j <= pr->network.Nnodes; j++)
        {
            // Free memory used for demands and WQ source data
            freedemands(&(pr->network.Node[j]));
            free(pr->network.Node[j].S);
            free(pr->network.Node[j].Comment);
        }
        free(pr->network.Node);
    }

    // Free memory for link data
    if (pr->network.Link != NULL)
    {
        for (j = 1; j <= pr->network.Nlinks; j++)
        {
            freelinkvertices(&pr->network.Link[j]);
            free(pr->network.Link[j].Comment);
        }
    }
    free(pr->network.Link);

    // Free memory for other network objects
    free(pr->network.Tank);
    free(pr->network.Pump);
    free(pr->network.Valve);
    free(pr->network.Control);

    // Free memory for time patterns
    if (pr->network.Pattern != NULL)
    {
        for (j = 0; j <= pr->network.Npats; j++)
        {
            free(pr->network.Pattern[j].F);
            free(pr->network.Pattern[j].Comment);
        }
        free(pr->network.Pattern);
    }

    // Free memory for curves
    if (pr->network.Curve != NULL)
    {
        // There is no Curve[0]
        for (j = 1; j <= pr->network.Ncurves; j++)
        {
            free(pr->network.Curve[j].X);
            free(pr->network.Curve[j].Y);
            free(pr->network.Curve[j].Comment);
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

Pdemand finddemand(Pdemand d, int index)
/*----------------------------------------------------------------
**  Input:   d = pointer to start of a list of demands
**           index = the position of the demand to retrieve
**  Output:  none
**  Returns: the demand at the requested position
**  Purpose: finds the demand at a given position in a demand list
**----------------------------------------------------------------
*/
{
    int n = 1;
    if (index <= 0)return NULL;
    while (d)
    {
        if (n == index) break;
        n++;
        d = d->next;
    }
    return d;
}

int adddemand(Snode *node, double dbase, int dpat, char *dname)
/*----------------------------------------------------------------
**  Input:   node = a network junction node
**           dbase = base demand value
**           dpat = demand pattern index
**           dname = name of demand category
**  Output:  returns TRUE if successful, FALSE if not
**  Purpose: adds a new demand category to a node.
**----------------------------------------------------------------
*/
{
    Pdemand demand, lastdemand;

    // Create a new demand struct
    demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
    if (demand == NULL) return FALSE;

    // Assign it the designated properties
    demand->Base = dbase;
    demand->Pat = dpat;
    demand->Name = NULL;
    if (dname && strlen(dname) > 0) xstrcpy(&demand->Name, dname, MAXID);
    demand->next = NULL;

    // If node has no demands make this its first demand category
    if (node->D == NULL) node->D = demand;

    // Otherwise append this demand to the end of the node's demands list
    else
    {
        lastdemand = node->D;
        while (lastdemand->next) lastdemand = lastdemand->next;
        lastdemand->next = demand;
    }
    return TRUE;
}

void freedemands(Snode *node)
/*----------------------------------------------------------------
**  Input:   node = a network junction node
**  Output:  node
**  Purpose: frees the memory used for a node's list of demands.
**----------------------------------------------------------------
*/
{
    Pdemand nextdemand;
    Pdemand demand = node->D;
    while (demand != NULL)
    {
        nextdemand = demand->next;
        free(demand->Name);
        free(demand);
        demand = nextdemand;
    }
    node->D = NULL;
}

int  addlinkvertex(Slink *link, double x, double y)
/*----------------------------------------------------------------
**  Input:   link = pointer to a network link
**           x = x-coordinate of a new vertex
**           y = y-coordiante of a new vertex
**  Returns: an error code
**  Purpose: adds to a link's collection of vertex points.
**----------------------------------------------------------------
*/
{
    static int CHUNKSIZE = 5;
    int n;
    Pvertices vertices;
    if (link->Vertices == NULL)
    {
        vertices = (struct Svertices *) malloc(sizeof(struct Svertices));
        if (vertices == NULL) return 101;
        vertices->Npts = 0;
        vertices->Capacity = CHUNKSIZE;
        vertices->X = (double *) calloc(vertices->Capacity, sizeof(double));
        vertices->Y = (double *) calloc(vertices->Capacity, sizeof(double));
        link->Vertices = vertices;
    }
    vertices = link->Vertices;
    if (vertices->Npts >= vertices->Capacity)
    {
        vertices->Capacity += CHUNKSIZE;
        vertices->X = realloc(vertices->X, vertices->Capacity * sizeof(double));
        vertices->Y = realloc(vertices->Y, vertices->Capacity * sizeof(double));
    }
    if (vertices->X == NULL || vertices->Y == NULL) return 101;
    n = vertices->Npts;
    vertices->X[n] = x;
    vertices->Y[n] = y;
    vertices->Npts++;
    return 0;    
}

void freelinkvertices(Slink *link)
/*----------------------------------------------------------------
**  Input:   vertices = list of link vertex points
**  Output:  none
**  Purpose: frees the memory used for a link's list of vertices.
**----------------------------------------------------------------
*/
{
    if (link->Vertices)
    {
        free(link->Vertices->X);
        free(link->Vertices->Y);
        free(link->Vertices);
        link->Vertices = NULL;
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

int valvecheck(Project *pr, int index, int type, int j1, int j2)
/*
**--------------------------------------------------------------
**  Input:   index = link index
**           type = valve type
**           j1   = index of upstream node
**           j2   = index of downstream node
**  Output:  returns an error code
**  Purpose: checks for illegal connections between valves
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int k, vj1, vj2;
    LinkType vtype;
    Slink *link;
    Svalve *valve;

    if (type == PRV || type == PSV || type == FCV)
    {
        // Can't be connected to a fixed grade node
        if (j1 > net->Njuncs || j2 > net->Njuncs) return 219;

        // Examine each existing valve
        for (k = 1; k <= net->Nvalves; k++)
        {
            valve = &net->Valve[k];
            if (valve->Link == index) continue;
            link = &net->Link[valve->Link];
            vj1 = link->N1;
            vj2 = link->N2;
            vtype = link->Type;

            // Cannot have two PRVs sharing downstream nodes or in series
            if (vtype == PRV && type == PRV)
            {
                if (vj2 == j2 || vj2 == j1 || vj1 == j2) return 220;
            }

            // Cannot have two PSVs sharing upstream nodes or in series
            if (vtype == PSV && type == PSV)
            {
                if (vj1 == j1 || vj1 == j2 || vj2 == j1) return 220;
            }

            // Cannot have PSV connected to downstream node of PRV
            if (vtype == PSV && type == PRV && vj1 == j2) return 220;
            if (vtype == PRV && type == PSV && vj2 == j1) return 220;

            // Cannot have PSV connected to downstream node of FCV
            // nor have PRV connected to upstream node of FCV
            if (vtype == FCV && type == PSV && vj2 == j1) return 220;
            if (vtype == FCV && type == PRV && vj1 == j2) return 220;
            if (vtype == PSV && type == FCV && vj1 == j2) return 220;
            if (vtype == PRV && type == FCV && vj2 == j1) return 220;
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

int findpattern(Network *network, char *id)
/*----------------------------------------------------------------
**  Input:   id = time pattern ID
**  Output:  none
**  Returns: time pattern index, or -1 if pattern not found
**  Purpose: finds index of time pattern given its ID
**----------------------------------------------------------------
*/
{
    int i;

    // Don't forget to include the "dummy" pattern 0 in the search
    for (i = 0; i <= network->Npats; i++)
    {
        if (strcmp(id, network->Pattern[i].ID) == 0) return i;
    }
    return -1;
}

int findcurve(Network *network, char *id)
/*----------------------------------------------------------------
**  Input:   id = data curve ID
**  Output:  none
**  Returns: data curve index, or 0 if curve not found
**  Purpose: finds index of data curve given its ID
**----------------------------------------------------------------
*/
{
    int i;
    for (i = 1; i <= network->Ncurves; i++)
    {
        if (strcmp(id, network->Curve[i].ID) == 0) return i;
    }
    return 0;
}

void adjustpattern(int *pat, int index)
/*----------------------------------------------------------------
** Local function that modifies a reference to a deleted time pattern
**----------------------------------------------------------------
*/
{
	if (*pat == index) *pat = 0;
	else if (*pat > index) (*pat)--;
}

void adjustpatterns(Network *network, int index)
/*----------------------------------------------------------------
**  Input:   index = index of time pattern being deleted
**  Output:  none
**  Purpose: modifies references made to a deleted time pattern
**----------------------------------------------------------------
*/
{
    int j;
    Pdemand demand;
    Psource source;

    // Adjust patterns used by junctions
    for (j = 1; j <= network->Nnodes; j++)
    {
        // Adjust demand patterns
        for (demand = network->Node[j].D; demand != NULL; demand = demand->next)
        {
            adjustpattern(&demand->Pat, index);
        }
        // Adjust WQ source patterns
        source = network->Node[j].S;
        if (source) adjustpattern(&source->Pat, index);
    }

    // Adjust patterns used by reservoir tanks
    for (j = 1; j <= network->Ntanks; j++)
    {
        adjustpattern(&network->Tank[j].Pat, index);
    }

    // Adjust patterns used by pumps
    for (j = 1; j <= network->Npumps; j++)
    {
        adjustpattern(&network->Pump[j].Upat, index);
        adjustpattern(&network->Pump[j].Epat, index);
    }
}

void adjustcurve(int *curve, int index)
/*----------------------------------------------------------------
** Local function that modifies a reference to a deleted data curve
**----------------------------------------------------------------
*/
{
    if (*curve == index) *curve = 0;
    else if (*curve > index) (*curve)--;
}

void adjustcurves(Network *network, int index)
/*----------------------------------------------------------------
**  Input:   index = index of data curve being deleted
**  Output:  none
**  Purpose: modifies references made to a deleted data curve
**----------------------------------------------------------------
*/
{
    int j, k, setting;

    // Adjust tank volume curves
    for (j = 1; j <= network->Ntanks; j++)
    {
        adjustcurve(&network->Tank[j].Vcurve, index);
    }

    // Adjust pump curves
    for (j = 1; j <= network->Npumps; j++)
    {
        adjustcurve(&network->Pump[j].Hcurve, index);
        adjustcurve(&network->Pump[j].Ecurve, index);
    }

    // Adjust GPV curves
    for (j = 1; j <= network->Nvalves; j++)
    {
        k = network->Valve[j].Link;
        if (network->Link[k].Type == GPV)
        {
            setting = INT(network->Link[k].Kc);
            adjustcurve(&setting, index);
            network->Link[k].Kc = setting;
        }
    }
}

int adjustpumpparams(Project *pr, int curveIndex)
/*----------------------------------------------------------------
**  Input:   curveIndex = index of a data curve
**  Output:  returns an error code
**  Purpose: updates head curve parameters for pumps using a 
**           curve whose data have been modified.
**----------------------------------------------------------------
*/
{
    Network *network = &pr->network;

    double *Ucf = pr->Ucf;
    int j, err = 0;
    Spump *pump;
    
    // Check each pump
    for (j = 1; j <= network->Npumps; j++)
    {
        // Pump uses curve as head curve
        pump = &network->Pump[j];
        if ( curveIndex == pump->Hcurve)
        {
            // Update its head curve parameters
            pump->Ptype = NOCURVE;
            err = updatepumpparams(pr, curveIndex);
            if (err > 0) break;
            
            // Convert parameters to internal units
            if (pump->Ptype == POWER_FUNC)
            {
                pump->H0 /= Ucf[HEAD];
                pump->R *= (pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
            }
            pump->Q0 /= Ucf[FLOW];
            pump->Qmax /= Ucf[FLOW];
            pump->Hmax /= Ucf[HEAD];
        }
    }
    return err;
}
        

int resizecurve(Scurve *curve, int size)
/*----------------------------------------------------------------
**  Input:   curve = a data curve object
**           size = desired number of curve data points
**  Output:  error code
**  Purpose: resizes a data curve to a desired size
**----------------------------------------------------------------
*/
{
    double *x;
    double *y;

    if (curve->Capacity < size)
    {
        x = (double *)realloc(curve->X, size * sizeof(double));
        if (x == NULL) return 101;
        y = (double *)realloc(curve->Y, size * sizeof(double));
        if (y == NULL)
        {
            free(x);
            return 101;
        }
        curve->X = x;
        curve->Y = y;
        curve->Capacity = size;
    }
    return 0;
}

int  getcomment(Network *network, int object, int index, char *comment)
//----------------------------------------------------------------
//  Input:   object = a type of network object
//           index = index of the specified object
//           comment = the object's comment string
//  Output:  error code
//  Purpose: gets the comment string assigned to an object.
//----------------------------------------------------------------
{
    char *currentcomment;

    // Get pointer to specified object's comment
    switch (object)
    {
    case NODE:
        if (index < 1 || index > network->Nnodes) return 251;
        currentcomment = network->Node[index].Comment;
        break;
    case LINK:
        if (index < 1 || index > network->Nlinks) return 251;
        currentcomment = network->Link[index].Comment;
        break;
    case TIMEPAT:
        if (index < 1 || index > network->Npats) return 251;
        currentcomment = network->Pattern[index].Comment;
        break;
    case CURVE:
        if (index < 1 || index > network->Ncurves) return 251;
        currentcomment = network->Curve[index].Comment;
        break;
    default:
        strcpy(comment, "");
        return 251;
    }

    // Copy the object's comment to the returned string
    if (currentcomment) strcpy(comment, currentcomment);
    else comment[0] = '\0';
    return 0;
}

int setcomment(Network *network, int object, int index, const char *newcomment)
//----------------------------------------------------------------
//  Input:   object = a type of network object
//           index = index of the specified object
//           newcomment = new comment string
//  Output:  error code
//  Purpose: sets the comment string of an object.
//----------------------------------------------------------------
{
    char *comment;

    switch (object)
    {
    case NODE:
        if (index < 1 || index > network->Nnodes) return 251;
        comment = network->Node[index].Comment;
        network->Node[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
        return 0;

    case LINK:
        if (index < 1 || index > network->Nlinks) return 251;
        comment = network->Link[index].Comment;
        network->Link[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
        return 0;

    case TIMEPAT:
        if (index < 1 || index > network->Npats) return 251;
        comment = network->Pattern[index].Comment;
        network->Pattern[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
        return 0;

    case CURVE:
        if (index < 1 || index > network->Ncurves) return 251;
        comment = network->Curve[index].Comment;
        network->Curve[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
        return 0;

    default: return 251;
    }
}

int namevalid(const char *name)
//----------------------------------------------------------------
//  Input:   name = name used to ID an object
//  Output:  returns TRUE if name is valid, FALSE if not
//  Purpose: checks that an object's ID name is valid.
//----------------------------------------------------------------
{
    size_t n = strlen(name);
    if (n < 1 || n > MAXID || strpbrk(name, " ;") || name[0] == '"') return FALSE;
    return TRUE;
}

void getTmpName(char *fname)
//----------------------------------------------------------------
//  Input:   fname = file name string
//  Output:  an unused file name
//  Purpose: creates a temporary file name with an "en" prefix
//           or a blank name if an error occurs.
//----------------------------------------------------------------
{
#ifdef _WIN32

    char* name = NULL;

    // --- use Windows _tempnam function to get a pointer to an
    //     unused file name that begins with "en"
    strcpy(fname, "");
    name = _tempnam(NULL, "en");
    if (name)
    {
        // --- copy the file name to fname
        if (strlen(name) < MAXFNAME) strncpy(fname, name, MAXFNAME);

        // --- free the pointer returned by _tempnam
        free(name);
    }
    // --- for non-Windows systems:
#else
    // --- use system function mkstemp() to create a temporary file name
/*    
    int f = -1;
    strcpy(fname, "enXXXXXX");
    f = mkstemp(fname);
    close(f);
    remove(fname);
*/
    strcpy(fname, "enXXXXXX");
    FILE *f = fdopen(mkstemp(fname), "r");
    if (f == NULL) strcpy(fname, "");
    else fclose(f);
    remove(fname);
#endif
}

char *xstrcpy(char **s1, const char *s2, const size_t n)
//----------------------------------------------------------------
//  Input:   s1 = destination string
//           s2 = source string
//           n = maximum size of strings
//  Output:  none
//  Purpose: like strcpy except for dynamic strings.
//  Note:    The calling program is responsible for ensuring that
//           s1 points to a valid memory location or is NULL. E.g.,
//           the following code will likely cause a segment fault:
//             char *s;
//             s = xstrcpy(s, "Some text");
//           while this would work correctly:
//             char *s = NULL;
//             s = xstrcpy(s, "Some text");
//----------------------------------------------------------------
{
    size_t n1 = 0, n2 = 0;

    // Find size of source string
    if (s2) n2 = strlen(s2);
    if (n2 > n) n2 = n;

    // Source string is empty -- free destination string
    if (n2 == 0)
    {
        free(*s1);
        *s1 = NULL;
        return NULL;
    }

    // See if size of destination string needs to grow
    if (*s1) n1 = strlen(*s1);
    if (n2 > n1) *s1 = realloc(*s1, (n2 + 1) * sizeof(char));

    // Copy the source string into the destination string
    strncpy(*s1, s2, n2+1);
    return *s1;
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
    else if (pr->report.RptFile != NULL && pr->report.Messageflag && errcode > 100)
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
