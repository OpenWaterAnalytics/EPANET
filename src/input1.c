/*
*********************************************************************

INPUT1.C -- Input Functions for EPANET Program

VERSION:    2.00
DATE:       5/30/00
            9/7/00
            11/19/01
            6/24/02
            2/14/08     (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL

  This module initializes, retrieves, and adjusts the input
  data for a network simulation.

  The entry point for this module is:
     getdata() -- called from ENopen() in EPANET.C.

*********************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "hash.h"
#include "text.h"
#include "types.h"
#include "epanet2.h"
#include "funcs.h"
#include <math.h>
#define EXTERN extern
#include "vars.h"

/*
  --------------------- Module Global Variables  ----------------------
*/

#define MAXITER  200 /* Default max. # hydraulic iterations    */ 
#define HACC 0.001   /* Default hydraulics convergence ratio   */
#define HTOL 0.0005  /* Default hydraulic head tolerance (ft)  */

/*** Updated 11/19/01 ***/
#define QTOL 0.0001 /* Default flow rate tolerance (cfs)      */

#define AGETOL 0.01  /* Default water age tolerance (hrs)      */
#define CHEMTOL 0.01 /* Default concentration tolerance        */
#define PAGESIZE 0   /* Default uses no page breaks            */
#define SPGRAV 1.0   /* Default specific gravity               */
#define EPUMP 75     /* Default pump efficiency                */
#define DEFPATID "1" /* Default demand pattern ID              */

/*
  These next three parameters are used in the hydraulics solver:
*/

#define RQTOL 1E-7  /* Default low flow resistance tolerance  */
#define CHECKFREQ 2 /* Default status check frequency         */
#define MAXCHECK 10 /* Default # iterations for status checks */
#define DAMPLIMIT 0 /* Default damping threshold              */ 

extern char *Fldname[]; /* Defined in enumstxt.h in EPANET.C      */
extern char *RptFlowUnitsTxt[];

