/*
*********************************************************************

OUTPUT.C -- Binary File Transfer Routines for EPANET Program

VERSION:    2.00
DATE:       5/8/00
            8/15/07    (2.00.11)
AUTHOR:     L. Rossman
            US EPA - NRMRL

********************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "epanet2.h"
#include "funcs.h"
#include "text.h"
#include "types.h"
#include <math.h>
#define EXTERN extern
#include "hash.h"
#include "vars.h"

/* write x[1] to x[n] to file */
size_t f_save(REAL4 *x, int n, FILE *file) {
  return fwrite(x + 1, sizeof(REAL4), n, file);
}


int savenetdata(EN_Project *pr)
/*
**---------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: saves input data in original units to binary
**            output file using fixed-sized (4-byte) records
**---------------------------------------------------------------
*/
{
  int i, nmax;
  INT4 *ibuf;
  REAL4 *x;
  int errcode = 0;

  EN_Network *net = &pr->network;
  out_file_t *out = &pr->out_files;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  time_options_t *time = &pr->time_options;
  FILE *outFile = out->OutFile;

  /* Allocate buffer arrays */
  nmax = MAX(net->Nnodes, net->Nlinks) + 1;
  nmax = MAX(nmax, 15);
  ibuf = (INT4 *)calloc(nmax, sizeof(INT4));
  x = (REAL4 *)calloc(nmax, sizeof(REAL4));
  ERRCODE(MEMCHECK(ibuf));
  ERRCODE(MEMCHECK(x));

  if (!errcode) {
    /* Write integer variables to outFile */
    ibuf[0] = MAGICNUMBER;

    /*** CODEVERSION replaces VERSION ***/ 
    //ibuf[1] = CODEVERSION;                 
    ibuf[1] = 20012;  // keep version at 2.00.12 so that GUI will run 

    ibuf[2] = net->Nnodes;
    ibuf[3] = net->Ntanks;
    ibuf[4] = net->Nlinks;
    ibuf[5] = net->Npumps;
    ibuf[6] = net->Nvalves;
    ibuf[7] = qu->Qualflag;
    ibuf[8] = qu->TraceNode;
    ibuf[9] = par->Flowflag;
    ibuf[10] = par->Pressflag;
    ibuf[11] = rep->Tstatflag;
    ibuf[12] = (INT4)time->Rstart;
    ibuf[13] = (INT4)time->Rstep;
    ibuf[14] = (INT4)time->Dur;
    fwrite(ibuf, sizeof(INT4), 15, outFile);

    /* Write string variables to outFile */
    fwrite(pr->Title[0], sizeof(char), MAXMSG + 1, outFile);
    fwrite(pr->Title[1], sizeof(char), MAXMSG + 1, outFile);
    fwrite(pr->Title[2], sizeof(char), MAXMSG + 1, outFile);
    fwrite(par->InpFname, sizeof(char), MAXFNAME + 1, outFile);
    fwrite(rep->Rpt2Fname, sizeof(char), MAXFNAME + 1, outFile);
    fwrite(qu->ChemName, sizeof(char), MAXID + 1, outFile);
    fwrite(rep->Field[QUALITY].Units, sizeof(char), MAXID + 1, outFile);

    /* Write node ID information to outFile */
    for (i = 1; i <= net->Nnodes; i++) {
      Snode *node = &net->Node[i];
      fwrite(node->ID, MAXID + 1, 1, outFile);
    }

    /* Write link information to outFile            */
    /* (Note: first transfer values to buffer array,*/
    /* then fwrite buffer array at offset of 1 )    */
    for (i = 1; i <= net->Nlinks; i++)
      fwrite(net->Link[i].ID, MAXID + 1, 1, outFile);

    for (i = 1; i <= net->Nlinks; i++)
      ibuf[i] = net->Link[i].N1;
    fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

    for (i = 1; i <= net->Nlinks; i++)
      ibuf[i] = net->Link[i].N2;
    fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

    for (i = 1; i <= net->Nlinks; i++)
      ibuf[i] = net->Link[i].Type;
    fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

    /* Write tank information to outFile.*/
    for (i = 1; i <= net->Ntanks; i++)
      ibuf[i] = net->Tank[i].Node;
    fwrite(ibuf + 1, sizeof(INT4), net->Ntanks, outFile);

    for (i = 1; i <= net->Ntanks; i++)
      x[i] = (REAL4)net->Tank[i].A;
    f_save(x, net->Ntanks, outFile);

    /* Save node elevations to outFile.*/
    for (i = 1; i <= net->Nnodes; i++)
      x[i] = (REAL4)(net->Node[i].El * pr->Ucf[ELEV]);
    f_save(x, net->Nnodes, outFile);
    
    /* Save link lengths & diameters to outFile.*/
    for (i = 1; i <= net->Nlinks; i++)
      x[i] = (REAL4)(net->Link[i].Len * pr->Ucf[ELEV]);
    f_save(x, net->Nlinks, outFile);

    for (i = 1; i <= net->Nlinks; i++) {
      if (net->Link[i].Type != EN_PUMP)
        x[i] = (REAL4)(net->Link[i].Diam * pr->Ucf[DIAM]);
      else
        x[i] = 0.0f;
    }
    if (f_save(x, net->Nlinks, outFile) < (unsigned)net->Nlinks)
      errcode = 308;
  }

  /* Free memory used for buffer arrays */
  free(ibuf);
  free(x);
  return (errcode);
}

