/*
*********************************************************************

REPORT.C -- Reporting Routines for EPANET Program

VERSION:    2.00
DATE:       5/30/00
            6/24/02
            8/15/07  (2.00.11)
            2/14/08  (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL

This module contains various procedures (all beginning with
'write') that are called from other modules to write formatted
output to a report file.

It also contains function disconnected(), called from writehydwarn()
and writehyderr(), that checks if a hydraulic solution causes a
network to become disconnected.

The function writeline(pr, S) is used throughout to write a
formatted string S to the report file.

********************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include "funcs.h"
#include "hash.h"
#include "text.h"
#include "types.h"
#include <math.h>
#include <time.h>

#undef WINDOWS
#ifdef _WIN32
#define WINDOWS
#define snprintf _snprintf
#endif

#define MAXCOUNT 10 /* Max. # of disconnected nodes listed */

/* Defined in enumstxt.h */
extern char *NodeTxt[];
extern char *LinkTxt[];
extern char *StatTxt[];
extern char *TstatTxt[];
extern char *RptFormTxt[];
extern char *DemandModelTxt[];

typedef REAL4 *Pfloat;
void writenodetable(EN_Project *pr, Pfloat *);
void writelinktable(EN_Project *pr, Pfloat *);

int writereport(EN_Project *pr)
/*
**------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes formatted output report to file
**
**   Calls strcomp() from the EPANET.C module.
**------------------------------------------------------
*/
{
  report_options_t *rep = &pr->report;
  parser_data_t *par = &pr->parser;
  
  char tflag;
  FILE *tfile;
  int errcode = 0;

  /* If no secondary report file specified then    */
  /* write formatted output to primary report file. */
  rep->Fprinterr = FALSE;
  if (rep->Rptflag && strlen(rep->Rpt2Fname) == 0 && rep->RptFile != NULL) {
    if (rep->Energyflag)
      writeenergy(pr);
    errcode = writeresults(pr);
  }

  /* A secondary report file was specified */
  else if (strlen(rep->Rpt2Fname) > 0) {

    /* If secondary report file has same name as either input */
    /* or primary report file then use primary report file.   */
    if (strcomp(rep->Rpt2Fname, par->InpFname) || strcomp(rep->Rpt2Fname, rep->Rpt1Fname)) {
      if (rep->Energyflag)
        writeenergy(pr);
      errcode = writeresults(pr);
    }

    /* Otherwise write report to secondary report file. */
    else {

      /* Try to open file */
      tfile = rep->RptFile;
      tflag = rep->Rptflag;
      if ((rep->RptFile = fopen(rep->Rpt2Fname, "wt")) == NULL) {
        rep->RptFile = tfile;
        rep->Rptflag = tflag;
        errcode = 303;
      }

      /* Write full formatted report to file */
      else {
        rep->Rptflag = 1;
        writelogo(pr);
        if (rep->Summaryflag)
          writesummary(pr);
        if (rep->Energyflag)
          writeenergy(pr);
        errcode = writeresults(pr);
        fclose(rep->RptFile);
        rep->RptFile = tfile;
        rep->Rptflag = tflag;
      }
    }
  }

  /* Special error handler for write-to-file error */
  if (rep->Fprinterr)
    errmsg(pr,309);
  return (errcode);
} /* End of writereport */

