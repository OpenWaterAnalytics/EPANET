/*
*******************************************************************************

QUALITY.C -- Water Quality Simulator for EPANET Program

VERSION:    2.00
DATE:       5/29/00
            9/7/00
            10/25/00
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL

  This module contains the network water quality simulator.

  For each time period, hydraulic results are read in from the
  binary file HydFile, hydraulic and water quality results are
  written to the binary output file OutFile (if the current period
  is a reporting period), and the water quality is transported
  and reacted over the duration of the time period.

  The entry points for this module are:
    openqual()   -- called from ENopenQ() in EPANET.C
    initqual()   -- called from ENinitQ() in EPANET.C
    runqual()    -- called from ENrunQ() in EPANET.C
    nextqual()   -- called from ENnextQ() in EPANET.C
    stepqual()   -- called from ENstepQ() in EPANET.C
    closequal()  -- called from ENcloseQ() in EPANET.C

  Calls are made to:
    AllocInit()
    Alloc()
    AllocFree()
  in MEMPOOL.C to utilize a memory pool to prevent excessive malloc'ing
  when constantly creating and destroying pipe sub-segments during
  the water quality transport calculations.

  Calls are also made to:
    readhyd()
    readhydstep()
    savenetdata()
    saveoutput()
    savefinaloutput()
  in OUTPUT.C to retrieve hydraulic results and save all results.

*******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "hash.h"
#include "text.h"
#include "types.h"
#include "epanet2.h"
#include "funcs.h"
#include <math.h>
#define EXTERN extern
#include "mempool.h"
#include "vars.h"

/*
** Macros to identify upstream & downstream nodes of a link
** under the current flow and to compute link volume
*/
#define UP_NODE(x)                                                             \
  ((qu->FlowDir[(x)] == POSITIVE) ? net->Link[(x)].N1 : net->Link[(x)].N2)
#define DOWN_NODE(x)                                                           \
  ((qu->FlowDir[(x)] == POSITIVE) ? net->Link[(x)].N2 : net->Link[(x)].N1)
#define LINKVOL(k) (0.785398 * net->Link[(k)].Len * SQR(net->Link[(k)].Diam))

