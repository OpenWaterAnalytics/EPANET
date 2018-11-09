
/*
*******************************************************************************

PROJECTS.C

VERSION:      2.2

DATE CREATED: November 9, 2018
AUTHOR:       Michael Tryby
              US EPA ORD/NRMRL

*******************************************************************************
*/


#include "funcs.h"
#include "types.h"



/*
----------------------------------------------------------------
   Functions for opening files
----------------------------------------------------------------
*/

int openfiles(EN_Project *p, const char *f1, const char *f2, const char *f3)
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
    return 301;
  }

  /* Attempt to open input and report files */
  if ((par->InFile = fopen(f1, "rt")) == NULL) {
    return 302;
  }
  if (strlen(f2) == 0)
    rep->RptFile = stdout;
  else if ((rep->RptFile = fopen(f2, "wt")) == NULL) {
    return 303;
  }

  return 0;
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
      return set_error(p->error_handle, 0);
    fclose(out->HydFile);
  }

  /* Use Hydflag to determine the type of hydraulics file to use. */
  /* Write error message if the file cannot be opened.            */
  out->HydFile = NULL;
  switch (out->Hydflag) {
  case SCRATCH:
    getTmpName(p, out->HydFname);
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
    return set_error(p->error_handle, 305);

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
      return set_error(p->error_handle, 306);
    fread(&version, sizeof(INT4), 1, out->HydFile);
    if (version != ENGINE_VERSION)
      return set_error(p->error_handle, 306);
    if (fread(nsize, sizeof(INT4), 6, out->HydFile) < 6)
      return set_error(p->error_handle, 306);
    if (nsize[0] != Nnodes || nsize[1] != Nlinks || nsize[2] != Ntanks ||
        nsize[3] != Npumps || nsize[4] != Nvalves || nsize[5] != time->Dur)
      return set_error(p->error_handle, 306);
    p->save_options.SaveHflag = TRUE;
  }

  /* Save current position in hydraulics file  */
  /* where storage of hydraulic results begins */
  out->HydOffset = ftell(out->HydFile);

  return errcode;
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
      errcode = 304;
    }
  }
  // else if ( (OutFile = tmpfile()) == NULL)
  else
  {
    getTmpName(p, out->OutFname);
    if ((out->OutFile = fopen(out->OutFname, "w+b")) == NULL)
    {
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
      getTmpName(p, out->TmpFname);
      out->TmpOutFile = fopen(out->TmpFname, "w+b");
      if (out->TmpOutFile == NULL)
        errcode = 304;
    } else
      out->TmpOutFile = out->OutFile;
  }

  return errcode;
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
  EN_Network *net;
  hydraulics_t *hyd;
  quality_t *qu;
  parser_data_t *par;

  /* Allocate node & link ID hash tables */
  p->network.NodeHashTable = hashtable_create();
  p->network.LinkHashTable = hashtable_create();
  ERRCODE(MEMCHECK(p->network.NodeHashTable));
  ERRCODE(MEMCHECK(p->network.LinkHashTable));

  net = &p->network;
  hyd = &p->hydraulics;
  qu = &p->quality;
  par = &p->parser;

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
      net->Curve[n].Type = G_CURVE;
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

  return errcode;
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
  if (net->NodeHashTable != NULL) hashtable_free(net->NodeHashTable);

  if (net->LinkHashTable != NULL) hashtable_free(net->LinkHashTable);
}

/*
----------------------------------------------------------------
   General purpose functions
----------------------------------------------------------------
*/

char *getTmpName(EN_Project *p, char *fname)
//
//  Input:   fname = file name string
//  Output:  returns pointer to file name
//  Purpose: creates a temporary file name with path prepended to it.
//
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