void writelogo(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes program logo to report file.
**--------------------------------------------------------------
*/
{
  report_options_t *rep = &pr->report;
  
  int version;
  int major;
  int minor;
  char s[80];
  time_t timer; /* time_t structure & functions time() & */
                /* ctime() are defined in time.h         */

  version = CODEVERSION;
  major = version / 10000;
  minor = (version % 10000) / 100;

  time(&timer);
  strcpy(rep->DateStamp, ctime(&timer));
  rep->PageNum = 1;
  rep->LineNum = 2;
  fprintf(rep->RptFile, FMT18);
  fprintf(rep->RptFile, "%s", rep->DateStamp);
  writeline(pr, LOGO1);
  writeline(pr, LOGO2);
  writeline(pr, LOGO3);
  writeline(pr, LOGO4);
  sprintf(s, LOGO5, major, minor);
  writeline(pr, s);
  writeline(pr, LOGO6);
  writeline(pr, "");
} /* End of writelogo */

void writesummary(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes summary system information to report file
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  time_options_t *time = &pr->time_options;
  time_options_t *ti = &pr->time_options;
  
  char s[MAXFNAME + 1];
  int i;
  int nres = 0;

  for (i = 0; i < 3; i++) {
    if (strlen(pr->Title[i]) > 0) {
      sprintf(s, "%-.70s", pr->Title[i]);
      writeline(pr, s);
    }
  }
  writeline(pr, " ");
  sprintf(s, FMT19, par->InpFname);
  writeline(pr, s);
  sprintf(s, FMT20, net->Njuncs);
  writeline(pr, s);
  for (i = 1; i <= net->Ntanks; i++)
    if (net->Tank[i].A == 0.0)
      nres++;
  sprintf(s, FMT21a, nres);
  writeline(pr, s);
  sprintf(s, FMT21b, net->Ntanks - nres);
  writeline(pr, s);
  sprintf(s, FMT22, net->Npipes);
  writeline(pr, s);
  sprintf(s, FMT23, net->Npumps);
  writeline(pr, s);
  sprintf(s, FMT24, net->Nvalves);
  writeline(pr, s);
  sprintf(s, FMT25, RptFormTxt[hyd->Formflag]);
  writeline(pr, s);
  sprintf(s, FMT25a, DemandModelTxt[hyd->DemandModel]);
  writeline(pr, s);
  sprintf(s, FMT26, time->Hstep * pr->Ucf[TIME], rep->Field[TIME].Units);
  writeline(pr, s);
  sprintf(s, FMT27, hyd->Hacc);
  writeline(pr, s);

  if (hyd->HeadErrorLimit > 0.0) {
      sprintf(s, FMT27d, hyd->HeadErrorLimit*pr->Ucf[HEAD], rep->Field[HEAD].Units);
      writeline(pr, s);
  }
  if (hyd->FlowChangeLimit > 0.0) {
      sprintf(s, FMT27e, hyd->FlowChangeLimit*pr->Ucf[FLOW], rep->Field[FLOW].Units);
      writeline(pr, s);
  }

  sprintf(s, FMT27a, hyd->CheckFreq); 
  writeline(pr, s);                   
  sprintf(s, FMT27b, hyd->MaxCheck);       
  writeline(pr, s);                   
  sprintf(s, FMT27c, hyd->DampLimit);      
  writeline(pr, s);                   

  sprintf(s, FMT28, hyd->MaxIter);
  writeline(pr, s);
  if (qu->Qualflag == NONE || time->Dur == 0.0)
    sprintf(s, FMT29);
  else if (qu->Qualflag == CHEM)
    sprintf(s, FMT30, qu->ChemName);
  else if (qu->Qualflag == TRACE)
    sprintf(s, FMT31, net->Node[qu->TraceNode].ID);
  else if (qu->Qualflag == AGE)
    sprintf(s, FMT32);
  writeline(pr, s);
  if (qu->Qualflag != NONE && ti->Dur > 0) {
    sprintf(s, FMT33, (float)qu->Qstep / 60.0);
    writeline(pr, s);
    sprintf(s, FMT34, qu->Ctol * pr->Ucf[QUALITY], rep->Field[QUALITY].Units);
    writeline(pr, s);
  }
  sprintf(s, FMT36, hyd->SpGrav);
  writeline(pr, s);
  sprintf(s, FMT37a, hyd->Viscos / VISCOS);
  writeline(pr, s);
  sprintf(s, FMT37b, qu->Diffus / DIFFUS);
  writeline(pr, s);
  sprintf(s, FMT38, hyd->Dmult);
  writeline(pr, s);
  sprintf(s, FMT39, time->Dur * pr->Ucf[TIME], rep->Field[TIME].Units);
  writeline(pr, s);
  if (rep->Rptflag) {
    sprintf(s, FMT40);
    writeline(pr, s);
    if (rep->Nodeflag == 0)
      writeline(pr, FMT41);
    if (rep->Nodeflag == 1)
      writeline(pr, FMT42);
    if (rep->Nodeflag == 2)
      writeline(pr, FMT43);
    writelimits(pr, DEMAND, QUALITY);
    if (rep->Linkflag == 0)
      writeline(pr, FMT44);
    if (rep->Linkflag == 1)
      writeline(pr, FMT45);
    if (rep->Linkflag == 2)
      writeline(pr, FMT46);
    writelimits(pr, DIAM, HEADLOSS);
  }
  writeline(pr, " ");
} /* End of writesummary */

void writehydstat(EN_Project *pr, int iter, double relerr)
/*
**--------------------------------------------------------------
**   Input:   iter   = # iterations to find hydraulic solution
**            relerr = convergence error in hydraulic solution
**   Output:  none
**   Purpose: writes hydraulic status report for solution found
**            at current time period to report file
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;

  
  double *NodeDemand = hyd->NodeDemand;
  Stank *Tank = net->Tank;
  Slink *Link = net->Link;
  
  int i, n;
  StatType newstat;
  char s1[MAXLINE + 1];

  /*** Updated 6/24/02 ***/
  char atime[13];

  /* Display system status */
  strcpy(atime, clocktime(rep->Atime, time->Htime));
  if (iter > 0) {
    if (relerr <= hyd->Hacc)
      sprintf(s1, FMT58, atime, iter);
    else
      sprintf(s1, FMT59, atime, iter, relerr);
    writeline(pr, s1);
  }

  /*
     Display status changes for tanks.
     D[n] is net inflow to tank at node n.
     Old tank status is stored in OldStat[]
     at indexes Nlinks+1 to Nlinks+Ntanks.
  */
  for (i = 1; i <= net->Ntanks; i++) {
    n = net->Tank[i].Node;
    if (ABS(NodeDemand[n]) < 0.001)
      newstat = CLOSED;
    else if (NodeDemand[n] > 0.0)
      newstat = FILLING;
    else if (NodeDemand[n] < 0.0)
      newstat = EMPTYING;
    else
      newstat = hyd->OldStat[net->Nlinks + i];
    if (newstat != hyd->OldStat[net->Nlinks + i]) {
      if (Tank[i].A > 0.0)
        snprintf(s1, MAXLINE, FMT50, atime, net->Node[n].ID, StatTxt[newstat],
                 (hyd->NodeHead[n] - net->Node[n].El) * pr->Ucf[HEAD],
                 rep->Field[HEAD].Units);
      else
        snprintf(s1, MAXLINE, FMT51, atime, net->Node[n].ID, StatTxt[newstat]);
      writeline(pr, s1);
      hyd->OldStat[net->Nlinks + i] = newstat;
    }
  }

  /* Display status changes for links */
  for (i = 1; i <= net->Nlinks; i++) {
    if (hyd->LinkStatus[i] != hyd->OldStat[i]) {
      if (time->Htime == 0)
        sprintf(s1, FMT52, atime, LinkTxt[(int)net->Link[i].Type], net->Link[i].ID,
                StatTxt[(int)hyd->LinkStatus[i]]);
      else
        sprintf(s1, FMT53, atime, LinkTxt[Link[i].Type], net->Link[i].ID,
                StatTxt[hyd->OldStat[i]], StatTxt[hyd->LinkStatus[i]]);
      writeline(pr, s1);
      hyd->OldStat[i] = hyd->LinkStatus[i];
    }
  }
  writeline(pr, " ");
} /* End of writehydstat */