int savehyd(EN_Project *pr, long *htime)
/*
**--------------------------------------------------------------
**   Input:   *htime   = current time
**   Output:  returns error code
**   Purpose: saves current hydraulic solution to file HydFile
**            in binary format
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  out_file_t *out = &pr->out_files;
  FILE *HydFile = out->HydFile;
  hydraulics_t *hyd = &pr->hydraulics;
  
  int i;
  INT4 t;
  int errcode = 0;
  REAL4 *x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
  if (x == NULL)
    return 101;

  /* Save current time (htime) */
  t = (INT4)(*htime);
  fwrite(&t, sizeof(INT4), 1, HydFile);

  /* Save current nodal demands (D) */
  for (i = 1; i <= net->Nnodes; i++)
    x[i] = (REAL4)hyd->NodeDemand[i];
  fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);

  /* Copy heads (H) to buffer of floats (x) and save buffer */
  for (i = 1; i <= net->Nnodes; i++)
    x[i] = (REAL4)hyd->NodeHead[i];
  fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);

  /* Force flow in closed links to be zero then save flows */
  for (i = 1; i <= net->Nlinks; i++) {
    if (hyd->LinkStatus[i] <= CLOSED)
      x[i] = 0.0f;
    else
      x[i] = (REAL4)hyd->LinkFlows[i];
  }
  fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);

  /* Copy link status to buffer of floats (x) & write buffer */
  for (i = 1; i <= net->Nlinks; i++)
    x[i] = (REAL4)hyd->LinkStatus[i];
  fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);

  /* Save link settings & check for successful write-to-disk */
  /* (We assume that if any of the previous fwrites failed,  */
  /* then this one will also fail.) */
  for (i = 1; i <= net->Nlinks; i++)
    x[i] = (REAL4)hyd->LinkSetting[i];
  if (fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile) <
      (unsigned)net->Nlinks)
    errcode = 308;
  free(x);
  fflush(HydFile); /* added TNT */
  return (errcode);
} /* End of savehyd */

int savehydstep(EN_Project *pr, long *hydstep)
/*
**--------------------------------------------------------------
**   Input:   *hydstep = next time step
**   Output:  returns error code
**   Purpose: saves next hydraulic timestep to file HydFile
**            in binary format
**--------------------------------------------------------------
*/
{
  out_file_t *out = &pr->out_files;
  INT4 t;
  int errcode = 0;
  t = (INT4)(*hydstep);
  if (fwrite(&t, sizeof(INT4), 1, out->HydFile) < 1)
    errcode = 308;
  if (t == 0)
    fputc(EOFMARK, out->HydFile);
  fflush(out->HydFile); /* added TNT */
  return (errcode);
}