int openqual(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: opens WQ solver system
**--------------------------------------------------------------
*/
{
  int errcode = 0;
  int n;

  quality_t *qu = &pr->quality;
  EN_Network *net = &pr->network;

  /* Allocate memory pool for WQ segments */
  qu->OutOfMemory = FALSE;
  qu->SegPool = AllocInit(); 
  if (qu->SegPool == NULL) {
    errcode = 101; 
  }

  /* Allocate scratch array & reaction rate array*/
  qu->TempQual = (double *)calloc(MAX((net->Nnodes + 1), (net->Nlinks + 1)),
                                  sizeof(double));
  qu->PipeRateCoeff = (double *)calloc((net->Nlinks + 1), sizeof(double));
  ERRCODE(MEMCHECK(qu->TempQual));
  ERRCODE(MEMCHECK(qu->PipeRateCoeff));

  /* Allocate memory for WQ solver */
  n = net->Nlinks + net->Ntanks + 1;
  qu->FirstSeg = (Pseg *)calloc(n, sizeof(Pseg));
  qu->LastSeg = (Pseg *)calloc(n, sizeof(Pseg));
  qu->FlowDir = (FlowDirection *)calloc(n, sizeof(FlowDirection));
  n = net->Nnodes + 1;
  qu->VolIn = (double *)calloc(n, sizeof(double));
  qu->MassIn = (double *)calloc(n, sizeof(double));
  ERRCODE(MEMCHECK(qu->FirstSeg));
  ERRCODE(MEMCHECK(qu->LastSeg));
  ERRCODE(MEMCHECK(qu->FlowDir));
  ERRCODE(MEMCHECK(qu->VolIn));
  ERRCODE(MEMCHECK(qu->MassIn));
  return (errcode);
}

/* Local function to compute unit conversion factor for bulk reaction rates */
double getucf(double order) {
  if (order < 0.0) {
    order = 0.0;
  }
  if (order == 1.0) {
    return (1.0);
  } else {
    return (1. / pow(LperFT3, (order - 1.0)));
  }
}

void initqual(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: re-initializes WQ solver system
**--------------------------------------------------------------
*/
{
  int i;
  quality_t *qu = &pr->quality;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  time_options_t *time = &pr->time_options;

  /* Initialize quality, tank volumes, & source mass flows */
  for (i = 1; i <= net->Nnodes; i++) {
    qu->NodeQual[i] = net->Node[i].C0;
  }
  for (i = 1; i <= net->Ntanks; i++) {
    net->Tank[i].C = net->Node[net->Tank[i].Node].C0;
  }
  for (i = 1; i <= net->Ntanks; i++) {
    net->Tank[i].V = net->Tank[i].V0;
  }
  for (i = 1; i <= net->Nnodes; i++) {
    if (net->Node[i].S != NULL)
      net->Node[i].S->Smass = 0.0;
  }

  qu->QTankVolumes =
      calloc(net->Ntanks,
             sizeof(double)); // keep track of previous step's tank volumes.
  qu->QLinkFlow = calloc(
      net->Nlinks, sizeof(double)); // keep track of previous step's link flows.

  /* Set WQ parameters */
  qu->Bucf = 1.0;
  qu->Tucf = 1.0;
  qu->Reactflag = 0;
  if (qu->Qualflag != NONE) {
    /* Initialize WQ at trace node (if applicable) */
    if (qu->Qualflag == TRACE) {
      qu->NodeQual[qu->TraceNode] = 100.0;
    }

    /* Compute Schmidt number */
    if (qu->Diffus > 0.0)
      qu->Sc = hyd->Viscos / qu->Diffus;
    else
      qu->Sc = 0.0;

    /* Compute unit conversion factor for bulk react. coeff. */
    qu->Bucf = getucf(qu->BulkOrder);
    qu->Tucf = getucf(qu->TankOrder);

    /* Check if modeling a reactive substance */
    qu->Reactflag = setReactflag(pr);

    /* Reset memory pool */
    qu->FreeSeg = NULL;
    AllocSetPool(qu->SegPool); 
    AllocReset();              
  }

  /* Initialize avg. reaction rates */
  qu->Wbulk = 0.0;
  qu->Wwall = 0.0;
  qu->Wtank = 0.0;
  qu->Wsource = 0.0;

  /* Re-position hydraulics file */
  if (!hyd->OpenHflag) {
    fseek(pr->out_files.HydFile, pr->out_files.HydOffset, SEEK_SET);
  }

  /* Set elapsed times to zero */
  time->Htime = 0;
  qu->Qtime = 0;
  pr->time_options.Rtime = pr->time_options.Rstart;
  pr->report.Nperiods = 0;

  initsegs(pr);
}

int runqual(EN_Project *pr, long *t)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  t = pointer to current simulation time (sec)
**   Returns: error code
**   Purpose: retrieves hydraulics for next hydraulic time step
**            (at time *t) and saves current results to file
**--------------------------------------------------------------
*/
{
  long hydtime; /* Hydraulic solution time */
  long hydstep; /* Hydraulic time step     */
  int errcode = 0;
  int i;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  /* Update reported simulation time */
  *t = qu->Qtime;

  /* Read hydraulic solution from hydraulics file */
  if (qu->Qtime == time->Htime) {
    errcode = gethyd(pr, &hydtime, &hydstep);
    if (!hyd->OpenHflag) { // test for sequential vs stepwise
      // sequential
      time->Htime = hydtime + hydstep;
    } else {
      // stepwise calculation - hydraulic results are already in memory
      for (i = 1; i <= net->Ntanks; ++i) {
        qu->QTankVolumes[i - 1] = net->Tank[i].V;
      }

      for (i = 1; i <= net->Nlinks; ++i) {
        if (hyd->LinkStatus[i] <= CLOSED) {
          qu->QLinkFlow[i - 1] = hyd->LinkFlows[i];
        }
      }
    }
  } else {
    // stepwise calculation
    for (i = 1; i <= net->Ntanks; ++i) {
      qu->QTankVolumes[i - 1] = net->Tank[i].V;
    }

    for (i = 1; i <= net->Nlinks; ++i) {
      if (hyd->LinkStatus[i] <= CLOSED) {
        qu->QLinkFlow[i - 1] = hyd->LinkFlows[i];
      }
    }
  }

  return (errcode);
}

int nextqual(EN_Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  tstep = pointer to time step (sec)
**   Returns: error code
**   Purpose: updates WQ conditions until next hydraulic
**            solution occurs (after *tstep secs.)
**--------------------------------------------------------------
*/
{
  long hydstep; /* Hydraulic solution time step */
  int errcode = 0;
  double *tankVolumes;
  int i;
  EN_Network *net;
  hydraulics_t *hyd;
  quality_t *qu;
  time_options_t *time;
  save_options_t *sav;
  
  /* Determine time step */
  *tstep = 0;

  net = &pr->network;
  hyd = &pr->hydraulics;
  qu = &pr->quality;
  time = &pr->time_options;
  sav = &pr->save_options;
  
  // hydstep = time->Htime - qu->Qtime;

  if (time->Htime <= time->Dur) {
    hydstep = time->Htime - qu->Qtime;
  } else {
    hydstep = 0;
  }

  // if we're operating in stepwise mode, capture the tank levels so we can
  // restore them later.
  if (hyd->OpenHflag) {
    tankVolumes = calloc(net->Ntanks, sizeof(double));
    for (i = 1; i <= net->Ntanks; ++i) {
      if (net->Tank[i].A != 0) { // skip reservoirs
        tankVolumes[i - 1] = net->Tank[i].V;
      }
    }

    // restore the previous step's tank volumes
    for (i = 1; i <= net->Ntanks; i++) {
      if (net->Tank[i].A != 0) { // skip reservoirs again
        int n = net->Tank[i].Node;
        net->Tank[i].V = qu->QTankVolumes[i - 1];
        hyd->NodeHead[n] = tankgrade(pr, i, net->Tank[i].V);
      }
    }

    // restore the previous step's pipe link flows
    for (i = 1; i <= net->Nlinks; i++) {
      if (hyd->LinkStatus[i] <= CLOSED) {
        hyd->LinkFlows[i] = 0.0;
      }
    }
  }

  /* Perform water quality routing over this time step */
  if (qu->Qualflag != NONE && hydstep > 0)
    transport(pr,hydstep);

  /* Update current time */
  if (qu->OutOfMemory)
    errcode = 101;
  if (!errcode)
    *tstep = hydstep;
  qu->Qtime += hydstep;

  /* Save final output if no more time steps */
  if (!errcode && sav->Saveflag && *tstep == 0)
    errcode = savefinaloutput(pr);

  // restore tank levels to post-runH state, if needed.
  if (hyd->OpenHflag) {
    for (i = 1; i <= net->Ntanks; i++) {
      if (net->Tank[i].A != 0) { // skip reservoirs again
        int n = net->Tank[i].Node;
        net->Tank[i].V = tankVolumes[i - 1];
        hyd->NodeHead[n] = tankgrade(pr, i, net->Tank[i].V);
      }
    }

    for (i = 1; i <= net->Nlinks; ++i) {
      if (hyd->LinkStatus[i] <= CLOSED) {
        hyd->LinkFlows[i] = qu->QLinkFlow[i - 1];
      }
    }

    free(tankVolumes);
  }

  return (errcode);
}

int stepqual(EN_Project *pr, long *tleft)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  tleft = pointer to time left in simulation
**   Returns: error code
**   Purpose: updates WQ conditions over a single WQ time step
**--------------------------------------------------------------
*/
{
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;
  save_options_t *sav = &pr->save_options;

  long dt, hstep, t, tstep;
  int errcode = 0;
  tstep = qu->Qstep;
  do {
    dt = tstep;
    hstep = time->Htime - qu->Qtime;
    if (hstep < dt) {
      dt = hstep;
      if (qu->Qualflag != NONE) {
        transport(pr,dt);
      }
      qu->Qtime += dt;
      errcode = runqual(pr,&t);
      qu->Qtime = t;
    } else {
      if (qu->Qualflag != NONE)
        transport(pr,dt);
      qu->Qtime += dt;
    }
    tstep -= dt;
    if (qu->OutOfMemory) {
      errcode = 101;
    }
  } while (!errcode && tstep > 0);
  *tleft = time->Dur - qu->Qtime;

  if (!errcode && sav->Saveflag && *tleft == 0) {
    errcode = savefinaloutput(pr);
  }
  return (errcode);
}

int closequal(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: closes WQ solver system
**--------------------------------------------------------------
*/
{
  quality_t *qu = &pr->quality;
  int errcode = 0;

  /* Free memory pool */
  if (qu->SegPool)             
  {                            
    AllocSetPool(qu->SegPool); 
    AllocFreePool();           
  }                            

  free(qu->FirstSeg);
  free(qu->LastSeg);
  free(qu->FlowDir);
  free(qu->VolIn);
  free(qu->MassIn);
  free(qu->PipeRateCoeff);
  free(qu->TempQual);
  free(qu->QTankVolumes);
  free(qu->QLinkFlow);
  return (errcode);
}

int gethyd(EN_Project *pr, long *hydtime, long *hydstep)
/*
**-----------------------------------------------------------
**   Input:   none
**   Output:  hydtime = pointer to hydraulic solution time
**            hydstep = pointer to hydraulic time step
**   Returns: error code
**   Purpose: retrieves hydraulic solution and hydraulic
**            time step for next hydraulic event
**
**   NOTE: when this function is called, WQ results have
**         already been updated to the point in time when
**         the next hydraulic event occurs.
**-----------------------------------------------------------
*/
{
  int errcode = 0;
  
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;
  save_options_t *sav = &pr->save_options;

  // if hydraulics are not open, then we're operating in sequential mode.
  // else hydraulics are open, so use the hydraulic results in memory rather
  // than reading from the temp file.
  if (!hyd->OpenHflag) {
    /* Read hydraulic results from file */
    if (!readhyd(pr,hydtime)) {
      return (307);
    }
    if (!readhydstep(pr->out_files.HydFile, hydstep)) {
      return (307);
    }
    time->Htime = *hydtime;
  }

  /* Save current results to output file */
  if (time->Htime >= time->Rtime) {
    if (sav->Saveflag) {
      errcode = saveoutput(pr);
      rep->Nperiods++;
    }
    time->Rtime += time->Rstep;
  }

  /* If simulating WQ: */
  if (qu->Qualflag != NONE && qu->Qtime < time->Dur) {

    /* Compute reaction rate coeffs. */
    if (qu->Reactflag && qu->Qualflag != AGE) {
      ratecoeffs(pr);
    }

    /* Initialize pipe segments (at time 0) or  */
    /* else re-orient segments if flow reverses.*/
    // if (qu->Qtime == 0)
    //  initsegs();
    // else
    // if hydraulics are open, or if we're in sequential mode (where qtime can
    // increase)
    if (hyd->OpenHflag || qu->Qtime != 0) {
      reorientsegs(pr);
    }
  }
  return (errcode);
}

char setReactflag(EN_Project *pr)
/*
**-----------------------------------------------------------
**   Input:   none
**   Output:  returns 1 for reactive WQ constituent, 0 otherwise
**   Purpose: checks if reactive chemical being simulated
**-----------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  
  int i;
  if (qu->Qualflag == TRACE) {
    return (0);
  } else if (qu->Qualflag == AGE) {
    return (1);
  } else {
    for (i = 1; i <= net->Nlinks; i++) {
      if (net->Link[i].Type <= EN_PIPE) {
        if (net->Link[i].Kb != 0.0 || net->Link[i].Kw != 0.0) {
          return (1);
        }
      }
    }
    for (i = 1; i <= net->Ntanks; i++) {
      if (net->Tank[i].Kb != 0.0) {
        return (1);
      }
    }
  }
  return (0);
}

void transport(EN_Project *pr, long tstep)
/*
**--------------------------------------------------------------
**   Input:   tstep = length of current time step
**   Output:  none
**   Purpose: transports constituent mass through pipe network
**            under a period of constant hydraulic conditions.
**--------------------------------------------------------------
*/
{
  long qtime, dt;
  quality_t *qu = &pr->quality;

  /* Repeat until elapsed time equals hydraulic time step */

  AllocSetPool(qu->SegPool); 
  qtime = 0;
  while (!qu->OutOfMemory && qtime < tstep) { /* Qstep is quality time step */
    dt = MIN(qu->Qstep, tstep - qtime);       /* Current time step */
    qtime += dt;                              /* Update elapsed time */
    if (qu->Reactflag) {
      updatesegs(pr, dt); /* Update quality in inner link segs */
    }
    accumulate(pr, dt);  /* Accumulate flow at nodes */
    updatenodes(pr, dt); /* Update nodal quality */
    sourceinput(pr, dt); /* Compute inputs from sources */
    release(pr, dt);     /* Release new nodal flows */
  }
  updatesourcenodes(pr, tstep); /* Update quality at source nodes */
}

void initsegs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: initializes water quality segments
**--------------------------------------------------------------
*/
{
  int j, k;
  double c, v;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  /* Examine each link */
  for (k = 1; k <= net->Nlinks; k++) {

    /* Establish flow direction */
    qu->FlowDir[k] = POSITIVE;
    if (hyd->LinkFlows[k] < 0.) {
      qu->FlowDir[k] = NEGATIVE;
    }

    /* Set segs to zero */
    qu->LastSeg[k] = NULL;
    qu->FirstSeg[k] = NULL;

    /* Find quality of downstream node */
    j = DOWN_NODE(k);
    if (j <= net->Njuncs) {
      c = qu->NodeQual[j];
    } else {
      c = net->Tank[j - net->Njuncs].C;
    }

    /* Fill link with single segment with this quality */
    addseg(pr, k, LINKVOL(k), c);
  }

  /* Initialize segments in tanks that use them */
  for (j = 1; j <= net->Ntanks; j++) {

    /* Skip reservoirs & complete mix tanks */
    if (net->Tank[j].A == 0.0 || net->Tank[j].MixModel == MIX1)
      continue;

    /* Tank segment pointers are stored after those for links */
    k = net->Nlinks + j;
    c = net->Tank[j].C;
    qu->LastSeg[k] = NULL;
    qu->FirstSeg[k] = NULL;

    /* Add 2 segments for 2-compartment model */
    if (net->Tank[j].MixModel == MIX2) {
      v = MAX(0, net->Tank[j].V - net->Tank[j].V1max);
      addseg(pr, k, v, c);
      v = net->Tank[j].V - v;
      addseg(pr, k, v, c);
    }

    /* Add one segment for FIFO & LIFO models */
    else {
      v = net->Tank[j].V;
      addseg(pr, k, v, c);
    }
  }
}

void reorientsegs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: re-orients segments (if flow reverses)
**--------------------------------------------------------------
*/
{
  Pseg seg, nseg, pseg;
  int k;
  FlowDirection newdir;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  /* Examine each link */
  for (k = 1; k <= net->Nlinks; k++) {

    /* Find new flow direction */
    newdir = POSITIVE;
    if (hyd->LinkFlows[k] == 0.0) {
      newdir = qu->FlowDir[k];
    } else if (hyd->LinkFlows[k] < 0.0) {
      newdir = NEGATIVE;
    }

    /* If direction changes, then reverse order of segments */
    /* (first to last) and save new direction */
    if (newdir != qu->FlowDir[k]) {
      seg = qu->FirstSeg[k];
      qu->FirstSeg[k] = qu->LastSeg[k];
      qu->LastSeg[k] = seg;
      pseg = NULL;
      while (seg != NULL) {
        nseg = seg->prev;
        seg->prev = pseg;
        pseg = seg;
        seg = nseg;
      }
      qu->FlowDir[k] = newdir;
    }
  }
}

void updatesegs(EN_Project *pr, long dt)
/*
**-------------------------------------------------------------
**   Input:   t = time from last WQ segment update
**   Output:  none
**   Purpose: reacts material in pipe segments up to time t
**-------------------------------------------------------------
*/
{
  int k;
  Pseg seg;
  double cseg, rsum, vsum;
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;

  /* Examine each link in network */
  for (k = 1; k <= net->Nlinks; k++) {

    /* Skip zero-length links (pumps & valves) */
    rsum = 0.0;
    vsum = 0.0;
    if (net->Link[k].Len == 0.0) {
      continue;
    }

    /* Examine each segment of the link */
    seg = qu->FirstSeg[k];
    while (seg != NULL) {

      /* React segment over time dt */
      cseg = seg->c;
      seg->c = pipereact(pr, k, seg->c, seg->v, dt);

      /* Accumulate volume-weighted reaction rate */
      if (qu->Qualflag == CHEM) {
        rsum += ABS((seg->c - cseg)) * seg->v;
        vsum += seg->v;
      }
      seg = seg->prev;
    }

    /* Normalize volume-weighted reaction rate */
    if (vsum > 0.0) {
      qu->PipeRateCoeff[k] = rsum / vsum / dt * SECperDAY;
    } else
      qu->PipeRateCoeff[k] = 0.0;
  }
}

void removesegs(EN_Project *pr, int k)
/*
**-------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: removes all segments in link k
**-------------------------------------------------------------
*/
{
  Pseg seg;
  quality_t *qu = &pr->quality;

  seg = qu->FirstSeg[k];
  while (seg != NULL) {
    qu->FirstSeg[k] = seg->prev;
    seg->prev = qu->FreeSeg;
    qu->FreeSeg = seg;
    seg = qu->FirstSeg[k];
  }
  qu->LastSeg[k] = NULL;
}

void addseg(EN_Project *pr, int k, double v, double c)
/*
**-------------------------------------------------------------
**   Input:   k = link segment
**            v = segment volume
**            c = segment quality
**   Output:  none
**   Purpose: adds a segment to start of link k (i.e., upstream
**            of current last segment).
**-------------------------------------------------------------
*/
{
  Pseg seg;
  quality_t *qu = &pr->quality;

  if (qu->FreeSeg != NULL) {
    seg = qu->FreeSeg;
    qu->FreeSeg = seg->prev;
  } else {
    seg = (struct Sseg *)Alloc(sizeof(struct Sseg));
    if (seg == NULL) {
      qu->OutOfMemory = TRUE;
      return;
    }
  }
  seg->v = v;
  seg->c = c;
  seg->prev = NULL;
  if (qu->FirstSeg[k] == NULL) {
    qu->FirstSeg[k] = seg;
  }
  if (qu->LastSeg[k] != NULL) {
    qu->LastSeg[k]->prev = seg;
  }
  qu->LastSeg[k] = seg;
}

void accumulate(EN_Project *pr, long dt)
/*
**-------------------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: accumulates mass flow at nodes and updates nodal
**            quality
**-------------------------------------------------------------
*/
{
  int i, j, k;
  double cseg, v, vseg;
  Pseg seg;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  /* Re-set memory used to accumulate mass & volume */
  memset(qu->VolIn, 0, (net->Nnodes + 1) * sizeof(double));
  memset(qu->MassIn, 0, (net->Nnodes + 1) * sizeof(double));
  memset(qu->TempQual, 0, (net->Nnodes + 1) * sizeof(double));

  /* Compute average conc. of segments adjacent to each node */
  /* (For use if there is no transport through the node) */
  for (k = 1; k <= net->Nlinks; k++) {
    j = DOWN_NODE(k);            /* Downstream node */
    if (qu->FirstSeg[k] != NULL) /* Accumulate concentrations */
    {
      qu->MassIn[j] += qu->FirstSeg[k]->c;
      qu->VolIn[j]++;
    }
    j = UP_NODE(k);             /* Upstream node */
    if (qu->LastSeg[k] != NULL) /* Accumulate concentrations */
    {
      qu->MassIn[j] += qu->LastSeg[k]->c;
      qu->VolIn[j]++;
    }
  }

  for (k = 1; k <= net->Nnodes; k++) {
    if (qu->VolIn[k] > 0.0) {
      qu->TempQual[k] = qu->MassIn[k] / qu->VolIn[k];
    }
  }

  /* Move mass from first segment of each pipe into downstream node */
  memset(qu->VolIn, 0, (net->Nnodes + 1) * sizeof(double));
  memset(qu->MassIn, 0, (net->Nnodes + 1) * sizeof(double));
  for (k = 1; k <= net->Nlinks; k++) {
    i = UP_NODE(k);     /* Upstream node */
    j = DOWN_NODE(k);   /* Downstream node */
    v = ABS(hyd->LinkFlows[k]) * dt; /* Flow volume */

    while (v > 0.0) 
    {
      /* Identify leading segment in pipe */
      seg = qu->FirstSeg[k];
      if (seg == NULL)
        break;

      /* Volume transported from this segment is */
      /* minimum of flow volume & segment volume */
      /* (unless leading segment is also last segment) */
      vseg = seg->v;
      vseg = MIN(vseg, v);
      if (seg == qu->LastSeg[k])
        vseg = v;

      /* Update volume & mass entering downstream node  */
      cseg = seg->c;
      qu->VolIn[j] += vseg;
      qu->MassIn[j] += vseg * cseg;

      /* Reduce flow volume by amount transported */
      v -= vseg;

      /* If all of segment's volume was transferred, then */
      /* replace leading segment with the one behind it   */
      /* (Note that the current seg is recycled for later use.) */
      if (v >= 0.0 && vseg >= seg->v) {
        qu->FirstSeg[k] = seg->prev;
        if (qu->FirstSeg[k] == NULL)
          qu->LastSeg[k] = NULL;
        seg->prev = qu->FreeSeg;
        qu->FreeSeg = seg;
      }

      /* Otherwise reduce segment's volume */
      else {
        seg->v -= vseg;
      }
    } /* End while */
  }   /* Next link */
}

void updatenodes(EN_Project *pr, long dt)
/*
**---------------------------------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: updates concentration at all nodes to mixture of accumulated
**            inflow from connecting pipes.
**
**  Note:     Does not account for source flow effects. TempQual[i] contains
**            average concen. of segments adjacent to node i, used in case
**            there was no inflow into i.
**---------------------------------------------------------------------------
*/
{
  int i;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  /* Update junction quality */
  for (i = 1; i <= net->Njuncs; i++) {
    if (hyd->NodeDemand[i] < 0.0) {
      qu->VolIn[i] -= hyd->NodeDemand[i] * dt;
    }
    if (qu->VolIn[i] > 0.0) {
      qu->NodeQual[i] = qu->MassIn[i] / qu->VolIn[i];
    } else {
      qu->NodeQual[i] = qu->TempQual[i];
    }
  }

  /* Update tank quality */
  updatetanks(pr, dt);

  /* For flow tracing, set source node concen. to 100. */
  if (qu->Qualflag == TRACE)
    qu->NodeQual[qu->TraceNode] = 100.0;
}

void sourceinput(EN_Project *pr, long dt)
/*
**---------------------------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: computes contribution (if any) of mass additions from WQ
**            sources at each node.
**---------------------------------------------------------------------
*/
{
  int j, n;
  double massadded = 0.0, s, volout;
  double qout, qcutoff;
  Psource source;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  /* Establish a flow cutoff which indicates no outflow from a node */
  qcutoff = 10.0 * TINY;

  /* Zero-out the work array TempQual */
  memset(qu->TempQual, 0, (net->Nnodes + 1) * sizeof(double));
  if (qu->Qualflag != CHEM)
    return;

  /* Consider each node */
  for (n = 1; n <= net->Nnodes; n++) {
    double thisDemand = hyd->NodeDemand[n];
    /* Skip node if no WQ source */
    source = net->Node[n].S;
    if (source == NULL)
      continue;
    if (source->C0 == 0.0)
      continue;

    /* Find total flow volume leaving node */
    if (n <= net->Njuncs) {
      volout = qu->VolIn[n]; /* Junctions */
    } else {
      volout = qu->VolIn[n] - (thisDemand * dt); /* Tanks */
    }
    qout = volout / (double)dt;

    /* Evaluate source input only if node outflow > cutoff flow */
    if (qout > qcutoff) {

      /* Mass added depends on type of source */
      s = sourcequal(pr,source);
      switch (source->Type) {
      /* Concen. Source: */
      /* Mass added = source concen. * -(demand) */
      case CONCEN:

        /* Only add source mass if demand is negative */
        if (thisDemand < 0.0) {
          massadded = -s * thisDemand * dt;

          /* If node is a tank then set concen. to 0. */
          /* (It will be re-set to true value in updatesourcenodes()) */
          if (n > net->Njuncs)
            qu->NodeQual[n] = 0.0;
        } else
          massadded = 0.0;
        break;

      /* Mass Inflow Booster Source: */
      case MASS:
        massadded = s * dt;
        break;

      /* Setpoint Booster Source: */
      /* Mass added is difference between source */
      /* & node concen. times outflow volume  */
      case SETPOINT:
        if (s > qu->NodeQual[n]) {
          massadded = (s - qu->NodeQual[n]) * volout;
        } else {
          massadded = 0.0;
        }
        break;

      /* Flow-Paced Booster Source: */
      /* Mass added = source concen. times outflow volume */
      case FLOWPACED:
        massadded = s * volout;
        break;
      }

      /* Source concen. contribution = (mass added / outflow volume) */
      qu->TempQual[n] = massadded / volout;

      /* Update total mass added for time period & simulation */
      source->Smass += massadded;
      if (time->Htime >= time->Rstart) {
        qu->Wsource += massadded;
      }
    }
  }

  /* Add mass inflows from reservoirs to Wsource*/
  if (time->Htime >= time->Rstart) {
    for (j = 1; j <= net->Ntanks; j++) {
      if (net->Tank[j].A == 0.0) {
        n = net->Njuncs + j;
        volout = qu->VolIn[n] - hyd->NodeDemand[n] * dt;
        if (volout > 0.0)
          qu->Wsource += volout * qu->NodeQual[n];
      }
    }
  }
}

void release(EN_Project *pr, long dt)
/*
**---------------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: creates new segments in outflow links from nodes.
**---------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  
  int k, n;
  double c, q, v;
  Pseg seg;

  /* Examine each link */
  for (k = 1; k <= net->Nlinks; k++) {

    /* Ignore links with no flow */
    if (hyd->LinkFlows[k] == 0.0)
      continue;

    /* Find flow volume released to link from upstream node */
    /* (NOTE: Flow volume is allowed to be > link volume.) */
    n = UP_NODE(k);
    q = ABS(hyd->LinkFlows[k]);
    v = q * dt;

    /* Include source contribution in quality released from node. */
    c = qu->NodeQual[n] + qu->TempQual[n];

    /* If link has a last seg, check if its quality     */
    /* differs from that of the flow released from node.*/
    if ((seg = qu->LastSeg[k]) != NULL) {
      /* Quality of seg close to that of node */
      if (ABS(seg->c - c) < qu->Ctol) {
        seg->c = (seg->c * seg->v + c * v) / (seg->v + v); 
        seg->v += v;
      }

      /* Otherwise add a new seg to end of link */
      else
        addseg(pr, k, v, c);
    }

    /* If link has no segs then add a new one. */
    else
      addseg(pr, k, LINKVOL(k), c);
  }
}

void updatesourcenodes(EN_Project *pr, long dt)
/*
**---------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: updates quality at source nodes.
**            (TempQual[n] = concen. added by source at node n)
**---------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  
  int i, n;
  Psource source;

  if (qu->Qualflag != CHEM)
    return;

  /* Examine each WQ source node */
  for (n = 1; n <= net->Nnodes; n++) {
    source = net->Node[n].S;
    if (source == NULL)
      continue;

    /* Add source to current node concen. */
    qu->NodeQual[n] += qu->TempQual[n];

    /* For tanks, node concen. = internal concen. */
    if (n > net->Njuncs) {
      i = n - net->Njuncs;
      if (net->Tank[i].A > 0.0)
        qu->NodeQual[n] = net->Tank[i].C;
    }

    /* Normalize mass added at source to time step */
    source->Smass /= (double)dt;
  }
}

void updatetanks(EN_Project *pr, long dt)
/*
**---------------------------------------------------
**   Input:   dt = current WQ time step
**   Output:  none
**   Purpose: updates tank volumes & concentrations
**---------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  
  int i, n;

  /* Examine each reservoir & tank */
  for (i = 1; i <= net->Ntanks; i++) {
    n = net->Tank[i].Node;
    /* Use initial quality for reservoirs */
    if (net->Tank[i].A == 0.0) {
      qu->NodeQual[n] = net->Node[n].C0;
    }
    /* Update tank WQ based on mixing model */
    else {
      switch (net->Tank[i].MixModel) {
      case MIX2:
        tankmix2(pr, i, dt);
        break;
      case FIFO:
        tankmix3(pr, i, dt);
        break;
      case LIFO:
        tankmix4(pr, i, dt);
        break;
      default:
        tankmix1(pr, i, dt);
        break;
      }
    }
  }
}

////  New version of tankmix1  //// 
void tankmix1(EN_Project *pr, int i, long dt)
/*
**---------------------------------------------
**   Input:   i = tank index
**            dt = current WQ time step
**   Output:  none
**   Purpose: complete mix tank model
**---------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  
  int n;
  double cin;
  double c, cmax, vold, vin;

  Stank *tank = &net->Tank[i];

  /* React contents of tank */
  c = tankreact(pr, tank->C, tank->V, tank->Kb, dt);

  /* Determine tank & volumes */
  vold = tank->V;
  n = tank->Node;
  tank->V += hyd->NodeDemand[n] * dt;
  vin = qu->VolIn[n];

  /* Compute inflow concen. */
  if (vin > 0.0)
    cin = qu->MassIn[n] / vin;
  else
    cin = 0.0;
  cmax = MAX(c, cin);

  /* Mix inflow with tank contents */
  if (vin > 0.0)
    c = (c * vold + cin * vin) / (vold + vin);
  c = MIN(c, cmax);
  c = MAX(c, 0.0);
  tank->C = c;
  qu->NodeQual[n] = tank->C;
}