int getdata(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: reads in network data from disk file
**----------------------------------------------------------------
*/
{
  int errcode = 0;
  setdefaults(pr);           /* Assign default data values     */
  initreport(&pr->report);   /* Initialize reporting options   */
  rewind(pr->parser.InFile); /* Rewind input file              */
  ERRCODE(readdata(pr));       /* Read in network data           */
  if (!errcode)
    adjustdata(pr); /* Adjust data for default values */
  if (!errcode)
    initunits(pr);        /* Initialize units on input data */
  ERRCODE(inittanks(pr)); /* Initialize tank volumes        */
  if (!errcode)
    convertunits(pr); /* Convert units on input data    */
  return (errcode);
} /*  End of getdata  */

void setdefaults(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: assigns default values to global variables
**----------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;
  report_options_t *rep = &pr->report;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;
  out_file_t *out = &pr->out_files;

  strncpy(pr->Title[0], "", MAXMSG);
  strncpy(pr->Title[1], "", MAXMSG);
  strncpy(pr->Title[2], "", MAXMSG);
  strncpy(out->TmpDir, "", MAXFNAME);   
  strncpy(out->TmpFname, "", MAXFNAME); 
  strncpy(out->HydFname, "", MAXFNAME);
  strncpy(pr->MapFname, "", MAXFNAME);
  strncpy(qu->ChemName, t_CHEMICAL, MAXID);
  strncpy(qu->ChemUnits, u_MGperL, MAXID);
  strncpy(par->DefPatID, DEFPATID, MAXID);
  out->Hydflag = SCRATCH;  /* No external hydraulics file    */
  qu->Qualflag = NONE;     /* No quality simulation          */
  hyd->Formflag = HW;      /* Use Hazen-Williams formula     */
  par->Unitsflag = US;     /* US unit system                 */
  par->Flowflag = GPM;     /* Flow units are gpm             */
  par->Pressflag = PSI;    /* Pressure units are psi         */
  rep->Tstatflag = SERIES; /* Generate time series output    */
  pr->Warnflag = FALSE;    /* Warning flag is off            */
  hyd->Htol = HTOL;        /* Default head tolerance         */
  hyd->Qtol = QTOL;        /* Default flow tolerance         */
  hyd->Hacc = HACC;        /* Default hydraulic accuracy     */

  hyd->FlowChangeLimit = 0.0; /* Default flow change limit   */
  hyd->HeadErrorLimit = 0.0;  /* Default head error limit    */

  qu->Ctol = MISSING;      /* No pre-set quality tolerance   */
  hyd->MaxIter = MAXITER;  /* Default max. hydraulic trials  */
  hyd->ExtraIter = -1;     /* Stop if network unbalanced     */
  time->Dur = 0;           /* 0 sec duration (steady state)  */
  time->Tstart = 0;        /* Starting time of day           */
  time->Pstart = 0;        /* Starting pattern period        */
  time->Hstep = 3600;      /* 1 hr hydraulic time step       */
  qu->Qstep = 0;           /* No pre-set quality time step   */
  time->Pstep = 3600;      /* 1 hr time pattern period       */
  time->Rstep = 3600;      /* 1 hr reporting period          */
  time->Rulestep = 0;      /* No pre-set rule time step      */
  time->Rstart = 0;        /* Start reporting at time 0      */
  qu->TraceNode = 0;       /* No source tracing              */
  qu->BulkOrder = 1.0;     /* 1st-order bulk reaction rate   */
  qu->WallOrder = 1.0;     /* 1st-order wall reaction rate   */
  qu->TankOrder = 1.0;     /* 1st-order tank reaction rate   */
  qu->Kbulk = 0.0;         /* No global bulk reaction        */
  qu->Kwall = 0.0;         /* No global wall reaction        */
  qu->Climit = 0.0;        /* No limiting potential quality  */
  qu->Diffus = MISSING;    /* Temporary diffusivity          */
  qu->Rfactor = 0.0;       /* No roughness-reaction factor   */
  hyd->Viscos = MISSING;     /* Temporary viscosity            */
  hyd->SpGrav = SPGRAV;      /* Default specific gravity       */
  hyd->DefPat = 0;           /* Default demand pattern index   */
  hyd->Epat = 0;             /* No energy price pattern        */
  hyd->Ecost = 0.0;          /* Zero unit energy cost          */
  hyd->Dcost = 0.0;          /* Zero energy demand charge      */
  hyd->Epump = EPUMP;        /* Default pump efficiency        */
  hyd->Emax = 0.0;           /* Zero peak energy usage         */
  hyd->Qexp = 2.0;           /* Flow exponent for emitters     */
  hyd->Dmult = 1.0;          /* Demand multiplier              */
  hyd->RQtol = RQTOL;        /* Default hydraulics parameters  */
  hyd->CheckFreq = CHECKFREQ;
  hyd->MaxCheck = MAXCHECK;
  hyd->DampLimit = DAMPLIMIT; 
} /*  End of setdefaults  */

void initreport(report_options_t *r)
/*
**----------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes reporting options
**----------------------------------------------------------------------
*/
{
  int i;
  strncpy(r->Rpt2Fname, "", MAXFNAME);
  r->PageSize = PAGESIZE;      /* Default page size for report   */
  r->Summaryflag = TRUE;       /* Write summary report           */
  r->Messageflag = TRUE;       /* Report error/warning messages  */
  r->Statflag = FALSE;         /* No hydraulic status reports    */
  r->Energyflag = FALSE;       /* No energy usage report         */
  r->Nodeflag = 0;             /* No reporting on nodes          */
  r->Linkflag = 0;             /* No reporting on links          */
  for (i = 0; i < MAXVAR; i++) /* For each reporting variable:   */
  {
    strncpy(r->Field[i].Name, Fldname[i], MAXID);
    r->Field[i].Enabled = FALSE; /* Not included in report  */
    r->Field[i].Precision = 2;   /* 2 decimal precision     */

    /*** Updated 6/24/02 ***/
    r->Field[i].RptLim[LOW] = SQR(BIG); /* No reporting limits   */
    r->Field[i].RptLim[HI] = -SQR(BIG);
  }
  r->Field[FRICTION].Precision = 3;
  for (i = DEMAND; i <= QUALITY; i++) {
    r->Field[i].Enabled = TRUE;
  }
  for (i = FLOW; i <= HEADLOSS; i++) {
    r->Field[i].Enabled = TRUE;
  }
}

void adjustdata(EN_Project *pr)
/*
**----------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: adjusts data after input file has been processed
**----------------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;
  parser_data_t *par = &pr->parser;
  report_options_t *rep = &pr->report;

  int i;
  double ucf;     /* Unit conversion factor */
  Pdemand demand; /* Pointer to demand record */

  /* Use 1 hr pattern & report time step if none specified */
  if (time->Pstep <= 0)
    time->Pstep = 3600;
  if (time->Rstep == 0)
    time->Rstep = time->Pstep;

  /* Hydraulic time step cannot be greater than pattern or report time step */
  if (time->Hstep <= 0)
    time->Hstep = 3600;
  if (time->Hstep > time->Pstep)
    time->Hstep = time->Pstep;
  if (time->Hstep > time->Rstep)
    time->Hstep = time->Rstep;

  /* Report start time cannot be greater than simulation duration */
  if (time->Rstart > time->Dur)
    time->Rstart = 0;

  /* No water quality analysis for steady state run */
  if (time->Dur == 0)
    qu->Qualflag = NONE;

  /* If no quality timestep, then make it 1/10 of hydraulic timestep */
  if (qu->Qstep == 0)
    qu->Qstep = time->Hstep / 10;

  /* If no rule time step then make it 1/10 of hydraulic time step; */
  /* Rule time step cannot be greater than hydraulic time step */
  if (time->Rulestep == 0)
    time->Rulestep = time->Hstep / 10;
  time->Rulestep = MIN(time->Rulestep, time->Hstep);

  /* Quality timestep cannot exceed hydraulic timestep */
  qu->Qstep = MIN(qu->Qstep, time->Hstep);

  /* If no quality tolerance, then use default values */
  if (qu->Ctol == MISSING) {
    if (qu->Qualflag == AGE)
      qu->Ctol = AGETOL;
    else
      qu->Ctol = CHEMTOL;
  }

  /* Determine unit system based on flow units */
  switch (par->Flowflag) {
  case LPS: /* Liters/sec */
  case LPM: /* Liters/min */
  case MLD: /* megaliters/day  */
  case CMH: /* cubic meters/hr */
  case CMD: /* cubic meters/day */
    par->Unitsflag = SI;
    break;
  default:
    par->Unitsflag = US;
  }

  /* Revise pressure units depending on flow units */
  if (par->Unitsflag != SI)
    par->Pressflag = PSI;
  else if (par->Pressflag == PSI)
    par->Pressflag = METERS;

  /* Store value of viscosity & diffusivity */
  ucf = 1.0;
  if (par->Unitsflag == SI)
    ucf = SQR(MperFT);

  if (hyd->Viscos == MISSING) /* No value supplied */
    hyd->Viscos = VISCOS;
  else if (hyd->Viscos > 1.e-3) /* Multiplier supplied */
    hyd->Viscos = hyd->Viscos * VISCOS;
  else /* Actual value supplied */
    hyd->Viscos = hyd->Viscos / ucf;

  if (qu->Diffus == MISSING)
    qu->Diffus = DIFFUS;
  else if (qu->Diffus > 1.e-4)
    qu->Diffus = qu->Diffus * DIFFUS;
  else
    qu->Diffus = qu->Diffus / ucf;

  /*
    Set exponent in head loss equation and adjust flow-resistance tolerance.
  */
  if (hyd->Formflag == HW)
    hyd->Hexp = 1.852;
  else
    hyd->Hexp = 2.0;

  /*** Updated 9/7/00 ***/
  /*** No adjustment made to flow-resistance tolerance ***/
  /*hyd->RQtol = hyd->RQtol/Hexp;*/

  /* See if default reaction coeffs. apply */
  for (i = 1; i <= net->Nlinks; i++) {
    Slink *link = &net->Link[i];
    if (link->Type > EN_PIPE)
      continue;
    if (link->Kb == MISSING)
      link->Kb = qu->Kbulk;      /* Bulk coeff. */
    if (link->Kw == MISSING) /* Wall coeff. */
    {
      /* Rfactor is the pipe roughness correlation factor */
      if (qu->Rfactor == 0.0)
        link->Kw = qu->Kwall;
      else if ((link->Kc > 0.0) && (link->Diam > 0.0)) {
        if (hyd->Formflag == HW)
          link->Kw = qu->Rfactor / link->Kc;
        if (hyd->Formflag == DW)
          link->Kw = qu->Rfactor / ABS(log(link->Kc / link->Diam));
        if (hyd->Formflag == CM)
          link->Kw = qu->Rfactor * link->Kc;
      } else
        link->Kw = 0.0;
    }
  }
  for (i = 1; i <= net->Ntanks; i++) {
    Stank *tank = &net->Tank[i];
    if (tank->Kb == MISSING) {
      tank->Kb = qu->Kbulk;
    }
  }
  /* Use default pattern if none assigned to a demand */
  for (i = 1; i <= net->Nnodes; i++) {
    Snode *node = &net->Node[i];
    for (demand = node->D; demand != NULL; demand = demand->next) {
      if (demand->Pat == 0) {
        demand->Pat = hyd->DefPat;
      }
    }
  }

  /* Remove QUALITY as a reporting variable if no WQ analysis */
  if (qu->Qualflag == NONE)
    rep->Field[QUALITY].Enabled = FALSE;

} /*  End of adjustdata  */