int saveenergy(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: saves energy usage by each pump to outFile
**            in binary format
**--------------------------------------------------------------
*/
{
  
  EN_Network     *net = &pr->network;
  hydraulics_t   *hyd = &pr->hydraulics;
  out_file_t     *out = &pr->out_files;
  parser_data_t  *par = &pr->parser;
  time_options_t *time = &pr->time_options;
  FILE           *outFile = out->OutFile;
  Spump *pump;
  
  int i, j;
  INT4 index;
  REAL4 x[MAX_ENERGY_STATS]; // work array
  double hdur,               // total simulation duration in hours
         t;                  // total pumping time duration

  hdur = time->Dur / 3600.0;
  for (i = 1; i <= net->Npumps; i++) {
    pump = &net->Pump[i];
    if (hdur == 0.0) {
        pump->Energy[TOTAL_COST] *= 24.0;
    }
    else {
        // ... convert total hrs. online to fraction of total time online
        t = pump->Energy[PCNT_ONLINE];  //currently holds total hrs. online
        pump->Energy[PCNT_ONLINE] = t / hdur;

        // ... convert cumulative values to time-averaged ones
        if (t > 0.0) {
            pump->Energy[PCNT_EFFIC]   /= t;
            pump->Energy[KWH_PER_FLOW] /= t;
            pump->Energy[TOTAL_KWH]    /= t;
        }

        // ... convert total cost to cost per day
        pump->Energy[TOTAL_COST] *= 24.0 / hdur;
    }

    // ... express time online and avg. efficiency as percentages
    pump->Energy[PCNT_ONLINE] *= 100.0;
    pump->Energy[PCNT_EFFIC] *= 100.0;

    // ... compute KWH per Million Gallons or per Cubic Meter
    if (par->Unitsflag == SI) {
        pump->Energy[KWH_PER_FLOW] *= (1000. / LPSperCFS / 3600.);
    }
    else {
        pump->Energy[KWH_PER_FLOW] *= (1.0e6 / GPMperCFS / 60.);
    }

    // ... save energy stats to REAL4 work array
    for (j = 0; j < MAX_ENERGY_STATS; j++) {
        x[j] = (REAL4)pump->Energy[j];
    }

    // ... save energy results to output file
    index = pump->Link;
    if (fwrite(&index, sizeof(INT4), 1, outFile) < 1) {
      return (308);
    }
    if (fwrite(x, sizeof(REAL4), MAX_ENERGY_STATS, outFile) < MAX_ENERGY_STATS) {
      return (308);
    }
  }

  // ... compute and save demand charge
  hyd->Emax = hyd->Emax * hyd->Dcost;
  x[0] = (REAL4)hyd->Emax;
  if (fwrite(&x[0], sizeof(REAL4), 1, outFile) < 1) {
    return (308);
  }
  return (0);
}

int readhyd(EN_Project *pr, long *hydtime)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  *hydtime = time of hydraulic solution
**   Returns: 1 if successful, 0 if not
**   Purpose: reads hydraulic solution from file HydFile
**
**   NOTE: A hydraulic solution consists of the current time
**         (hydtime), nodal demands (D) and heads (H), link
**         flows (Q), link status (S), and link settings (K).
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  out_file_t *out = &pr->out_files;
  FILE *HydFile = out->HydFile;
  
  int i;
  INT4 t;
  int result = 1;
  REAL4 *x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
  if (x == NULL)
    return 0;

  if (fread(&t, sizeof(INT4), 1, HydFile) < 1)
    result = 0;
  *hydtime = t;

  if (fread(x + 1, sizeof(REAL4), net->Nnodes, HydFile) < (unsigned)net->Nnodes)
    result = 0;
  else
    for (i = 1; i <= net->Nnodes; i++)
      hyd->NodeDemand[i] = x[i];

  if (fread(x + 1, sizeof(REAL4), net->Nnodes, HydFile) < (unsigned)net->Nnodes)
    result = 0;
  else
    for (i = 1; i <= net->Nnodes; i++)
      hyd->NodeHead[i] = x[i];

  if (fread(x + 1, sizeof(REAL4), net->Nlinks, HydFile) < (unsigned)net->Nlinks)
    result = 0;
  else
    for (i = 1; i <= net->Nlinks; i++)
      hyd->LinkFlows[i] = x[i];

  if (fread(x + 1, sizeof(REAL4), net->Nlinks, HydFile) < (unsigned)net->Nlinks)
    result = 0;
  else
    for (i = 1; i <= net->Nlinks; i++)
      hyd->LinkStatus[i] = (char)x[i];

  if (fread(x + 1, sizeof(REAL4), net->Nlinks, HydFile) < (unsigned)net->Nlinks)
    result = 0;
  else
    for (i = 1; i <= net->Nlinks; i++)
      hyd->LinkSetting[i] = x[i];

  free(x);
  return result;
} /* End of readhyd */