/*** Updated 10/25/00 ***/
////  New version of tankmix2  //// 
void tankmix2(EN_Project *pr, int i, long dt)
/*
**------------------------------------------------
**   Input:   i = tank index
**            dt = current WQ time step
**   Output:  none
**   Purpose: 2-compartment tank model
**            (seg1 = mixing zone,
**             seg2 = ambient zone)
**------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  
  int k, n;
  double cin,      /* Inflow quality */
      vin,         /* Inflow volume */
      vt,          /* Transferred volume */
      vnet,        /* Net volume change */
      v1max;       /* Full mixing zone volume */
  Pseg seg1, seg2; /* Compartment segments */

  Stank *tank = &pr->network.Tank[i];

  /* Identify segments for each compartment */
  k = net->Nlinks + i;
  seg1 = qu->LastSeg[k];
  seg2 = qu->FirstSeg[k];
  if (seg1 == NULL || seg2 == NULL)
    return;

  /* React contents of each compartment */
  seg1->c = tankreact(pr, seg1->c, seg1->v, tank->Kb, dt);
  seg2->c = tankreact(pr, seg2->c, seg2->v, tank->Kb, dt);

  /* Find inflows & outflows */
  n = tank->Node;
  vnet = hyd->NodeDemand[n] * dt;
  vin = qu->VolIn[n];
  if (vin > 0.0)
    cin = qu->MassIn[n] / vin;
  else
    cin = 0.0;
  v1max = tank->V1max;

  /* Tank is filling */
  vt = 0.0;
  if (vnet > 0.0) {
    vt = MAX(0.0, (seg1->v + vnet - v1max));
    if (vin > 0.0) {
      seg1->c = ((seg1->c) * (seg1->v) + cin * vin) / (seg1->v + vin);
    }
    if (vt > 0.0) {
      seg2->c = ((seg2->c) * (seg2->v) + (seg1->c) * vt) / (seg2->v + vt);
    }
  }

  /* Tank is emptying */
  if (vnet < 0.0) {
    if (seg2->v > 0.0) {
      vt = MIN(seg2->v, (-vnet));
    }
    if (vin + vt > 0.0) {
      seg1->c = ((seg1->c) * (seg1->v) + cin * vin + (seg2->c) * vt) /
                (seg1->v + vin + vt);
    }
  }

  /* Update segment volumes */
  if (vt > 0.0) {
    seg1->v = v1max;
    if (vnet > 0.0)
      seg2->v += vt;
    else
      seg2->v = MAX(0.0, ((seg2->v) - vt));
  } else {
    seg1->v += vnet;
    seg1->v = MIN(seg1->v, v1max);
    seg1->v = MAX(0.0, seg1->v);
    seg2->v = 0.0;
  }
  tank->V += vnet;
  tank->V = MAX(0.0, tank->V);

  /* Use quality of mixed compartment (seg1) to */
  /* represent quality of tank since this is where */
  /* outflow begins to flow from */
  tank->C = seg1->c;
  qu->NodeQual[n] = tank->C;
}