int inittanks(EN_Project *pr)
/*
**---------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: initializes volumes in non-cylindrical tanks
**---------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;

  int i, j, n = 0;
  double a;
  int errcode = 0, levelerr;

  for (j = 1; j <= net->Ntanks; j++) {
    Stank *tank = &net->Tank[j];
    /* Skip reservoirs */
    if (tank->A == 0.0)
      continue;

    /* Check for valid lower/upper tank levels */
    levelerr = 0;
    if (tank->H0 > tank->Hmax || tank->Hmin > tank->Hmax ||
        tank->H0 < tank->Hmin) {
      levelerr = 1;
    }

    /* Check that tank heights are within volume curve */
    i = tank->Vcurve;
    if (i > 0) {
      Scurve *curve = &net->Curve[i];
      n = curve->Npts - 1;
      if (tank->Hmin < curve->X[0] || tank->Hmax > curve->X[n])
        levelerr = 1;
    }

    /* Report error in levels if found */
    if (levelerr) {
      char errMsg[MAXMSG+1];
      EN_geterror(225, errMsg, MAXMSG);
      sprintf(pr->Msg, "%s node: %s", errMsg, net->Node[tank->Node].ID);
      writeline(pr, pr->Msg);
      errcode = 200;
    }

    /* Else if tank has a volume curve, */
    else if (i > 0) {
      Scurve *curve = &net->Curve[i];
      /* Find min., max., and initial volumes from curve */
      tank->Vmin = interp(curve->Npts, curve->X, curve->Y, tank->Hmin);
      tank->Vmax = interp(curve->Npts, curve->X, curve->Y, tank->Hmax);
      tank->V0 = interp(curve->Npts, curve->X, curve->Y, tank->H0);

      /* Find a "nominal" diameter for tank */
      a = (curve->Y[n] - curve->Y[0]) / (curve->X[n] - curve->X[0]);
      tank->A = sqrt(4.0 * a / PI);
    }
  }
  return (errcode);
} /* End of inittanks */