int readhydstep(FILE *hydFile, long *hydstep)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  *hydstep = next hydraulic time step (sec)
**   Returns: 1 if successful, 0 if not
**   Purpose: reads hydraulic time step from file HydFile
**--------------------------------------------------------------
*/
{
  INT4 t;
  if (fread(&t, sizeof(INT4), 1, hydFile) < 1)
    return (0);
  *hydstep = t;
  return (1);
} /* End of readhydstep */

int saveoutput(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes simulation results to output file
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  
  int j;
  int errcode = 0;
  REAL4 *x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
  if (x == NULL)
    return 101;

  /* Write out node results, then link results */
  for (j = DEMAND; j <= QUALITY; j++)
    ERRCODE(nodeoutput(pr, j, x, pr->Ucf[j]));
  for (j = FLOW; j <= FRICTION; j++)
    ERRCODE(linkoutput(pr, j, x, pr->Ucf[j]));
  free(x);
  return (errcode);
} /* End of saveoutput */

int nodeoutput(EN_Project *pr, int j, REAL4 *x, double ucf)
/*
**--------------------------------------------------------------
**   Input:   j   = type of node variable
**            *x  = buffer for node values
**            ucf = units conversion factor
**   Output:  returns error code
**   Purpose: writes results for node variable j to output file
**-----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  out_file_t *out = &pr->out_files;
  quality_t *qu = &pr->quality;
  
  int i;

  /* Load computed results (in proper units) into buffer x */
  switch (j) {
  case DEMAND:
    for (i = 1; i <= net->Nnodes; i++)
      x[i] = (REAL4)(hyd->NodeDemand[i] * ucf);
    break;
  case HEAD:
    for (i = 1; i <= net->Nnodes; i++)
      x[i] = (REAL4)(hyd->NodeHead[i] * ucf);
    break;
  case PRESSURE:
    for (i = 1; i <= net->Nnodes; i++)
      x[i] = (REAL4)((hyd->NodeHead[i] - net->Node[i].El) * ucf);
    break;
  case QUALITY:
    for (i = 1; i <= net->Nnodes; i++)
      x[i] = (REAL4)(qu->NodeQual[i] * ucf);
  }

  /* Write x[1] to x[net->Nnodes] to output file */
  if (fwrite(x + 1, sizeof(REAL4), net->Nnodes, out->TmpOutFile) < (unsigned)net->Nnodes) {
    return (308);
  }
  return (0);
} /* End of nodeoutput */