void tankmix3(EN_Project *pr, int i, long dt)
/*
**----------------------------------------------------------
**   Input:   i = tank index
**            dt = current WQ time step
**   Output:  none
**   Purpose: First-In-First-Out (FIFO) tank model
**----------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  
  int k, n;
  double vin, vnet, vout, vseg;
  double cin, vsum, csum;
  Pseg seg;
  Stank *tank = &pr->network.Tank[i];
  k = net->Nlinks + i;
  if (qu->LastSeg[k] == NULL || qu->FirstSeg[k] == NULL)
    return;

  /* React contents of each compartment */
  if (qu->Reactflag) {
    seg = qu->FirstSeg[k];
    while (seg != NULL) {
      seg->c = tankreact(pr, seg->c, seg->v, tank->Kb, dt);
      seg = seg->prev;
    }
  }

  /* Find inflows & outflows */
  n = tank->Node;
  vnet = hyd->NodeDemand[n] * dt;
  vin = qu->VolIn[n];
  vout = vin - vnet;
  if (vin > 0.0)
    cin = qu->MassIn[n] / qu->VolIn[n];
  else
    cin = 0.0;
  tank->V += vnet;
  tank->V = MAX(0.0, tank->V); 

  /* Withdraw flow from first segment */
  vsum = 0.0;
  csum = 0.0;
  while (vout > 0.0) {
    seg = qu->FirstSeg[k];
    if (seg == NULL)
      break;
    vseg = seg->v; /* Flow volume from leading seg */
    vseg = MIN(vseg, vout);
    if (seg == qu->LastSeg[k])
      vseg = vout;
    vsum += vseg;
    csum += (seg->c) * vseg;
    vout -= vseg;                      /* Remaining flow volume */
    if (vout >= 0.0 && vseg >= seg->v) /* Seg used up */
    {
      if (seg->prev) 
      {              
        qu->FirstSeg[k] = seg->prev;
        seg->prev = qu->FreeSeg;
        qu->FreeSeg = seg;
      }    
    } else /* Remaining volume in segment */
    {
      seg->v -= vseg;
    }
  }

  /* Use quality withdrawn from 1st segment */
  /* to represent overall quality of tank */
  if (vsum > 0.0)
    tank->C = csum / vsum;
  else
    tank->C = qu->FirstSeg[k]->c;
  qu->NodeQual[n] = tank->C;

  /* Add new last segment for new flow entering tank */
  if (vin > 0.0) {
    if ((seg = qu->LastSeg[k]) != NULL) {
      /* Quality is the same, so just add flow volume to last seg */
      if (ABS(seg->c - cin) < qu->Ctol)
        seg->v += vin;

      /* Otherwise add a new seg to tank */
      else
        addseg(pr, k, vin, cin);
    }

    /* If no segs left then add a new one. */
    else
      addseg(pr, k, vin, cin);
  }
}