void initunits(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: determines unit conversion factors
**--------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;
  report_options_t *rep = &pr->report;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  double dcf, /* distance conversion factor      */
      ccf,    /* concentration conversion factor */
      qcf,    /* flow conversion factor          */
      hcf,    /* head conversion factor          */
      pcf,    /* pressure conversion factor      */
      wcf;    /* energy conversion factor        */

  if (par->Unitsflag == SI) /* SI units */
  {
    strcpy(rep->Field[DEMAND].Units, RptFlowUnitsTxt[par->Flowflag]);
    strcpy(rep->Field[ELEV].Units, u_METERS);
    strcpy(rep->Field[HEAD].Units, u_METERS);
    if (par->Pressflag == METERS)
      strcpy(rep->Field[PRESSURE].Units, u_METERS);
    else
      strcpy(rep->Field[PRESSURE].Units, u_KPA);
    strcpy(rep->Field[LENGTH].Units, u_METERS);
    strcpy(rep->Field[DIAM].Units, u_MMETERS);
    strcpy(rep->Field[FLOW].Units, RptFlowUnitsTxt[par->Flowflag]);
    strcpy(rep->Field[VELOCITY].Units, u_MperSEC);
    strcpy(rep->Field[HEADLOSS].Units, u_per1000M);
    strcpy(rep->Field[FRICTION].Units, "");
    strcpy(rep->Field[POWER].Units, u_KW);
    dcf = 1000.0 * MperFT;
    qcf = LPSperCFS;
    if (par->Flowflag == LPM)
      qcf = LPMperCFS;
    if (par->Flowflag == MLD)
      qcf = MLDperCFS;
    if (par->Flowflag == CMH)
      qcf = CMHperCFS;
    if (par->Flowflag == CMD)
      qcf = CMDperCFS;
    hcf = MperFT;
    if (par->Pressflag == METERS)
      pcf = MperFT * hyd->SpGrav;
    else
      pcf = KPAperPSI * PSIperFT * hyd->SpGrav;
    wcf = KWperHP;
  } else /* US units */
  {
    strcpy(rep->Field[DEMAND].Units, RptFlowUnitsTxt[par->Flowflag]);
    strcpy(rep->Field[ELEV].Units, u_FEET);
    strcpy(rep->Field[HEAD].Units, u_FEET);
    strcpy(rep->Field[PRESSURE].Units, u_PSI);
    strcpy(rep->Field[LENGTH].Units, u_FEET);
    strcpy(rep->Field[DIAM].Units, u_INCHES);
    strcpy(rep->Field[FLOW].Units, RptFlowUnitsTxt[par->Flowflag]);
    strcpy(rep->Field[VELOCITY].Units, u_FTperSEC);
    strcpy(rep->Field[HEADLOSS].Units, u_per1000FT);
    strcpy(rep->Field[FRICTION].Units, "");
    strcpy(rep->Field[POWER].Units, u_HP);
    dcf = 12.0;
    qcf = 1.0;
    if (par->Flowflag == GPM)
      qcf = GPMperCFS;
    if (par->Flowflag == MGD)
      qcf = MGDperCFS;
    if (par->Flowflag == IMGD)
      qcf = IMGDperCFS;
    if (par->Flowflag == AFD)
      qcf = AFDperCFS;
    hcf = 1.0;
    pcf = PSIperFT * hyd->SpGrav;
    wcf = 1.0;
  }
  strcpy(rep->Field[QUALITY].Units, "");
  ccf = 1.0;
  if (qu->Qualflag == CHEM) {
    ccf = 1.0 / LperFT3;
    strncpy(rep->Field[QUALITY].Units, qu->ChemUnits, MAXID);
    strncpy(rep->Field[REACTRATE].Units, qu->ChemUnits, MAXID);
    strcat(rep->Field[REACTRATE].Units, t_PERDAY);
  } else if (qu->Qualflag == AGE)
    strcpy(rep->Field[QUALITY].Units, u_HOURS);
  else if (qu->Qualflag == TRACE)
    strcpy(rep->Field[QUALITY].Units, u_PERCENT);
  pr->Ucf[DEMAND] = qcf;
  pr->Ucf[ELEV] = hcf;
  pr->Ucf[HEAD] = hcf;
  pr->Ucf[PRESSURE] = pcf;
  pr->Ucf[QUALITY] = ccf;
  pr->Ucf[LENGTH] = hcf;
  pr->Ucf[DIAM] = dcf;
  pr->Ucf[FLOW] = qcf;
  pr->Ucf[VELOCITY] = hcf;
  pr->Ucf[HEADLOSS] = hcf;
  pr->Ucf[LINKQUAL] = ccf;
  pr->Ucf[REACTRATE] = ccf;
  pr->Ucf[FRICTION] = 1.0;
  pr->Ucf[POWER] = wcf;
  pr->Ucf[VOLUME] = hcf * hcf * hcf;
  if (time->Hstep < 1800)             /* Report time in mins.    */
  {                                   /* if hydraulic time step  */
    pr->Ucf[TIME] = 1.0 / 60.0;       /* is less than 1/2 hour.  */
    strcpy(rep->Field[TIME].Units, u_MINUTES);
  } else {
    pr->Ucf[TIME] = 1.0 / 3600.0;
    strcpy(rep->Field[TIME].Units, u_HOURS);
  }

} /*  End of initunits  */