int linkoutput(EN_Project *pr, int j, REAL4 *x, double ucf)
/*
**----------------------------------------------------------------
**   Input:   j   = type of link variable
**            *x  = buffer for link values
**            ucf = units conversion factor
**   Output:  returns error code
**   Purpose: writes results for link variable j to output file
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  out_file_t *out = &pr->out_files;
  quality_t *qu = &pr->quality;
  
  int i;
  double a, h, q, f;

  /* Load computed results (in proper units) into buffer x */
  switch (j) {
  case FLOW:
    for (i = 1; i <= net->Nlinks; i++)
      x[i] = (REAL4)(hyd->LinkFlows[i] * ucf);
    break;
  case VELOCITY:
    for (i = 1; i <= net->Nlinks; i++) {
      if (net->Link[i].Type == EN_PUMP)
        x[i] = 0.0f;
      else {
        q = ABS(hyd->LinkFlows[i]);
        a = PI * SQR(net->Link[i].Diam) / 4.0;
        x[i] = (REAL4)(q / a * ucf);
      }
    }
    break;
  case HEADLOSS:
    for (i = 1; i <= net->Nlinks; i++) {
      if (hyd->LinkStatus[i] <= CLOSED)
        x[i] = 0.0f;
      else {
        h = hyd->NodeHead[net->Link[i].N1] - hyd->NodeHead[net->Link[i].N2];
        if (net->Link[i].Type != EN_PUMP)
          h = ABS(h);
        if (net->Link[i].Type <= EN_PIPE)
          x[i] = (REAL4)(1000.0 * h / net->Link[i].Len);
        else
          x[i] = (REAL4)(h * ucf);
      }
    }
    break;
  case LINKQUAL:
    for (i = 1; i <= net->Nlinks; i++)
      x[i] = (REAL4)(avgqual(pr,i) * ucf);
    break;
  case STATUS:
    for (i = 1; i <= net->Nlinks; i++)
      x[i] = (REAL4)hyd->LinkStatus[i];
    break;
  case SETTING:
    for (i = 1; i <= net->Nlinks; i++) {
      double setting = hyd->LinkSetting[i];
      if (setting != MISSING)
        switch (net->Link[i].Type) {
        case EN_CVPIPE:
        case EN_PIPE:
          x[i] = (REAL4)setting;
          break;
        case EN_PUMP:
          x[i] = (REAL4)setting;
          break;
        case EN_PRV:
        case EN_PSV:
        case EN_PBV:
          x[i] = (REAL4)(setting * pr->Ucf[PRESSURE]);
          break;
        case EN_FCV:
          x[i] = (REAL4)(setting * pr->Ucf[FLOW]);
          break;
        case EN_TCV:
          x[i] = (REAL4)setting;
          break;
        default:
          x[i] = 0.0f;
        }
      else
        x[i] = 0.0f;
    }
    break;
  case REACTRATE: /* Overall reaction rate in mass/L/day */
    if (qu->Qualflag == NONE)
      memset(x, 0, (net->Nlinks + 1) * sizeof(REAL4));
    else
      for (i = 1; i <= net->Nlinks; i++)
        x[i] = (REAL4)(qu->PipeRateCoeff[i] * ucf);
    break;
  case FRICTION: /* f = 2ghd/(Lu^2) where f = friction factor */
    /* u = velocity, g = grav. accel., h = head  */
    /*loss, d = diam., & L = pipe length         */
    for (i = 1; i <= net->Nlinks; i++) {
      if (net->Link[i].Type <= EN_PIPE && ABS(hyd->LinkFlows[i]) > TINY) {
        h = ABS(hyd->NodeHead[net->Link[i].N1] - hyd->NodeHead[net->Link[i].N2]);
        f = 39.725 * h * pow(net->Link[i].Diam, 5) / net->Link[i].Len / SQR(hyd->LinkFlows[i]);
        x[i] = (REAL4)f;
      } else
        x[i] = 0.0f;
    }
    break;
  }

  /* Write x[1] to x[net->Nlinks] to output file */
  if (fwrite(x + 1, sizeof(REAL4), net->Nlinks, out->TmpOutFile) <
      (unsigned)net->Nlinks)
    return (308);
  return (0);
} /* End of linkoutput */