void tankmix4(EN_Project *pr, int i, long dt)
/*
**----------------------------------------------------------
**   Input:   i = tank index
**            dt = current WQ time step
**   Output:  none
**   Purpose: Last In-First Out (LIFO) tank model
**----------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  
  int k, n;
  double vin, vnet, cin, vsum, csum, vseg;
  Pseg seg, tmpseg;
  Stank *tank = &pr->network.Tank[i];
  k = net->Nlinks + i;
  if (qu->LastSeg[k] == NULL || qu->FirstSeg[k] == NULL)
    return;

  /* React contents of each compartment */
  if (qu->Reactflag) {
    seg = qu->LastSeg[k];
    while (seg != NULL) {
      seg->c = tankreact(pr, seg->c, seg->v, tank->Kb, dt);
      seg = seg->prev;
    }
  }

  /* Find inflows & outflows */
  n = tank->Node;
  vnet = hyd->NodeDemand[n] * dt;
  vin = qu->VolIn[n];
  if (vin > 0.0)
    cin = qu->MassIn[n] / qu->VolIn[n];
  else
    cin = 0.0;
  tank->V += vnet;
  tank->V = MAX(0.0, tank->V); 
  tank->C = qu->LastSeg[k]->c;

  /* If tank filling, then create new last seg */
  if (vnet > 0.0) {
    if ((seg = qu->LastSeg[k]) != NULL) {
      /* Quality is the same, so just add flow volume to last seg */
      if (ABS(seg->c - cin) < qu->Ctol)
        seg->v += vnet;

      /* Otherwise add a new last seg to tank */
      /* which points to old last seg */
      else {
        tmpseg = seg;
        qu->LastSeg[k] = NULL;
        addseg(pr, k, vnet, cin);
        qu->LastSeg[k]->prev = tmpseg;
      }
    }

    /* If no segs left then add a new one. */
    else
      addseg(pr, k, vnet, cin);

    /* Update reported tank quality */
    tank->C = qu->LastSeg[k]->c;
  }

  /* If net emptying then remove last segments until vnet consumed */
  else if (vnet < 0.0) {
    vsum = 0.0;
    csum = 0.0;
    vnet = -vnet;
    while (vnet > 0.0) {
      seg = qu->LastSeg[k];
      if (seg == NULL)
        break;
      vseg = seg->v;
      vseg = MIN(vseg, vnet);
      if (seg == qu->FirstSeg[k])
        vseg = vnet;
      vsum += vseg;
      csum += (seg->c) * vseg;
      vnet -= vseg;
      if (vnet >= 0.0 && vseg >= seg->v) /* Seg used up */
      {
        if (seg->prev) 
        {              
          qu->LastSeg[k] = seg->prev;
          seg->prev = qu->FreeSeg;
          qu->FreeSeg = seg;
        }    
      } else /* Remaining volume in segment */
      {
        seg->v -= vseg;
      }
    }
    /* Reported tank quality is mixture of flow released and any inflow */
    tank->C = (csum + qu->MassIn[n]) / (vsum + vin);
  }
  qu->NodeQual[n] = tank->C;
}