void writemassbalance(EN_Project *pr)
/*
**-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes water quality mass balance ratio
**            (Outflow + Final Storage) / Inflow + Initial Storage)
**            to report file.
**-------------------------------------------------------------
*/
{
    char s1[MAXMSG+1];
    char *units[] = {"", " (mg)", " (ug)", " (hrs)"};
    int  kunits = 0;
    quality_t *qual = &pr->quality;

    if      (qual->Qualflag == TRACE) kunits = 1;
    else if (qual->Qualflag == AGE)   kunits = 3;
    else
    {
        if      (match(qual->ChemUnits, "mg")) kunits = 1;
        else if (match(qual->ChemUnits, "ug")) kunits = 2;
    }

    snprintf(s1, MAXMSG, "Water Quality Mass Balance%s", units[kunits]);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "================================");
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Initial Mass:      %12.5e", qual->massbalance.initial);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Inflow:       %12.5e", qual->massbalance.inflow);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Outflow:      %12.5e", qual->massbalance.outflow);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Reacted:      %12.5e", qual->massbalance.reacted);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Final Mass:        %12.5e", qual->massbalance.final);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Ratio:         %-0.5f", qual->massbalance.ratio);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "================================\n");
    writeline(pr, s1);
}


void writeenergy(EN_Project *pr)
/*
**-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes energy usage report to report file
**-------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  Spump *pump;

  int j;
  double csum;
  char s[MAXLINE + 1];
  if (net->Npumps == 0)
    return;
  writeline(pr, " ");
  writeheader(pr,ENERHDR, 0);
  csum = 0.0;
  for (j = 1; j <= net->Npumps; j++) {
    pump = &net->Pump[j];
    csum += pump->Energy[TOTAL_COST];
    if (rep->LineNum == (long)rep->PageSize)
      writeheader(pr, ENERHDR, 1);
    sprintf(s, "%-8s  %6.2f %6.2f %9.2f %9.2f %9.2f %9.2f",
            net->Link[pump->Link].ID, pump->Energy[PCNT_ONLINE], pump->Energy[PCNT_EFFIC],
            pump->Energy[KWH_PER_FLOW], pump->Energy[TOTAL_KWH], pump->Energy[MAX_KW],
            pump->Energy[TOTAL_COST]);
    writeline(pr, s);
  }
  fillstr(s, '-', 63);
  writeline(pr, s);

  /*** Updated 6/24/02 ***/
  sprintf(s, FMT74, "", hyd->Emax * hyd->Dcost);
  writeline(pr, s);
  sprintf(s, FMT75, "", csum + hyd->Emax * hyd->Dcost);
  /*** End of update ***/

  writeline(pr, s);
  writeline(pr, " ");
} /* End of writeenergy */