int savefinaloutput(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: saves time series statistics, reaction rates &
**            epilog to output file.
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  out_file_t *out = &pr->out_files;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  FILE *outFile = out->OutFile;
  
  int errcode = 0;
  REAL4 *x;

  /* Save time series statistic if computed */
  if (rep->Tstatflag != SERIES && out->TmpOutFile != NULL) {
    x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
    if (x == NULL)
      return 101;
    ERRCODE(savetimestat(pr, x, NODEHDR));
    ERRCODE(savetimestat(pr, x, LINKHDR));
    if (!errcode)
      rep->Nperiods = 1;
    fclose(out->TmpOutFile);
    out->TmpOutFile = NULL;
    free(x);
  }

  /* Save avg. reaction rates & file epilog */
  if (outFile != NULL) {
    ERRCODE(savenetreacts(pr, qu->Wbulk, qu->Wwall, qu->Wtank, qu->Wsource));
    ERRCODE(saveepilog(pr));
  }
  return (errcode);
}

int savetimestat(EN_Project *pr, REAL4 *x, HdrType objtype)
/*
**--------------------------------------------------------------
**   Input:   *x  = buffer for node values
**            objtype = NODEHDR (for nodes) or LINKHDR (for links)
**   Output:  returns error code
**   Purpose: computes time series statistic for nodes or links
**            and saves to normal output file.
**
**   NOTE: This routine is dependent on how the output reporting
**         variables were assigned to FieldType in TYPES.H.
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  out_file_t *out = &pr->out_files;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  FILE *outFile = out->OutFile;
  
  int n, n1, n2;
  int i, j, p, errcode = 0;
  long startbyte, skipbytes;
  float *stat1, *stat2, xx;

  /*
    Compute number of bytes in temp output file to skip over (skipbytes)
    when moving from one time period to the next for a particular variable.
  */
  if (objtype == NODEHDR) {
    /*
       For nodes, we start at 0 and skip over node output for all
       node variables minus 1 plus link output for all link variables.
    */
    startbyte = 0;
    skipbytes = (net->Nnodes * (QUALITY - DEMAND) +
                 net->Nlinks * (FRICTION - FLOW + 1)) *
                sizeof(REAL4);
    n = net->Nnodes;
    n1 = DEMAND;
    n2 = QUALITY;
  } else {
    /*
       For links, we start at the end of all node variables and skip
       over node output for all node variables plus link output for
       all link variables minus 1.
    */
    startbyte = net->Nnodes * (QUALITY - DEMAND + 1) * sizeof(REAL4);
    skipbytes = (net->Nnodes * (QUALITY - DEMAND + 1) +
                 net->Nlinks * (FRICTION - FLOW)) *
                sizeof(REAL4);
    n = net->Nlinks;
    n1 = FLOW;
    n2 = FRICTION;
  }
  stat1 = (float *)calloc(n + 1, sizeof(float));
  stat2 = (float *)calloc(n + 1, sizeof(float));
  ERRCODE(MEMCHECK(stat1));
  ERRCODE(MEMCHECK(stat2));

  /* Process each output reporting variable */
  if (!errcode) {
    for (j = n1; j <= n2; j++) {

      /* Initialize stat arrays */
      if (rep->Tstatflag == AVG)
        memset(stat1, 0, (n + 1) * sizeof(float));
      else
        for (i = 1; i <= n; i++) {
          stat1[i] = -MISSING; /* +1E10 */
          stat2[i] = MISSING;  /* -1E10 */
        }

      /* Position temp output file at start of output */
      fseek(out->TmpOutFile, startbyte + (j - n1) * n * sizeof(REAL4), SEEK_SET);

      /* Process each time period */
      for (p = 1; p <= rep->Nperiods; p++) {

        /* Get output results for time period & update stats */
        fread(x + 1, sizeof(REAL4), n, out->TmpOutFile);
        for (i = 1; i <= n; i++) {
          xx = x[i];
          if (objtype == LINKHDR) {
            if (j == FLOW)
              xx = ABS(xx);
            if (j == STATUS) {
              if (xx >= OPEN)
                xx = 1.0;
              else
                xx = 0.0;
            }
          }
          if (rep->Tstatflag == AVG)
            stat1[i] += xx;
          else {
            stat1[i] = MIN(stat1[i], xx);
            stat2[i] = MAX(stat2[i], xx);
          }
        }

        /* Advance file to next period */
        if (p < rep->Nperiods)
          fseek(out->TmpOutFile, skipbytes, SEEK_CUR);
      }

      /* Compute resultant stat & save to regular output file */
      switch (rep->Tstatflag) {
      case AVG:
        for (i = 1; i <= n; i++)
          x[i] = stat1[i] / (float)rep->Nperiods;
        break;
      case MIN:
        for (i = 1; i <= n; i++)
          x[i] = stat1[i];
        break;
      case MAX:
        for (i = 1; i <= n; i++)
          x[i] = stat2[i];
        break;
      case RANGE:
        for (i = 1; i <= n; i++)
          x[i] = stat2[i] - stat1[i];
        break;
      }
      if (objtype == LINKHDR && j == STATUS) {
        for (i = 1; i <= n; i++) {
          if (x[i] < 0.5f)
            x[i] = CLOSED;
          else
            x[i] = OPEN;
        }
      }
      if (fwrite(x + 1, sizeof(REAL4), n, outFile) < (unsigned)n)
        errcode = 308;

      /* Update internal output variables where applicable */
      if (objtype == NODEHDR)
        switch (j) {
        case DEMAND:
          for (i = 1; i <= n; i++)
            hyd->NodeDemand[i] = x[i] / pr->Ucf[DEMAND];
          break;
        case HEAD:
          for (i = 1; i <= n; i++)
            hyd->NodeHead[i] = x[i] / pr->Ucf[HEAD];
          break;
        case QUALITY:
          for (i = 1; i <= n; i++)
            qu->NodeQual[i] = x[i] / pr->Ucf[QUALITY];
          break;
        }
      else if (j == FLOW)
        for (i = 1; i <= n; i++)
          hyd->LinkFlows[i] = x[i] / pr->Ucf[FLOW];
    }
  }

  /* Free allocated memory */
  free(stat1);
  free(stat2);
  return (errcode);
}