double sourcequal(EN_Project *pr, Psource source)
/*
**--------------------------------------------------------------
**   Input:   j = source index
**   Output:  returns source WQ value
**   Purpose: determines source concentration in current time period
**--------------------------------------------------------------
*/
{
  int i;
  long k;
  double c;
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  /* Get source concentration (or mass flow) in original units */
  c = source->C0;

  /* Convert mass flow rate from min. to sec. */
  /* and convert concen. from liters to cubic feet */
  if (source->Type == MASS)
    c /= 60.0;
  else
    c /= pr->Ucf[QUALITY];

  /* Apply time pattern if assigned */
  i = source->Pat;
  if (i == 0)
    return (c);
  k = ((qu->Qtime + time->Pstart) / time->Pstep) % (long)net->Pattern[i].Length;
  return (c * net->Pattern[i].F[k]);
}

double avgqual(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  returns WQ value
**   Purpose: computes average quality in link k
**--------------------------------------------------------------
*/
{
  double vsum = 0.0, msum = 0.0;
  Pseg seg;
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;

  if (qu->Qualflag == NONE)
    return (0.);
  seg = qu->FirstSeg[k];
  while (seg != NULL) {
    vsum += seg->v;
    msum += (seg->c) * (seg->v);
    seg = seg->prev;
  }
  if (vsum > 0.0 && qu->Qtime > 0)
    return (msum / vsum);
  else
    return ((qu->NodeQual[net->Link[k].N1] + qu->NodeQual[net->Link[k].N2]) /
            2.);
}

void ratecoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: determines wall reaction coeff. for each pipe
**--------------------------------------------------------------
*/
{
  int k;
  double kw;
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;

  for (k = 1; k <= net->Nlinks; k++) {
    kw = net->Link[k].Kw;
    if (kw != 0.0)
      kw = piperate(pr, k);
    net->Link[k].Rc = kw;
    qu->PipeRateCoeff[k] = 0.0;
  }
} /* End of ratecoeffs */

double piperate(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  returns reaction rate coeff. for 1st-order wall
**            reactions or mass transfer rate coeff. for 0-order
**            reactions
**   Purpose: finds wall reaction rate coeffs.
**--------------------------------------------------------------
*/
{
  double a, d, u, kf, kw, y, Re, Sh;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;

  d = net->Link[k].Diam; /* Pipe diameter, ft */

  /* Ignore mass transfer if Schmidt No. is 0 */
  if (qu->Sc == 0.0) {
    if (qu->WallOrder == 0.0)
      return (BIG);
    else
      return (net->Link[k].Kw * (4.0 / d) / pr->Ucf[ELEV]);
  }

  /* Compute Reynolds No. */
  a = PI * d * d / 4.0;
  u = ABS(hyd->LinkFlows[k]) / a;
  Re = u * d / hyd->Viscos;

  /* Compute Sherwood No. for stagnant flow  */
  /* (mass transfer coeff. = Diffus./radius) */
  if (Re < 1.0)
    Sh = 2.0;

  /* Compute Sherwood No. for turbulent flow */
  /* using the Notter-Sleicher formula.      */
  else if (Re >= 2300.0)
    Sh = 0.0149 * pow(Re, 0.88) * pow(qu->Sc, 0.333);

  /* Compute Sherwood No. for laminar flow */
  /* using Graetz solution formula.        */
  else {
    y = d / net->Link[k].Len * Re * qu->Sc;
    Sh = 3.65 + 0.0668 * y / (1.0 + 0.04 * pow(y, 0.667));
  }

  /* Compute mass transfer coeff. (in ft/sec) */
  kf = Sh * qu->Diffus / d;

  /* For zero-order reaction, return mass transfer coeff. */
  if (qu->WallOrder == 0.0)
    return (kf);

  /* For first-order reaction, return apparent wall coeff. */
  kw = net->Link[k].Kw / pr->Ucf[ELEV];          /* Wall coeff, ft/sec */
  kw = (4.0 / d) * kw * kf / (kf + ABS(kw)); /* Wall coeff, 1/sec  */
  return (kw);
} /* End of piperate */

double pipereact(EN_Project *pr, int k, double c, double v, long dt)
/*
**------------------------------------------------------------
**   Input:   k = link index
**            c = current WQ in segment
**            v = segment volume
**            dt = time step
**   Output:  returns new WQ value
**   Purpose: computes new quality in a pipe segment after
**            reaction occurs
**------------------------------------------------------------
*/
{
  double cnew, dc, dcbulk, dcwall, rbulk, rwall;
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  /* For water age (hrs), update concentration by timestep */
  if (qu->Qualflag == AGE)
    return (c + (double)dt / 3600.0);

  /* Otherwise find bulk & wall reaction rates */
  rbulk = bulkrate(pr, c, net->Link[k].Kb, qu->BulkOrder) * qu->Bucf;
  rwall = wallrate(pr, c, net->Link[k].Diam, net->Link[k].Kw, net->Link[k].Rc);

  /* Find change in concentration over timestep */
  dcbulk = rbulk * (double)dt;
  dcwall = rwall * (double)dt;

  /* Update cumulative mass reacted */
  if (time->Htime >= time->Rstart) {
    qu->Wbulk += ABS(dcbulk) * v;
    qu->Wwall += ABS(dcwall) * v;
  }

  /* Update concentration */
  dc = dcbulk + dcwall;
  cnew = c + dc;
  cnew = MAX(0.0, cnew);
  return (cnew);
}

double tankreact(EN_Project *pr, double c, double v, double kb, long dt)
/*
**-------------------------------------------------------
**   Input:   c = current WQ in tank
**            v = tank volume
**            kb = reaction coeff.
**            dt = time step
**   Output:  returns new WQ value
**   Purpose: computes new quality in a tank after
**            reaction occurs
**-------------------------------------------------------
*/
{
  double cnew, dc, rbulk;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;

  /*** Updated 9/7/00 ***/
  /* If no reaction then return current WQ */
  if (!qu->Reactflag) {
    return (c);
  }

  /* For water age, update concentration by timestep */
  if (qu->Qualflag == AGE) {
    return (c + (double)dt / 3600.0);
  }

  /* Find bulk reaction rate */
  rbulk = bulkrate(pr, c, kb, qu->TankOrder) * qu->Tucf;

  /* Find concentration change & update quality */
  dc = rbulk * (double)dt;
  if (time->Htime >= time->Rstart) {
    qu->Wtank += ABS(dc) * v;
  }
  cnew = c + dc;
  cnew = MAX(0.0, cnew);
  return (cnew);
}

double bulkrate(EN_Project *pr, double c, double kb, double order)
/*
**-----------------------------------------------------------
**   Input:   c = current WQ concentration
**            kb = bulk reaction coeff.
**            order = bulk reaction order
**   Output:  returns bulk reaction rate
**   Purpose: computes bulk reaction rate (mass/volume/time)
**-----------------------------------------------------------
*/
{
  double c1;
  quality_t *qu = &pr->quality;

  /* Find bulk reaction potential taking into account */
  /* limiting potential & reaction order. */

  /* Zero-order kinetics: */
  if (order == 0.0)
    c = 1.0;

  /* Michaelis-Menton kinetics: */
  else if (order < 0.0) {
    c1 = qu->Climit + SGN(kb) * c;
    if (ABS(c1) < TINY)
      c1 = SGN(c1) * TINY;
    c = c / c1;
  }

  /* N-th order kinetics: */
  else {
    /* Account for limiting potential */
    if (qu->Climit == 0.0)
      c1 = c;
    else
      c1 = MAX(0.0, SGN(kb) * (qu->Climit - c));

    /* Compute concentration potential */
    if (order == 1.0)
      c = c1;
    else if (order == 2.0)
      c = c1 * c;
    else
      c = c1 * pow(MAX(0.0, c), order - 1.0);
  }

  /* Reaction rate = bulk coeff. * potential) */
  if (c < 0)
    c = 0;
  return (kb * c);
}

double wallrate(EN_Project *pr, double c, double d, double kw, double kf)
/*
**------------------------------------------------------------
**   Input:   c = current WQ concentration
**            d = pipe diameter
**            kw = intrinsic wall reaction coeff.
**            kf = mass transfer coeff. for 0-order reaction
**                 (ft/sec) or apparent wall reaction coeff.
**                 for 1-st order reaction (1/sec)
**   Output:  returns wall reaction rate in mass/ft3/sec
**   Purpose: computes wall reaction rate
**------------------------------------------------------------
*/
{
  quality_t *qu = &pr->quality;
  if (kw == 0.0 || d == 0.0) {
    return (0.0);
  }
  if (qu->WallOrder == 0.0) {     /* 0-order reaction */
    kf = SGN(kw) * c * kf;        /* Mass transfer rate (mass/ft2/sec)*/
    kw = kw * SQR(pr->Ucf[ELEV]); /* Reaction rate (mass/ft2/sec) */
    if (ABS(kf) < ABS(kw)) {      /* Reaction mass transfer limited */
      kw = kf;
    }
    return (kw * 4.0 / d); /* Reaction rate (mass/ft3/sec) */
  } else
    return (c * kf); /* 1st-order reaction */
}

/************************* End of QUALITY.C ***************************/