int writeresults(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes simulation results to report file
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  out_file_t *out = &pr->out_files;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  FILE *outFile = out->OutFile;

  
  Pfloat *x; /* Array of pointers to floats */
  int j, m, n, np, nnv, nlv;
  int errcode = 0;

  /*
  **-----------------------------------------------------------
  **  NOTE:  The OutFile contains results for 4 node variables
  **         (demand, head, pressure, & quality) and 8 link
  **         variables (flow, velocity, headloss, quality,
  **         status, setting, reaction rate & friction factor)
  **         at each reporting time.
  **-----------------------------------------------------------
  */

  /* Return if no output file */
  if (outFile == NULL)
    return (106);

  /* Return if no nodes or links selected for reporting */
  /* or if no node or link report variables enabled.    */
  if (!rep->Nodeflag && !rep->Linkflag) {
    return (errcode);
  }
  nnv = 0;
  for (j = ELEV; j <= QUALITY; j++) {
    nnv += rep->Field[j].Enabled;
  }
  nlv = 0;
  for (j = LENGTH; j <= FRICTION; j++) {
    nlv += rep->Field[j].Enabled;
  }
  if (nnv == 0 && nlv == 0) {
    return (errcode);
  }

  /* Allocate memory for output variables. */
  /* m = larger of # node variables & # link variables */
  /* n = larger of # nodes & # links */
  m = MAX((QUALITY - DEMAND + 1), (FRICTION - FLOW + 1));
  n = MAX((net->Nnodes + 1), (net->Nlinks + 1));
  x = (Pfloat *)calloc(m, sizeof(Pfloat));
  ERRCODE(MEMCHECK(x));
  if (errcode)
    return (errcode);
  for (j = 0; j < m; j++) {
    x[j] = (REAL4 *)calloc(n, sizeof(REAL4));
    ERRCODE(MEMCHECK(x[j]));
  }
  if (errcode)
    return (errcode);

  /* Re-position output file & initialize report time. */
  fseek(outFile, out->OutOffset2, SEEK_SET);
  time->Htime = time->Rstart;

  /* For each reporting time: */
  for (np = 1; np <= rep->Nperiods; np++) {

    /* Read in node results & write node table. */
    /* (Remember to offset x[j] by 1 because array is zero-based). */
    for (j = DEMAND; j <= QUALITY; j++) {
      fread((x[j - DEMAND]) + 1, sizeof(REAL4), net->Nnodes, outFile);
    }
    if (nnv > 0 && rep->Nodeflag > 0) {
      writenodetable(pr,x);
    }

    /* Read in link results & write link table. */
    for (j = FLOW; j <= FRICTION; j++) {
      fread((x[j - FLOW]) + 1, sizeof(REAL4), net->Nlinks, outFile);
    }
    if (nlv > 0 && rep->Linkflag > 0) {
      writelinktable(pr,x);
    }
    time->Htime += time->Rstep;
  }

  /* Free allocated memory */
  for (j = 0; j < m; j++) {
    free(x[j]);
  }
  free(x);
  return (errcode);
} /* End of writereport */

void writenodetable(EN_Project *pr, Pfloat *x)
/*
**---------------------------------------------------------------
**   Input:   x = pointer to node results for current time
**   Output:  none
**   Purpose: writes node results for current time to report file
**---------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;

  int i, j;
  char s[MAXLINE + 1], s1[16];
  double y[MAXVAR];

  /* Write table header */
  writeheader(pr, NODEHDR, 0);

  /* For each node: */
  for (i = 1; i <= net->Nnodes; i++) {
    Snode *node = &net->Node[i];
    /* Place results for each node variable in y */
    y[ELEV] = node->El * pr->Ucf[ELEV];
    for (j = DEMAND; j <= QUALITY; j++)
      y[j] = *((x[j - DEMAND]) + i);

    /* Check if node gets reported on */
    if ((rep->Nodeflag == 1 || node->Rpt) && checklimits(rep, y, ELEV, QUALITY)) {

      /* Check if new page needed */
      if (rep->LineNum == (long)rep->PageSize)
        writeheader(pr, NODEHDR, 1);

      /* Add node ID and each reported field to string s */
      sprintf(s, "%-15s", node->ID);
      for (j = ELEV; j <= QUALITY; j++) {
        if (rep->Field[j].Enabled == TRUE) {

          /*** Updated 6/24/02 ***/
          if (fabs(y[j]) > 1.e6)
            sprintf(s1, "%10.2e", y[j]);
          else
            sprintf(s1, "%10.*f", rep->Field[j].Precision, y[j]);
          /*** End of update ***/

          strcat(s, s1);
        }
      }

      /* Note if node is a reservoir/tank */
      if (i > net->Njuncs) {
        strcat(s, "  ");
        strcat(s, NodeTxt[getnodetype(net,i)]);
      }

      /* Write results for node */
      writeline(pr, s);
    }
  }
  writeline(pr, " ");
}

void writelinktable(EN_Project *pr, Pfloat *x)
/*
**---------------------------------------------------------------
**   Input:   x = pointer to link results for current time
**   Output:  none
**   Purpose: writes link results for current time to report file
**---------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  Slink *Link = net->Link;
  double *Ucf = pr->Ucf;

  
  int i, j, k;
  char s[MAXLINE + 1], s1[16];
  double y[MAXVAR];

  /* Write table header */
  writeheader(pr,LINKHDR, 0);

  /* For each link: */
  for (i = 1; i <= net->Nlinks; i++) {

    /* Place results for each link variable in y */
    y[LENGTH] = Link[i].Len * Ucf[LENGTH];
    y[DIAM] = Link[i].Diam * Ucf[DIAM];
    for (j = FLOW; j <= FRICTION; j++)
      y[j] = *((x[j - FLOW]) + i);

    /* Check if link gets reported on */
    if ((rep->Linkflag == 1 || Link[i].Rpt) && checklimits(rep, y, DIAM, FRICTION)) {

      /* Check if new page needed */
      if (rep->LineNum == (long)rep->PageSize)
        writeheader(pr,LINKHDR, 1);

      /* Add link ID and each reported field to string s */
      sprintf(s, "%-15s", Link[i].ID);
      for (j = LENGTH; j <= FRICTION; j++) {
        if (rep->Field[j].Enabled == TRUE) {
          if (j == STATUS) {
            if (y[j] <= CLOSED)
              k = CLOSED;
            else if (y[j] == ACTIVE)
              k = ACTIVE;
            else
              k = OPEN;
            sprintf(s1, "%10s", StatTxt[k]);
          }

          /*** Updated 6/24/02 ***/
          else {
            if (fabs(y[j]) > 1.e6)
              sprintf(s1, "%10.2e", y[j]);
            else
              sprintf(s1, "%10.*f", rep->Field[j].Precision, y[j]);
          }
          /*** End of update ***/

          strcat(s, s1);
        }
      }

      /* Note if link is a pump or valve */
      if ((j = Link[i].Type) > EN_PIPE) {
        strcat(s, "  ");
        strcat(s, LinkTxt[j]);
      }

      /* Write results for link */
      writeline(pr, s);
    }
  }
  writeline(pr, " ");
}