void convertunits(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: converts units of input data
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;

  int i, j, k;
  double ucf;     /* Unit conversion factor */
  Pdemand demand; /* Pointer to demand record */
  Snode *node;
  Stank *tank;
  Slink *link;
  Spump *pump;
  Scontrol *control;
  
  /* Convert nodal elevations & initial WQ */
  /* (WQ source units are converted in QUALITY.C */
  for (i = 1; i <= net->Nnodes; i++) {
    node = &net->Node[i];
    node->El /= pr->Ucf[ELEV];
    node->C0 /= pr->Ucf[QUALITY];
  }

  /* Convert demands */
  for (i = 1; i <= net->Njuncs; i++) {
    node = &net->Node[i];
    for (demand = node->D; demand != NULL; demand = demand->next) {
      demand->Base /= pr->Ucf[DEMAND];
    }
  }

  /* Convert emitter discharge coeffs. to head loss coeff. */
  ucf = pow(pr->Ucf[FLOW], hyd->Qexp) / pr->Ucf[PRESSURE];
  for (i = 1; i <= net->Njuncs; i++) {
    node = &net->Node[i];
    if (node->Ke > 0.0) {
      node->Ke = ucf / pow(node->Ke, hyd->Qexp);
    }
  }
  /* Initialize tank variables (convert tank levels to elevations) */
  for (j = 1; j <= net->Ntanks; j++) {
    tank = &net->Tank[j];
    i = tank->Node;
    node = &net->Node[i];
    tank->H0 = node->El + tank->H0 / pr->Ucf[ELEV];
    tank->Hmin = node->El + tank->Hmin / pr->Ucf[ELEV];
    tank->Hmax = node->El + tank->Hmax / pr->Ucf[ELEV];
    tank->A = PI * SQR(tank->A / pr->Ucf[ELEV]) / 4.0;
    tank->V0 /= pr->Ucf[VOLUME];
    tank->Vmin /= pr->Ucf[VOLUME];
    tank->Vmax /= pr->Ucf[VOLUME];
    tank->Kb /= SECperDAY;
    tank->V = tank->V0;
    tank->C = node->C0;
    tank->V1max *= tank->Vmax;
  }

  /* Convert hydraulic convergence criteria */
  hyd->FlowChangeLimit /= pr->Ucf[FLOW];
  hyd->HeadErrorLimit  /= pr->Ucf[HEAD];

  /* Convert WQ option concentration units */
  qu->Climit /= pr->Ucf[QUALITY];
  qu->Ctol /= pr->Ucf[QUALITY];

  /* Convert global reaction coeffs. */
  qu->Kbulk /= SECperDAY;
  qu->Kwall /= SECperDAY;

  /* Convert units of link parameters */
  for (k = 1; k <= net->Nlinks; k++) {
    link = &net->Link[k];
    if (link->Type <= EN_PIPE) {
      /* Convert pipe parameter units:                         */
      /*    - for Darcy-Weisbach formula, convert roughness    */
      /*      from millifeet (or mm) to ft (or m)              */
      /*    - for US units, convert diameter from inches to ft */
      if (hyd->Formflag == DW)
        link->Kc /= (1000.0 * pr->Ucf[ELEV]);
      link->Diam /= pr->Ucf[DIAM];
      link->Len /= pr->Ucf[LENGTH];

      /* Convert minor loss coeff. from V^2/2g basis to Q^2 basis */
      link->Km = 0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam);

      /* Convert units on reaction coeffs. */
      link->Kb /= SECperDAY;
      link->Kw /= SECperDAY;
    }

    else if (link->Type == EN_PUMP) {
      /* Convert units for pump curve parameters */
      i = findpump(net, k);
      pump = &net->Pump[i];
      if (pump->Ptype == CONST_HP) {
        /* For constant hp pump, convert kw to hp */
        if (par->Unitsflag == SI)
          pump->R /= pr->Ucf[POWER];
      } else {
        /* For power curve pumps, convert     */
        /* shutoff head and flow coefficient  */
        if (pump->Ptype == POWER_FUNC) {
          pump->H0 /= pr->Ucf[HEAD];
          pump->R *= (pow(pr->Ucf[FLOW], pump->N) / pr->Ucf[HEAD]);
        }
        /* Convert flow range & max. head units */
        pump->Q0 /= pr->Ucf[FLOW];
        pump->Qmax /= pr->Ucf[FLOW];
        pump->Hmax /= pr->Ucf[HEAD];
      }
    }

    else {
      /* For flow control valves, convert flow setting    */
      /* while for other valves convert pressure setting  */
      link->Diam /= pr->Ucf[DIAM];
      link->Km = 0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam);
      if (link->Kc != MISSING)
        switch (link->Type) {
          case EN_FCV:
            link->Kc /= pr->Ucf[FLOW];
            break;
          case EN_PRV:
          case EN_PSV:
          case EN_PBV:
            link->Kc /= pr->Ucf[PRESSURE];
            break;
          default:
            break;
        }
      
    }

//////  Moved to inithyd() in hydraul.c  ///////
    /* Compute flow resistances */
    //resistance(pr, k);
  }

  /* Convert units on control settings */
  for (i = 1; i <= net->Ncontrols; i++) {
    control = &net->Control[i];
    if ((k = control->Link) == 0) {
      continue;
    }
    link = &net->Link[k];
    if ((j = control->Node) > 0) {
      node = &net->Node[j];
      if (j > net->Njuncs) {
        /* j > Njuncs, then control is based on tank level */
        control->Grade = node->El + control->Grade / pr->Ucf[ELEV];
      } else {
        /* otherwise control is based on nodal pressure    */
        control->Grade = node->El + control->Grade / pr->Ucf[PRESSURE];
      }
    }

    /* Convert units on valve settings */
    if (control->Setting != MISSING) {
      switch (link->Type) {
        case EN_PRV:
        case EN_PSV:
        case EN_PBV:
          control->Setting /= pr->Ucf[PRESSURE];
          break;
        case EN_FCV:
          control->Setting /= pr->Ucf[FLOW];
        default:
          break;
      }
    }
  }
} /*  End of convertunits  */

/************************ END OF INPUT1.C ************************/