int savenetreacts(EN_Project *pr, double wbulk, double wwall, double wtank, double wsource)
/*
**-----------------------------------------------------
**  Writes average network-wide reaction rates (in
**  mass/hr) to binary output file.
**-----------------------------------------------------
*/
{
  out_file_t *out = &pr->out_files;
  time_options_t *time = &pr->time_options;
  FILE *outFile = out->OutFile;
  
  int errcode = 0;
  double t;
  REAL4 w[4];
  if (time->Dur > 0)
    t = (double)time->Dur / 3600.;
  else
    t = 1.;
  w[0] = (REAL4)(wbulk / t);
  w[1] = (REAL4)(wwall / t);
  w[2] = (REAL4)(wtank / t);
  w[3] = (REAL4)(wsource / t);
  if (fwrite(w, sizeof(REAL4), 4, outFile) < 4)
    errcode = 308;
  return (errcode);
}

int saveepilog(EN_Project *pr)
/*
**-------------------------------------------------
**  Writes Nperiods, Warnflag, & Magic Number to
**  end of binary output file.
**-------------------------------------------------
*/
{
  out_file_t *out = &pr->out_files;
  report_options_t *rep = &pr->report;
  FILE *outFile = out->OutFile;
  
  int errcode = 0;
  INT4 i;
  i = rep->Nperiods;
  if (fwrite(&i, sizeof(INT4), 1, outFile) < 1)
    errcode = 308;
  i = pr->Warnflag;
  if (fwrite(&i, sizeof(INT4), 1, outFile) < 1)
    errcode = 308;
  i = MAGICNUMBER;
  if (fwrite(&i, sizeof(INT4), 1, outFile) < 1)
    errcode = 308;
  return (errcode);
}

/********************** END OF OUTPUT.C **********************/