void writeheader(EN_Project *pr, int type, int contin)
/*
**--------------------------------------------------------------
**   Input:   type   = table type
**            contin = table continuation flag
**   Output:  none
**   Purpose: writes column headings for output report tables
**--------------------------------------------------------------
*/
{
  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  time_options_t *time = &pr->time_options;
  
  char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1], s3[MAXLINE + 1];
  int i, n;

  /* Move to next page if < 11 lines remain on current page. */
  if (rep->Rptflag && rep->LineNum + 11 > (long)rep->PageSize) {
    while (rep->LineNum < (long)rep->PageSize)
      writeline(pr, " ");
  }
  writeline(pr, " ");

  /* Hydraulic Status Table */
  if (type == STATHDR) {
    sprintf(s, FMT49);
    if (contin)
      strcat(s, t_CONTINUED);
    writeline(pr, s);
    fillstr(s, '-', 70);
    writeline(pr, s);
  }

  /* Energy Usage Table */
  if (type == ENERHDR) {
    if (par->Unitsflag == SI)
      strcpy(s1, t_perM3);
    else
      strcpy(s1, t_perMGAL);
    sprintf(s, FMT71);
    if (contin)
      strcat(s, t_CONTINUED);
    writeline(pr, s);
    fillstr(s, '-', 63);
    writeline(pr, s);
    sprintf(s, FMT72);
    writeline(pr, s);
    sprintf(s, FMT73, s1);
    writeline(pr, s);
    fillstr(s, '-', 63);
    writeline(pr, s);
  }

  /* Node Results Table */
  if (type == NODEHDR) {
    if (rep->Tstatflag == RANGE)
      sprintf(s, FMT76, t_DIFFER);
    else if (rep->Tstatflag != SERIES)
      sprintf(s, FMT76, TstatTxt[rep->Tstatflag]);
    else if (time->Dur == 0)
      sprintf(s, FMT77);
    else
      sprintf(s, FMT78, clocktime(rep->Atime, time->Htime));
    if (contin)
      strcat(s, t_CONTINUED);
    writeline(pr, s);
    n = 15;
    sprintf(s2, "%15s", "");
    strcpy(s, t_NODEID);
    sprintf(s3, "%-15s", s);
    for (i = ELEV; i < QUALITY; i++)
      if (rep->Field[i].Enabled == TRUE) {
        n += 10;
        sprintf(s, "%10s", rep->Field[i].Name);
        strcat(s2, s);
        sprintf(s, "%10s", rep->Field[i].Units);
        strcat(s3, s);
      }
    if (rep->Field[QUALITY].Enabled == TRUE) {
      n += 10;
      sprintf(s, "%10s", qu->ChemName);
      strcat(s2, s);
      sprintf(s, "%10s", qu->ChemUnits);
      strcat(s3, s);
    }
    fillstr(s1, '-', n);
    writeline(pr, s1);
    writeline(pr, s2);
    writeline(pr, s3);
    writeline(pr, s1);
  }

  /* Link Results Table */
  if (type == LINKHDR) {
    if (rep->Tstatflag == RANGE)
      sprintf(s, FMT79, t_DIFFER);
    else if (rep->Tstatflag != SERIES)
      sprintf(s, FMT79, TstatTxt[rep->Tstatflag]);
    else if (time->Dur == 0)
      sprintf(s, FMT80);
    else
      sprintf(s, FMT81, clocktime(rep->Atime, time->Htime));
    if (contin)
      strcat(s, t_CONTINUED);
    writeline(pr, s);
    n = 15;
    sprintf(s2, "%15s", "");
    strcpy(s, t_LINKID);
    sprintf(s3, "%-15s", s);
    for (i = LENGTH; i <= FRICTION; i++)
      if (rep->Field[i].Enabled == TRUE) {
        n += 10;
        sprintf(s, "%10s", rep->Field[i].Name);
        strcat(s2, s);
        sprintf(s, "%10s", rep->Field[i].Units);
        strcat(s3, s);
      }
    fillstr(s1, '-', n);
    writeline(pr, s1);
    writeline(pr, s2);
    writeline(pr, s3);
    writeline(pr, s1);
  }
} /* End of writeheader */

void writeline(EN_Project *pr, char *s)
/*
**--------------------------------------------------------------
**   Input:   *s = text string
**   Output:  none
**   Purpose: writes a line of output to report file
**--------------------------------------------------------------
*/
{
  report_options_t *rpt = &pr->report;

  if (rpt->RptFile == NULL) {
    return;
  }
  if (rpt->Rptflag) {
    if (rpt->LineNum == (long)rpt->PageSize) {
      rpt->PageNum++;
      if (fprintf(rpt->RptFile, FMT82, (int)rpt->PageNum, pr->Title[0]) == EOF) {
        rpt->Fprinterr = TRUE;
      }
      rpt->LineNum = 3;
    }
  }
  if (fprintf(rpt->RptFile, "\n  %s", s) == EOF) {
    rpt->Fprinterr = TRUE;
  }
  rpt->LineNum++;
} /* End of writeline */

void writerelerr(EN_Project *pr, int iter, double relerr)
/*
**-----------------------------------------------------------------
**   Input:   iter   = current iteration of hydraulic solution
**            relerr = current convergence error
**   Output:  none
**   Purpose: writes out convergence status of hydraulic solution
**-----------------------------------------------------------------
*/
{
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  
  if (iter == 0) {
    sprintf(pr->Msg, FMT64, clocktime(rep->Atime, time->Htime));
    writeline(pr, pr->Msg);
  } else {
    sprintf(pr->Msg, FMT65, iter, relerr);
    writeline(pr, pr->Msg);
  }
} /* End of writerelerr */

void writestatchange(EN_Project *pr, int k, char s1, char s2)
/*
**--------------------------------------------------------------
**   Input:   k  = link index
**            s1 = old link status
**            s2 = new link status
**   Output:  none
**   Purpose: writes change in link status to output report
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *Link = net->Link;
  double *Ucf = pr->Ucf;
  double *LinkSetting = hyd->LinkSetting;
  
  int j1, j2;
  double setting;

  /* We have a pump/valve setting change instead of a status change */
  if (s1 == s2) {

    /*** Updated 10/25/00 ***/
    setting = LinkSetting[k]; // Link[k].Kc;

    switch (Link[k].Type) {
      case EN_PRV:
      case EN_PSV:
      case EN_PBV:
        setting *= Ucf[PRESSURE];
        break;
      case EN_FCV:
        setting *= Ucf[FLOW];
      default:
        break;
    }
    sprintf(pr->Msg, FMT56, LinkTxt[Link[k].Type], Link[k].ID, setting);
    writeline(pr, pr->Msg);
    return;
  }

  /* We have a status change. Write the old & new status types. */
  if (s1 == ACTIVE)
    j1 = ACTIVE;
  else if (s1 <= CLOSED)
    j1 = CLOSED;
  else
    j1 = OPEN;
  if (s2 == ACTIVE)
    j2 = ACTIVE;
  else if (s2 <= CLOSED)
    j2 = CLOSED;
  else
    j2 = OPEN;
  if (j1 != j2) {
    sprintf(pr->Msg, FMT57, LinkTxt[Link[k].Type], Link[k].ID, StatTxt[j1],
            StatTxt[j2]);
    writeline(pr, pr->Msg);
  }
} /* End of writestatchange */

void writecontrolaction(EN_Project *pr, int k, int i)
/*
----------------------------------------------------------------
**   Input:   k  = link index
**            i  = control index
**   Output:  none
**   Purpose: writes control action taken to status report
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  Snode *Node = net->Node;
  Slink *Link = net->Link;
  Scontrol *Control = net->Control;
  
  int n;
  switch (Control[i].Type) {
  case LOWLEVEL:
  case HILEVEL:
    n = Control[i].Node;
    sprintf(pr->Msg, FMT54, clocktime(rep->Atime, time->Htime), LinkTxt[Link[k].Type],
            Link[k].ID, NodeTxt[getnodetype(net,n)], Node[n].ID);
    break;
  case TIMER:
  case TIMEOFDAY:
    sprintf(pr->Msg, FMT55, clocktime(rep->Atime, time->Htime), LinkTxt[Link[k].Type], Link[k].ID);
    break;
  default:
    return;
  }
  writeline(pr, pr->Msg);
}

void writeruleaction(EN_Project *pr, int k, char *ruleID)
/*
**--------------------------------------------------------------
**   Input:   k  = link index
**            *ruleID  = rule ID
**   Output:  none
**   Purpose: writes rule action taken to status report
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  Slink *Link = net->Link;

  sprintf(pr->Msg, FMT63, clocktime(rep->Atime, time->Htime), LinkTxt[Link[k].Type],
          Link[k].ID, ruleID);
  writeline(pr, pr->Msg);
}

int writehydwarn(EN_Project *pr, int iter, double relerr)
/*
**--------------------------------------------------------------
**   Input:   iter = # iterations to find hydraulic solution
**   Output:  warning flag code
**   Purpose: writes hydraulic warning message to report file
**
**   Note: Warning conditions checked in following order:
**         1. System balanced but unstable
**         2. Negative pressures
**         3. FCV cannot supply flow or PRV/PSV cannot maintain pressure
**         4. Pump out of range
**         5. Network disconnected
**         6. System unbalanced
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  
  Snode *Node = net->Node;
  Slink *Link = net->Link;
  Spump *Pump = net->Pump;
  Svalve *Valve = net->Valve;
  
  const int Njuncs = net->Njuncs;
  double *NodeDemand = hyd->NodeDemand;
  double *LinkFlows = hyd->LinkFlows;
  double *LinkSetting = hyd->LinkSetting;

  int i, j;
  char flag = 0;
  char s; 

  /* Check if system unstable */
  if (iter > hyd->MaxIter && relerr <= hyd->Hacc) {
    sprintf(pr->Msg, WARN02, clocktime(rep->Atime, time->Htime));
    if (rep->Messageflag)
      writeline(pr, pr->Msg);
    flag = 2;
  }

  /* Check for negative pressures */
  for (i = 1; i <= Njuncs; i++) {
    Snode *node = &Node[i];
    if (hyd->NodeHead[i] < node->El && NodeDemand[i] > 0.0) {
      sprintf(pr->Msg, WARN06, clocktime(rep->Atime, time->Htime));
      if (rep->Messageflag) {
        writeline(pr, pr->Msg);
      }
      flag = 6;
      break;
    }
  }

  /* Check for abnormal valve condition */
  for (i = 1; i <= net->Nvalves; i++) {
    j = Valve[i].Link;
    if (hyd->LinkStatus[j] >= XFCV) {
      sprintf(pr->Msg, WARN05, LinkTxt[Link[j].Type], Link[j].ID,
              StatTxt[hyd->LinkStatus[j]], clocktime(rep->Atime, time->Htime));
      if (rep->Messageflag)
        writeline(pr, pr->Msg);
      flag = 5;
    }
  }

  /* Check for abnormal pump condition */
  for (i = 1; i <= net->Npumps; i++) {
    j = Pump[i].Link;
    s = hyd->LinkStatus[j];         
    if (hyd->LinkStatus[j] >= OPEN) 
    {                               
      if (LinkFlows[j] > LinkSetting[j] * Pump[i].Qmax)
        s = XFLOW; 
      if (LinkFlows[j] < 0.0)
        s = XHEAD;                
    }                             
    if (s == XHEAD || s == XFLOW) 
    {
      sprintf(pr->Msg, WARN04, Link[j].ID, StatTxt[s], 
              clocktime(rep->Atime, time->Htime));
      if (rep->Messageflag)
        writeline(pr, pr->Msg);
      flag = 4;
    }
  }

  /* Check if system is unbalanced */
  if (iter > hyd->MaxIter && relerr > hyd->Hacc) {
    sprintf(pr->Msg, WARN01, clocktime(rep->Atime, time->Htime));
    if (hyd->ExtraIter == -1)
      strcat(pr->Msg, t_HALTED);
    if (rep->Messageflag)
      writeline(pr, pr->Msg);
    flag = 1;
  }

  /* Check for disconnected network */
  /* & update global warning flag   */
  if (flag > 0) {
    disconnected(pr);
    pr->Warnflag = flag;
  }
  return (flag);
} /* End of writehydwarn */

void writehyderr(EN_Project *pr, int errnode)
/*
**-----------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: outputs status & checks connectivity when
**            network hydraulic equations cannot be solved.
**-----------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  Snode *Node = net->Node;
  
  sprintf(pr->Msg, FMT62, clocktime(rep->Atime, time->Htime), Node[errnode].ID);
  if (rep->Messageflag)
    writeline(pr, pr->Msg);
  writehydstat(pr, 0, 0);
  disconnected(pr);
} /* End of writehyderr */

int disconnected(EN_Project *pr)
/*
**-------------------------------------------------------------------
**   Input:   None
**   Output:  Returns number of disconnected nodes
**   Purpose: Tests current hydraulic solution to see if any closed
**            links have caused the network to become disconnected.
**-------------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  
  int i, j;
  int count, mcount;
  int errcode = 0;
  int *nodelist;
  char *marked;

  /* Allocate memory for node list & marked list */
  nodelist = (int *)calloc(net->Nnodes + 1, sizeof(int));
  marked = (char *)calloc(net->Nnodes + 1, sizeof(char));
  ERRCODE(MEMCHECK(nodelist));
  ERRCODE(MEMCHECK(marked));
  if (errcode)
    return (0);

  /* Place tanks on node list and marked list */
  for (i = 1; i <= net->Ntanks; i++) {
    j = net->Njuncs + i;
    nodelist[i] = j;
    marked[j] = 1;
  }

  /* Place junctions with negative demands on the lists */
  mcount = net->Ntanks;
  for (i = 1; i <= net->Njuncs; i++) {
    if (hyd->NodeDemand[i] < 0.0) {
      mcount++;
      nodelist[mcount] = i;
      marked[i] = 1;
    }
  }

  /* Mark all nodes that can be connected to tanks */
  /* and count number of nodes remaining unmarked. */
  marknodes(pr, mcount, nodelist, marked);
  j = 0;
  count = 0;
  for (i = 1; i <= net->Njuncs; i++) {
    Snode *node = &net->Node[i];
    if (!marked[i] && hyd->NodeDemand[i] != 0.0) {
      count++;
      if (count <= MAXCOUNT && rep->Messageflag) {
        sprintf(pr->Msg, WARN03a, node->ID, clocktime(rep->Atime, time->Htime));
        writeline(pr, pr->Msg);
      }
      j = i; /* Last unmarked node */
    }
  }

  /* Report number of unmarked nodes and find closed link */
  /* on path from node j back to a tank.                  */
  if (count > 0 && rep->Messageflag) {
    if (count > MAXCOUNT) {
      sprintf(pr->Msg, WARN03b, count - MAXCOUNT, clocktime(rep->Atime, time->Htime));
      writeline(pr, pr->Msg);
    }
    getclosedlink(pr, j, marked);
  }

  /* Free allocated memory */
  free(nodelist);
  free(marked);
  return (count);
} /* End of disconnected() */

void marknodes(EN_Project *pr, int m, int *nodelist, char *marked)
/*
**----------------------------------------------------------------
**   Input:   m = number of source nodes
**            nodelist[] = list of nodes to be traced from
**            marked[]   = TRUE if node connected to source
**   Output:  None.
**   Purpose: Marks all junction nodes connected to tanks.
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  
  int i, j, k, n;
  Padjlist alink;

  /* Scan each successive entry of node list */
  n = 1;
  while (n <= m) {

    /* Scan all nodes connected to current node */
    i = nodelist[n];
    for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next) {

      /* Get indexes of connecting link and node */
      k = alink->link;
      j = alink->node;
      if (marked[j]) {
        continue;
      }

      /* Check if valve connection is in correct direction */
      switch (net->Link[k].Type) {
        case EN_CVPIPE:
        case EN_PRV:
        case EN_PSV:
          if (j == net->Link[k].N1) {
            continue;
          }
        default:
          break;
      }

      /* Mark connection node if link not closed */
      if (hyd->LinkStatus[k] > CLOSED) {
        marked[j] = 1;
        m++;
        nodelist[m] = j;
      }
    }
    n++;
  }
} /* End of marknodes() */

void getclosedlink(EN_Project *pr, int i, char *marked)
/*
**----------------------------------------------------------------
**   Input:   i = junction index
**            marked[] = marks nodes already examined
**   Output:  None.
**   Purpose: Determines if a closed link connects to junction i.
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;

  int j, k;
  Padjlist alink;
  marked[i] = 2;
  for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next) {
    k = alink->link;
    j = alink->node;
    if (marked[j] == 2)
      continue;
    if (marked[j] == 1) {
      sprintf(pr->Msg, WARN03c, net->Link[k].ID);
      writeline(pr, pr->Msg);
      return;
    } else
      getclosedlink(pr, j, marked);
  }
}

void writelimits(EN_Project *pr, int j1, int j2)
/*
**--------------------------------------------------------------
**   Input:   j1 = index of first output variable
**            j2 = index of last output variable
**   Output:  none
**   Purpose: writes reporting criteria to output report
**--------------------------------------------------------------
*/
{
  report_options_t *rep = &pr->report;
  
  int j;
  for (j = j1; j <= j2; j++) {
    if (rep->Field[j].RptLim[LOW] < BIG) {
      sprintf(pr->Msg, FMT47, rep->Field[j].Name, rep->Field[j].RptLim[LOW],
              rep->Field[j].Units);
      writeline(pr, pr->Msg);
    }
    if (rep->Field[j].RptLim[HI] > -BIG) {
      sprintf(pr->Msg, FMT48, rep->Field[j].Name, rep->Field[j].RptLim[HI],
              rep->Field[j].Units);
      writeline(pr, pr->Msg);
    }
  }
} /* End of writelimits */

int checklimits(report_options_t *rep, double *y, int j1, int j2)
/*
**--------------------------------------------------------------
**   Input:   *y = array of output results
**            j1 = index of first output variable
**            j2 = index of last output variable
**   Output:  returns 1 if criteria met, 0 otherwise
**   Purpose: checks if output reporting criteria is met
**--------------------------------------------------------------
*/
{
  int j;
  for (j = j1; j <= j2; j++) {
    if (y[j] > rep->Field[j].RptLim[LOW] || y[j] < rep->Field[j].RptLim[HI])
      return (0);
  }
  return (1);
} /* End of checklim */

void writetime(EN_Project *pr, char *fmt)
/*
**----------------------------------------------------------------
**   Input:   fmt = format string
**   Output:  none
**   Purpose: writes starting/ending time of a run to report file
**----------------------------------------------------------------
*/
{
  time_t timer;
  time(&timer);
  sprintf(pr->Msg, fmt, ctime(&timer));
  writeline(pr, pr->Msg);
}

char *clocktime(char *atime, long seconds)
/*
**--------------------------------------------------------------
**   Input:   seconds = time in seconds
**   Output:  atime = time in hrs:min
**            (returns pointer to atime)
**   Purpose: converts time in seconds to hours:minutes format
**--------------------------------------------------------------
*/
{
  /*** Updated 6/24/02 ***/
  long h, m, s;
  h = seconds / 3600;
  m = seconds % 3600 / 60;
  s = seconds - 3600 * h - 60 * m;
  sprintf(atime, "%01d:%02d:%02d", (int)h, (int)m, (int)s);
  return (atime);
} /* End of clocktime */

char *fillstr(char *s, char ch, int n)
/*
**---------------------------------------------------------
**  Fills n bytes of s to character ch.
**  NOTE: does not check for overwriting s.
**---------------------------------------------------------
*/
{
  int i;
  for (i = 0; i <= n; i++)
    s[i] = ch;
  s[n + 1] = '\0';
  return (s);
}

int getnodetype(EN_Network *net, int i)
/*
**---------------------------------------------------------
**  Determines type of node with index i
**  (junction = 0, reservoir = 1, tank = 2).
**---------------------------------------------------------
*/
{
  if (i <= net->Njuncs)
    return (0);
  if (net->Tank[i - net->Njuncs].A == 0.0)
    return (1);
  return (2);
}

/********************* END OF REPORT.C ********************/
