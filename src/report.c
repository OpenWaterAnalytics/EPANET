/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       report.c
 Description:  procedures for writing formatted text to a report file
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 07/22/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

#include <math.h>
#include <time.h>

#include "types.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"

#define MAXCOUNT 10    // Max. # of disconnected nodes listed

// Defined in ENUMSTXT.H
extern char *NodeTxt[];
extern char *LinkTxt[];
extern char *StatTxt[];
extern char *TstatTxt[];
extern char *RptFormTxt[];
extern char *DemandModelTxt[];

// Local functions
typedef REAL4 *Pfloat;
static void writenodetable(Project *, Pfloat *);
static void writelinktable(Project *, Pfloat *);
static void writeenergy(Project *);
static int  writeresults(Project *);
static int  disconnected(Project *);
static void marknodes(Project *, int, int *, char *);
static void getclosedlink(Project *, int, char *);
static void writelimits(Project *, int, int);
static int  checklimits(Report *, double *, int, int);
static char *fillstr(char *, char, int);
static int  getnodetype(Network *, int);

int clearreport(Project *pr)
/*
**------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: clears contents of a project's report file
**------------------------------------------------------
*/
{
    Report *rpt = &pr->report;
    if (rpt->RptFile == NULL) return 0;
    if (freopen(rpt->Rpt1Fname, "w", rpt->RptFile) == NULL) return 303;
    writelogo(pr);
    return 0;
}

int copyreport(Project* pr, char *filename)
/*
**------------------------------------------------------
**   Input:   filename = name of file to copy to
**   Output:  returns error code
**   Purpose: copies contents of a project's report file
**------------------------------------------------------
*/
{
    FILE *tfile;
    int c;
    Report *rpt = &pr->report;

    // Check that project's report file exists
    if (rpt->RptFile == NULL) return 0;

    // Open the new destination file
    tfile = fopen(filename, "w");
    if (tfile == NULL) return 303;

    // Re-open project's report file in read mode
    fclose(rpt->RptFile);
    rpt->RptFile = fopen(rpt->Rpt1Fname, "r");

    // Copy contents of project's report file
    if (rpt->RptFile)
    {
        while ((c = fgetc(rpt->RptFile)) != EOF) fputc(c, tfile);
        fclose(rpt->RptFile);
    }

    // Close destination file
    fclose(tfile);

    // Re-open project's report file in append mode
    rpt->RptFile = fopen(rpt->Rpt1Fname, "a");
    if (rpt->RptFile == NULL) return 303;
    return 0;
}

int writereport(Project *pr)
/*
**------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes formatted output report to file
**------------------------------------------------------
*/
{
    Report *rpt = &pr->report;
    Parser *parser = &pr->parser;

    int tflag;
    FILE *tfile;
    int errcode = 0;

    // If no secondary report file specified then
    // write formatted output to primary report file
    rpt->Fprinterr = FALSE;
    if (rpt->Rptflag && strlen(rpt->Rpt2Fname) == 0 && rpt->RptFile != NULL)
    {
        if (rpt->Energyflag) writeenergy(pr);
        errcode = writeresults(pr);
    }

    // A secondary report file was specified
    else if (strlen(rpt->Rpt2Fname) > 0)
    {
        // If secondary report file has same name as either input
        // or primary report file then use primary report file.
        if (strcomp(rpt->Rpt2Fname, parser->InpFname) ||
            strcomp(rpt->Rpt2Fname, rpt->Rpt1Fname))
        {
            if (rpt->Energyflag) writeenergy(pr);
            errcode = writeresults(pr);
        }

        // Otherwise write report to secondary report file
        else
        {
            // Try to open file
            tfile = rpt->RptFile;
            tflag = rpt->Rptflag;
            if ((rpt->RptFile = fopen(rpt->Rpt2Fname, "wt")) == NULL)
            {
                rpt->RptFile = tfile;
                rpt->Rptflag = tflag;
                errcode = 303;
            }

            // Write full formatted report to file
            else
            {
                rpt->Rptflag = 1;
                writelogo(pr);
                if (rpt->Summaryflag) writesummary(pr);
                if (rpt->Energyflag)  writeenergy(pr);
                errcode = writeresults(pr);
                fclose(rpt->RptFile);
                rpt->RptFile = tfile;
                rpt->Rptflag = tflag;
            }
        }
    }

    // Special error handler for write-to-file error
    if (rpt->Fprinterr) errmsg(pr, 309);
    return errcode;
}

void writelogo(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes program logo to report file.
**--------------------------------------------------------------
*/
{
    Report *rpt = &pr->report;

    int version;
    int major;
    int minor;
    char s[80];
    time_t timer; // time_t structure & functions time() &
                  // ctime() are defined in time.h

    version = CODEVERSION;
    major = version / 10000;
    minor = (version % 10000) / 100;

    time(&timer);
    strcpy(rpt->DateStamp, ctime(&timer));
    rpt->PageNum = 1;
    rpt->LineNum = 2;
    fprintf(rpt->RptFile, FMT18);
    fprintf(rpt->RptFile, "%s", rpt->DateStamp);
    writeline(pr, LOGO1);
    writeline(pr, LOGO2);
    writeline(pr, LOGO3);
    writeline(pr, LOGO4);
    sprintf(s, LOGO5, major, minor);
    writeline(pr, s);
    writeline(pr, LOGO6);
    writeline(pr, "");
}

void writesummary(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes summary system information to report file
**--------------------------------------------------------------
*/
{
  Network *net = &pr->network;
  Hydraul *hyd = &pr->hydraul;
  Quality *qual = &pr->quality;
  Report  *rpt = &pr->report;
  Parser  *parser = &pr->parser;
  Times   *time = &pr->times;

  char s[MAXLINE + 1];
  int i;
  int nres = 0;

  for (i = 0; i < 3; i++)
  {
    if (strlen(pr->Title[i]) > 0)
    {
      sprintf(s, "%-.70s", pr->Title[i]);
      writeline(pr, s);
    }
  }
  writeline(pr, " ");
  sprintf(s, FMT19, parser->InpFname);
  writeline(pr, s);
  sprintf(s, FMT20, net->Njuncs);
  writeline(pr, s);
  for (i = 1; i <= net->Ntanks; i++) if (net->Tank[i].A == 0.0) nres++;
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
  sprintf(s, FMT26, time->Hstep * pr->Ucf[TIME], rpt->Field[TIME].Units);
  writeline(pr, s);
  sprintf(s, FMT27, hyd->Hacc);
  writeline(pr, s);

  if (hyd->HeadErrorLimit > 0.0)
  {
    sprintf(s, FMT27d, hyd->HeadErrorLimit*pr->Ucf[HEAD], rpt->Field[HEAD].Units);
    writeline(pr, s);
  }
  if (hyd->FlowChangeLimit > 0.0)
  {
    sprintf(s, FMT27e, hyd->FlowChangeLimit*pr->Ucf[FLOW], rpt->Field[FLOW].Units);
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

  if (qual->Qualflag == NONE || time->Dur == 0.0) sprintf(s, FMT29);
  else if (qual->Qualflag == CHEM)  sprintf(s, FMT30, qual->ChemName);
  else if (qual->Qualflag == TRACE) sprintf(s, FMT31, net->Node[qual->TraceNode].ID);
  else if (qual->Qualflag == AGE)   printf(s, FMT32);
  writeline(pr, s);
  if (qual->Qualflag != NONE && time->Dur > 0)
  {
    sprintf(s, FMT33, (float)time->Qstep / 60.0);
    writeline(pr, s);
    sprintf(s, FMT34, qual->Ctol * pr->Ucf[QUALITY], rpt->Field[QUALITY].Units);
    writeline(pr, s);
  }

  sprintf(s, FMT36, hyd->SpGrav);
  writeline(pr, s);
  sprintf(s, FMT37a, hyd->Viscos / VISCOS);
  writeline(pr, s);
  sprintf(s, FMT37b, qual->Diffus / DIFFUS);
  writeline(pr, s);
  sprintf(s, FMT38, hyd->Dmult);
  writeline(pr, s);
  sprintf(s, FMT39, time->Dur * pr->Ucf[TIME], rpt->Field[TIME].Units);
  writeline(pr, s);

  if (rpt->Rptflag)
  {
    sprintf(s, FMT40);
    writeline(pr, s);
    if (rpt->Nodeflag == 0)  writeline(pr, FMT41);
    if (rpt->Nodeflag == 1)  writeline(pr, FMT42);
    if (rpt->Nodeflag == 2)  writeline(pr, FMT43);
    writelimits(pr, DEMAND, QUALITY);
    if (rpt->Linkflag == 0)  writeline(pr, FMT44);
    if (rpt->Linkflag == 1)  writeline(pr, FMT45);
    if (rpt->Linkflag == 2)  writeline(pr, FMT46);
    writelimits(pr, DIAM, HEADLOSS);
  }
  writeline(pr, " ");
}

void writehydstat(Project *pr, int iter, double relerr)
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
  Network *net = &pr->network;
  Hydraul *hyd = &pr->hydraul;
  Report  *rpt = &pr->report;
  Times   *time = &pr->times;

  int i, n;
  double *NodeDemand;
  char s1[MAXLINE + 1];
  char atime[13];
  StatusType newstat;
  Stank *Tank = net->Tank;
  Slink *Link = net->Link;

  // Display system status
  strcpy(atime, clocktime(rpt->Atime, time->Htime));
  if (iter > 0)
  {
    if (relerr <= hyd->Hacc) sprintf(s1, FMT58, atime, iter);
    else sprintf(s1, FMT59, atime, iter, relerr);
    writeline(pr, s1); 
    if (hyd->DemandModel == PDA && hyd->DeficientNodes > 0)
    {
        if (hyd->DeficientNodes == 1)
          sprintf(s1, FMT69a, hyd->DemandReduction);
        else
          sprintf(s1, FMT69b, hyd->DeficientNodes, hyd->DemandReduction);
        writeline(pr, s1);        
    }    
  }

  // Display status changes for tanks:
  //   D[n] is net inflow to tank at node n;
  //   old tank status is stored in OldStatus[]
  //   at indexes Nlinks+1 to Nlinks+Ntanks.
  for (i = 1; i <= net->Ntanks; i++)
  {
    n = net->Tank[i].Node;
    NodeDemand = hyd->NodeDemand;
    if (ABS(NodeDemand[n]) < 0.001) newstat = CLOSED;
    else if (NodeDemand[n] < 0.0)   newstat = EMPTYING;
    else if (NodeDemand[n] > 0.0)
    {
        if (Tank[i].A > 0.0 && ABS(hyd->NodeHead[n] - Tank[i].Hmax) < 0.001)
            newstat = OVERFLOWING;
        else newstat = FILLING;
    }
    else newstat = hyd->OldStatus[net->Nlinks + i];
    if (newstat != hyd->OldStatus[net->Nlinks + i])
    {
      if (Tank[i].A > 0.0)
      {
        snprintf(s1, MAXLINE, FMT50, atime, net->Node[n].ID, StatTxt[newstat],
                 (hyd->NodeHead[n] - net->Node[n].El) * pr->Ucf[HEAD],
                 rpt->Field[HEAD].Units);
      }
      else
      {
        snprintf(s1, MAXLINE, FMT51, atime, net->Node[n].ID, StatTxt[newstat]);
      }
      writeline(pr, s1);
      hyd->OldStatus[net->Nlinks + i] = newstat;
    }
  }

  // Display status changes for links
  for (i = 1; i <= net->Nlinks; i++)
  {
    if (hyd->LinkStatus[i] != hyd->OldStatus[i])
    {
      if (time->Htime == 0)
      {
        sprintf(s1, FMT52, atime, LinkTxt[(int)net->Link[i].Type],
                net->Link[i].ID, StatTxt[(int)hyd->LinkStatus[i]]);
      }
      else sprintf(s1, FMT53, atime, LinkTxt[Link[i].Type], net->Link[i].ID,
                   StatTxt[hyd->OldStatus[i]], StatTxt[hyd->LinkStatus[i]]);
      writeline(pr, s1);
      hyd->OldStatus[i] = hyd->LinkStatus[i];
    }
  }
  writeline(pr, " ");
}

void writemassbalance(Project *pr)
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
    Quality *qual = &pr->quality;

    char s1[MAXMSG+1];
    char *units[] = {"", " (mg)", " (ug)", " (hrs)"};
    int  kunits = 0;

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
    snprintf(s1, MAXMSG, "Initial Mass:      %12.5e", qual->MassBalance.initial);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Inflow:       %12.5e", qual->MassBalance.inflow);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Outflow:      %12.5e", qual->MassBalance.outflow);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Reacted:      %12.5e", qual->MassBalance.reacted);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Final Mass:        %12.5e", qual->MassBalance.final);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "Mass Ratio:         %-.5f", qual->MassBalance.ratio);
    writeline(pr, s1);
    snprintf(s1, MAXMSG, "================================\n");
    writeline(pr, s1);
}

void writeenergy(Project *pr)
/*
**-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: writes energy usage report to report file
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;

    int j;
    double csum;
    char s[MAXLINE + 1];
    Spump *pump;

    if (net->Npumps == 0) return;
    writeline(pr, " ");
    writeheader(pr,ENERHDR, 0);

    csum = 0.0;
    for (j = 1; j <= net->Npumps; j++)
    {
        pump = &net->Pump[j];
        csum += pump->Energy.TotalCost;
        if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, ENERHDR, 1);

        sprintf(s, "%-8s  %6.2f %6.2f %9.2f %9.2f %9.2f %9.2f",
            net->Link[pump->Link].ID, pump->Energy.TimeOnLine,
            pump->Energy.Efficiency,  pump->Energy.KwHrsPerFlow,
            pump->Energy.KwHrs,       pump->Energy.MaxKwatts,
            pump->Energy.TotalCost);
        writeline(pr, s);
    }

    fillstr(s, '-', 63);
    writeline(pr, s);
    sprintf(s, FMT74, "", hyd->Emax * hyd->Dcost);
    writeline(pr, s);
    sprintf(s, FMT75, "", csum + hyd->Emax * hyd->Dcost);
    writeline(pr, s);
    writeline(pr, " ");
}

int writeresults(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes simulation results to report file
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Outfile *out = &pr->outfile;
    Report  *rpt = &pr->report;
    Times   *time = &pr->times;

    int j, m, n,
        np,           // Reporting period counter
        nnv,          // # node variables reported on
        nlv;          // # link variables reported on
    int errcode = 0;
    Pfloat *x;        // Array of pointers to floats (i.e., a 2-D array)
    FILE *outFile = out->OutFile;

  //-----------------------------------------------------------
  //  NOTE:  The OutFile contains results for 4 node variables
  //         (demand, head, pressure, & quality) and 8 link
  //         variables (flow, velocity, headloss, quality,
  //         status, setting, reaction rate & friction factor)
  //         at each reporting time.
  //-----------------------------------------------------------

    // Return if no nodes or links selected for reporting
    // or if no node or link report variables enabled
    if (!rpt->Nodeflag && !rpt->Linkflag)  return errcode;

    nnv = 0;
    for (j = ELEV; j <= QUALITY; j++) nnv += rpt->Field[j].Enabled;
    nlv = 0;
    for (j = LENGTH; j <= FRICTION; j++) nlv += rpt->Field[j].Enabled;
    if (nnv == 0 && nlv == 0) return errcode;

    // Return if no output file
    if (outFile == NULL) outFile = fopen(pr->outfile.OutFname, "rb");
    if (outFile == NULL) return 106;

    // Allocate memory for output variables:
    // m = larger of # node variables & # link variables
    // n = larger of # nodes & # links
    m = MAX((QUALITY - DEMAND + 1), (FRICTION - FLOW + 1));
    n = MAX((net->Nnodes + 1), (net->Nlinks + 1));
    x = (Pfloat *)calloc(m, sizeof(Pfloat));
    ERRCODE(MEMCHECK(x));
    if (errcode) return errcode;
    for (j = 0; j < m; j++)
    {
        x[j] = (REAL4 *)calloc(n, sizeof(REAL4));
        if (x[j] == NULL) errcode = 101;
    }
    if (!errcode)
    {
        // Re-position output file & initialize report time
        fseek(outFile, out->OutOffset2, SEEK_SET);
        time->Htime = time->Rstart;

        // For each reporting time:
        for (np = 1; np <= rpt->Nperiods; np++)
        {
            // Read in node results & write node table
            // (Remember to offset x[j] by 1 because array is zero-based)
            for (j = DEMAND; j <= QUALITY; j++)
            {
                fread((x[j - DEMAND]) + 1, sizeof(REAL4), net->Nnodes, outFile);
            }
            if (nnv > 0 && rpt->Nodeflag > 0) writenodetable(pr, x);

            // Read in link results & write link table
            for (j = FLOW; j <= FRICTION; j++)
            {
                fread((x[j - FLOW]) + 1, sizeof(REAL4), net->Nlinks, outFile);
            }
            if (nlv > 0 && rpt->Linkflag > 0) writelinktable(pr, x);
            time->Htime += time->Rstep;
        }
    }

    // Free output file
    if (outFile != NULL)
    {
        fclose(outFile);
        outFile = NULL;
    }

    // Free allocated memory
    for (j = 0; j < m; j++) free(x[j]);
    free(x);
    return errcode;
}

void writenodetable(Project *pr, Pfloat *x)
/*
**---------------------------------------------------------------
**   Input:   x = pointer to node results for current time
**   Output:  none
**   Purpose: writes node results for current time to report file
**---------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Report  *rpt = &pr->report;

    int i, j;
    char s[MAXLINE + 1], s1[16];
    double y[MAXVAR];
    Snode *node;

    // Write table header
    writeheader(pr, NODEHDR, 0);

    // For each node:
    for (i = 1; i <= net->Nnodes; i++)
    {
        // Place node's results for each variable in y
        node = &net->Node[i];
        y[ELEV] = node->El * pr->Ucf[ELEV];
        for (j = DEMAND; j <= QUALITY; j++) y[j] = *((x[j - DEMAND]) + i);

        // Check if node gets reported on
        if ((rpt->Nodeflag == 1 || node->Rpt) &&
             checklimits(rpt, y, ELEV, QUALITY))
        {
            // Check if new page needed
            if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, NODEHDR, 1);

            // Add node ID and each reported field to string s
            sprintf(s, "%-15s", node->ID);
            for (j = ELEV; j <= QUALITY; j++)
            {
                if (rpt->Field[j].Enabled == TRUE)
                {
                    if (fabs(y[j]) > 1.e6) sprintf(s1, "%10.2e", y[j]);
                    else sprintf(s1, "%10.*f", rpt->Field[j].Precision, y[j]);
                    strcat(s, s1);
                }
            }

            // Note if node is a reservoir/tank
            if (i > net->Njuncs)
            {
                strcat(s, "  ");
                strcat(s, NodeTxt[getnodetype(net, i)]);
            }

            // Write results for node to report file
            writeline(pr, s);
        }
    }
    writeline(pr, " ");
}

void writelinktable(Project *pr, Pfloat *x)
/*
**---------------------------------------------------------------
**   Input:   x = pointer to link results for current time
**   Output:  none
**   Purpose: writes link results for current time to report file
**---------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Report  *rpt = &pr->report;

    int i, j, k;
    char s[MAXLINE + 1], s1[16];
    double y[MAXVAR];
    double *Ucf = pr->Ucf;
    Slink *Link = net->Link;

    // Write table header
    writeheader(pr, LINKHDR, 0);

    // For each link:
    for (i = 1; i <= net->Nlinks; i++)
    {
        // Place results for each link variable in y
        y[LENGTH] = Link[i].Len * Ucf[LENGTH];
        y[DIAM] = Link[i].Diam * Ucf[DIAM];
        for (j = FLOW; j <= FRICTION; j++) y[j] = *((x[j - FLOW]) + i);

        // Check if link gets reported on
        if ((rpt->Linkflag == 1 || Link[i].Rpt) && checklimits(rpt, y, DIAM, FRICTION))
        {
            // Check if new page needed
            if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, LINKHDR, 1);

            // Add link ID and each reported field to string s
            sprintf(s, "%-15s", Link[i].ID);
            for (j = LENGTH; j <= FRICTION; j++)
            {
                if (rpt->Field[j].Enabled == TRUE)
                {
                    if (j == STATUS)
                    {
                        if      (y[j] <= CLOSED) k = CLOSED;
                        else if (y[j] == ACTIVE) k = ACTIVE;
                        else                     k = OPEN;
                        sprintf(s1, "%10s", StatTxt[k]);
                    }
                    else
                    {
                        if (fabs(y[j]) > 1.e6) sprintf(s1, "%10.2e", y[j]);
                        else sprintf(s1, "%10.*f", rpt->Field[j].Precision, y[j]);
                    }
                    strcat(s, s1);
                }
            }

            // Note if link is a pump or valve
            if ((j = Link[i].Type) > PIPE)
            {
                strcat(s, "  ");
                strcat(s, LinkTxt[j]);
            }

            // Write results for link
            writeline(pr, s);
        }
    }
    writeline(pr, " ");
}

void writeheader(Project *pr, int type, int contin)
/*
**--------------------------------------------------------------
**   Input:   type   = table type
**            contin = table continuation flag
**   Output:  none
**   Purpose: writes column headings for output report tables
**--------------------------------------------------------------
*/
{
    Report  *rpt = &pr->report;
    Quality *qual = &pr->quality;
    Parser  *parser = &pr->parser;
    Times   *time = &pr->times;

    char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1], s3[MAXLINE + 1];
    int i, n;

    // Move to next page if < 11 lines remain on current page
    if (rpt->Rptflag && rpt->LineNum + 11 > (long)rpt->PageSize)
    {
        while (rpt->LineNum < (long)rpt->PageSize) writeline(pr, " ");
    }
    writeline(pr, " ");

    // Hydraulic Status Table
    if (type == STATHDR)
    {
        sprintf(s, FMT49);
        if (contin) strcat(s, t_CONTINUED);
        writeline(pr, s);
        fillstr(s, '-', 70);
        writeline(pr, s);
    }

    // Energy Usage Table
    if (type == ENERHDR)
    {
        if (parser->Unitsflag == SI) strcpy(s1, t_perM3);
        else                         strcpy(s1, t_perMGAL);
        sprintf(s, FMT71);
        if (contin) strcat(s, t_CONTINUED);
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

    // Node Results Table
    if (type == NODEHDR)
    {
        if (rpt->Tstatflag == RANGE) sprintf(s, FMT76, t_DIFFER);
        else if (rpt->Tstatflag != SERIES)
        {
            sprintf(s, FMT76, TstatTxt[rpt->Tstatflag]);
        }
        else if (time->Dur == 0) sprintf(s, FMT77);
        else sprintf(s, FMT78, clocktime(rpt->Atime, time->Htime));
        if (contin) strcat(s, t_CONTINUED);
        writeline(pr, s);

        n = 15;
        sprintf(s2, "%15s", "");
        strcpy(s, t_NODEID);
        sprintf(s3, "%-15s", s);

        for (i = ELEV; i < QUALITY; i++)
        {
            if (rpt->Field[i].Enabled == TRUE)
            {
                n += 10;
                sprintf(s, "%10s", rpt->Field[i].Name);
                strcat(s2, s);
                sprintf(s, "%10s", rpt->Field[i].Units);
                strcat(s3, s);
            }
        }

        if (rpt->Field[QUALITY].Enabled == TRUE)
        {
            n += 10;
            sprintf(s, "%10s", qual->ChemName);
            strcat(s2, s);
            sprintf(s, "%10s", qual->ChemUnits);
            strcat(s3, s);
        }
        fillstr(s1, '-', n);
        writeline(pr, s1);
        writeline(pr, s2);
        writeline(pr, s3);
        writeline(pr, s1);
    }

    // Link Results Table
    if (type == LINKHDR)
    {
        if (rpt->Tstatflag == RANGE) sprintf(s, FMT79, t_DIFFER);
        else if (rpt->Tstatflag != SERIES)
        {
            sprintf(s, FMT79, TstatTxt[rpt->Tstatflag]);
        }
        else if (time->Dur == 0) sprintf(s, FMT80);
        else  sprintf(s, FMT81, clocktime(rpt->Atime, time->Htime));
        if (contin) strcat(s, t_CONTINUED);
        writeline(pr, s);

        n = 15;
        sprintf(s2, "%15s", "");
        strcpy(s, t_LINKID);
        sprintf(s3, "%-15s", s);
        for (i = LENGTH; i <= FRICTION; i++)
        {
            if (rpt->Field[i].Enabled == TRUE)
            {
                n += 10;
                sprintf(s, "%10s", rpt->Field[i].Name);
                strcat(s2, s);
                sprintf(s, "%10s", rpt->Field[i].Units);
                strcat(s3, s);
            }
        }
        fillstr(s1, '-', n);
        writeline(pr, s1);
        writeline(pr, s2);
        writeline(pr, s3);
        writeline(pr, s1);
    }
}

void writeline(Project *pr, char *s)
/*
**--------------------------------------------------------------
**   Input:   *s = text string
**   Output:  none
**   Purpose: writes a line of output to report file
**--------------------------------------------------------------
*/
{
    Report *rpt = &pr->report;

    if (rpt->RptFile == NULL) return;
    if (rpt->Rptflag)
    {
        if (rpt->LineNum == (long)rpt->PageSize)
        {
            rpt->PageNum++;
            if (fprintf(rpt->RptFile, FMT82, (int)rpt->PageNum, pr->Title[0]) < 0)
            {
                rpt->Fprinterr = TRUE;
            }
            rpt->LineNum = 3;
        }
    }
    if (fprintf(rpt->RptFile, "\n  %s", s) < 0) rpt->Fprinterr = TRUE;
    rpt->LineNum++;
}

void writerelerr(Project *pr, int iter, double relerr)
/*
**-----------------------------------------------------------------
**   Input:   iter   = current iteration of hydraulic solution
**            relerr = current convergence error
**   Output:  none
**   Purpose: writes out convergence status of hydraulic solution
**-----------------------------------------------------------------
*/
{
    Report *rpt = &pr->report;
    Times *time = &pr->times;

    if (iter == 0)
    {
        sprintf(pr->Msg, FMT64, clocktime(rpt->Atime, time->Htime));
        writeline(pr, pr->Msg);
    }
    else
    {
        sprintf(pr->Msg, FMT65, iter, relerr);
        writeline(pr, pr->Msg);
    }
}

void writestatchange(Project *pr, int k, char s1, char s2)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int j1, j2;
    double setting;
    double *Ucf = pr->Ucf;
    double *LinkSetting = hyd->LinkSetting;
    Slink *Link = net->Link;

    // We have a pump/valve setting change instead of a status change
    if (s1 == s2)
    {
        setting = LinkSetting[k];
        switch (Link[k].Type)
        {
          case PRV:
          case PSV:
          case PBV:
            setting *= Ucf[PRESSURE];
            break;
          case FCV:
            setting *= Ucf[FLOW];
            break;
          default:
            break;
        }
        sprintf(pr->Msg, FMT56, LinkTxt[Link[k].Type], Link[k].ID, setting);
        writeline(pr, pr->Msg);
        return;
    }

    // We have a status change  - write the old & new status types
    if      (s1 == ACTIVE) j1 = ACTIVE;
    else if (s1 <= CLOSED) j1 = CLOSED;
    else                   j1 = OPEN;
    if      (s2 == ACTIVE) j2 = ACTIVE;
    else if (s2 <= CLOSED) j2 = CLOSED;
    else                   j2 = OPEN;
    if (j1 != j2)
    {
        sprintf(pr->Msg, FMT57, LinkTxt[Link[k].Type], Link[k].ID, StatTxt[j1],
                StatTxt[j2]);
        writeline(pr, pr->Msg);
    }
}

void writecontrolaction(Project *pr, int k, int i)
/*
----------------------------------------------------------------
**   Input:   k  = link index
**            i  = control index
**   Output:  none
**   Purpose: writes control action taken to status report
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Report  *rpt = &pr->report;
    Times   *time = &pr->times;

    int n;
    Snode *Node = net->Node;
    Slink *Link = net->Link;
    Scontrol *Control = net->Control;

    switch (Control[i].Type)
    {
      case LOWLEVEL:
      case HILEVEL:
        n = Control[i].Node;
        sprintf(pr->Msg, FMT54, clocktime(rpt->Atime, time->Htime),
                LinkTxt[Link[k].Type], Link[k].ID,
                NodeTxt[getnodetype(net, n)], Node[n].ID);
        break;

      case TIMER:
      case TIMEOFDAY:
        sprintf(pr->Msg, FMT55, clocktime(rpt->Atime, time->Htime),
                LinkTxt[Link[k].Type], Link[k].ID);
        break;
      default:
        return;
    }
    writeline(pr, pr->Msg);
}

void writeruleaction(Project *pr, int k, char *ruleID)
/*
**--------------------------------------------------------------
**   Input:   k  = link index
**            *ruleID  = rule ID
**   Output:  none
**   Purpose: writes rule action taken to status report
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Report *rpt = &pr->report;
    Times  *time = &pr->times;

    Slink *Link = net->Link;

    sprintf(pr->Msg, FMT63, clocktime(rpt->Atime, time->Htime),
            LinkTxt[Link[k].Type], Link[k].ID, ruleID);
    writeline(pr, pr->Msg);
}

int writehydwarn(Project *pr, int iter, double relerr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;
    Times   *time = &pr->times;

    int i, j;
    char flag = 0;
    int s;
    Snode *node;
    Slink *link;
    Spump *pump;

    // Check if system unstable
    if (iter > hyd->MaxIter && relerr <= hyd->Hacc)
    {
        sprintf(pr->Msg, WARN02, clocktime(rpt->Atime, time->Htime));
        if (rpt->Messageflag) writeline(pr, pr->Msg);
        flag = 2;
    }

    // Check for pressure deficient nodes
    if (hyd->DemandModel == DDA)
    {
        hyd->DeficientNodes = 0;
        for (i = 1; i <= net->Njuncs; i++)
        {
            node = &net->Node[i];
            if (hyd->NodeHead[i] < node->El && hyd->NodeDemand[i] > 0.0)
                hyd->DeficientNodes++;
        }
        if (hyd->DeficientNodes > 0)
        {
            if (rpt->Messageflag)
            {
                sprintf(pr->Msg, WARN06, clocktime(rpt->Atime, time->Htime));
                writeline(pr, pr->Msg);
            }
            flag = 6;
        }
    }

    // Check for abnormal valve condition
    for (i = 1; i <= net->Nvalves; i++)
    {
        j = net->Valve[i].Link;
        link = &net->Link[j];
        if (hyd->LinkStatus[j] >= XFCV)
        {
            if (rpt->Messageflag)
            {
                sprintf(pr->Msg, WARN05, LinkTxt[link->Type], link->ID,
                        StatTxt[hyd->LinkStatus[j]],
                        clocktime(rpt->Atime, time->Htime));
                writeline(pr, pr->Msg);
            }
            flag = 5;
        }
    }

    // Check for abnormal pump condition
    for (i = 1; i <= net->Npumps; i++)
    {
        pump = &net->Pump[i];
        j = pump->Link;
        s = hyd->LinkStatus[j];
        if (hyd->LinkStatus[j] >= OPEN)
        {
            if (hyd->LinkFlow[j] > hyd->LinkSetting[j] * pump->Qmax) s = XFLOW;
            if (hyd->LinkFlow[j] < 0.0) s = XHEAD;
        }
        if (s == XHEAD || s == XFLOW)
        {
            if (rpt->Messageflag)
            {
                sprintf(pr->Msg, WARN04, net->Link[j].ID, StatTxt[s],
                        clocktime(rpt->Atime, time->Htime));
                writeline(pr, pr->Msg);
            }
            flag = 4;
        }
    }

    // Check if system is unbalanced
    if (iter > hyd->MaxIter && relerr > hyd->Hacc)
    {
        if (rpt->Messageflag)
        {
            sprintf(pr->Msg, WARN01, clocktime(rpt->Atime, time->Htime));
            if (hyd->ExtraIter == -1) strcat(pr->Msg, t_HALTED);
            writeline(pr, pr->Msg);
        }
        flag = 1;
    }

    // Check for disconnected network & update project's warning flag
    if (flag > 0)
    {
        disconnected(pr);
        pr->Warnflag = flag;
        if (rpt->Messageflag) writeline(pr, " ");
    }
    return flag;
}

void writehyderr(Project *pr, int errnode)
/*
**-----------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: outputs status & checks connectivity when
**            network hydraulic equations cannot be solved.
**-----------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Report  *rpt = &pr->report;
    Times   *time = &pr->times;

    Snode *Node = net->Node;

    if (rpt->Messageflag)
    {
        sprintf(pr->Msg, FMT62, clocktime(rpt->Atime, time->Htime),
                Node[errnode].ID);
        writeline(pr, pr->Msg);
    }
    writehydstat(pr, 0, 0);
    disconnected(pr);
}

int disconnected(Project *pr)
/*
**-------------------------------------------------------------------
**   Input:   None
**   Output:  Returns number of disconnected nodes
**   Purpose: Tests current hydraulic solution to see if any closed
**            links have caused the network to become disconnected.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;
    Times   *time = &pr->times;

    int i, j;
    int count, mcount;
    int errcode = 0;
    int *nodelist;
    char *marked;
    Snode *node;

    // Allocate memory for node list & marked list
    nodelist = (int *)calloc(net->Nnodes + 1, sizeof(int));
    marked = (char *)calloc(net->Nnodes + 1, sizeof(char));
    ERRCODE(MEMCHECK(nodelist));
    ERRCODE(MEMCHECK(marked));

    // If allocation fails return with 0 nodes disconnected
    if (errcode)
    {
        free(nodelist);
        free(marked);
        return (0);
    }

    // Place tanks on node list and marked list
    for (i = 1; i <= net->Ntanks; i++)
    {
        j = net->Njuncs + i;
        nodelist[i] = j;
        marked[j] = 1;
    }

    // Place junctions with negative demands on the lists
    mcount = net->Ntanks;
    for (i = 1; i <= net->Njuncs; i++)
    {
        if (hyd->NodeDemand[i] < 0.0)
        {
            mcount++;
            nodelist[mcount] = i;
            marked[i] = 1;
        }
    }

    // Mark all nodes that can be connected to tanks
    // and count number of nodes remaining unmarked
    marknodes(pr, mcount, nodelist, marked);
    j = 0;
    count = 0;
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        if (!marked[i] && hyd->NodeDemand[i] != 0.0)
        {
            count++;
            if (count <= MAXCOUNT && rpt->Messageflag)
            {
                sprintf(pr->Msg, WARN03a, node->ID,
                        clocktime(rpt->Atime, time->Htime));
                writeline(pr, pr->Msg);
            }
            j = i; // Last unmarked node
        }
    }

    // Report number of unmarked nodes and find closed link
    // on path from node j back to a tank
    if (count > 0 && rpt->Messageflag)
    {
        if (count > MAXCOUNT)
        {
            sprintf(pr->Msg, WARN03b, count - MAXCOUNT,
                    clocktime(rpt->Atime, time->Htime));
            writeline(pr, pr->Msg);
        }
        getclosedlink(pr, j, marked);
    }

    // Free allocated memory
    free(nodelist);
    free(marked);
    return count;
}

void marknodes(Project *pr, int m, int *nodelist, char *marked)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int i, j, k, n;
    Padjlist alink;

    // Scan each successive entry of node list
    n = 1;
    while (n <= m)
    {
        // Scan all nodes connected to current node
        i = nodelist[n];
        for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next)
        {
            // Get indexes of connecting link and node
            k = alink->link;
            j = alink->node;
            if (marked[j]) continue;

            // Check if valve connection is in correct direction
            switch (net->Link[k].Type)
            {
              case CVPIPE:
              case PRV:
              case PSV:
                if (j == net->Link[k].N1) continue;
                break;
              default:
                break;
            }

            // Mark connection node if link not closed
            if (hyd->LinkStatus[k] > CLOSED)
            {
                marked[j] = 1;
                m++;
                nodelist[m] = j;
            }
        }
        n++;
    }
}

void getclosedlink(Project *pr, int i, char *marked)
/*
**----------------------------------------------------------------
**   Input:   i = junction index
**            marked[] = marks nodes already examined
**   Output:  None.
**   Purpose: Determines if a closed link connects to junction i.
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int j, k;
    Padjlist alink;

    marked[i] = 2;
    for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next)
    {
        k = alink->link;
        j = alink->node;
        if (marked[j] == 2) continue;
        if (marked[j] == 1)
        {
            sprintf(pr->Msg, WARN03c, net->Link[k].ID);
            writeline(pr, pr->Msg);
            return;
        }
        else getclosedlink(pr, j, marked);
    }
}

void writelimits(Project *pr, int j1, int j2)
/*
**--------------------------------------------------------------
**   Input:   j1 = index of first output variable
**            j2 = index of last output variable
**   Output:  none
**   Purpose: writes reporting criteria to output report
**--------------------------------------------------------------
*/
{
    Report *rpt = &pr->report;
    int j;

    for (j = j1; j <= j2; j++)
    {
        if (rpt->Field[j].RptLim[LOW] < BIG)
        {
            sprintf(pr->Msg, FMT47, rpt->Field[j].Name,
                    rpt->Field[j].RptLim[LOW],
                    rpt->Field[j].Units);
            writeline(pr, pr->Msg);
        }
        if (rpt->Field[j].RptLim[HI] > -BIG)
        {
            sprintf(pr->Msg, FMT48, rpt->Field[j].Name,
            rpt->Field[j].RptLim[HI],
            rpt->Field[j].Units);
            writeline(pr, pr->Msg);
        }
    }
}

int checklimits(Report *rpt, double *y, int j1, int j2)
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
    for (j = j1; j <= j2; j++)
    {
        if (y[j] > rpt->Field[j].RptLim[LOW] ||
            y[j] < rpt->Field[j].RptLim[HI]
           ) return 0;
    }
    return 1;
}

void writetime(Project *pr, char *fmt)
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
    long h, m, s;
    h = seconds / 3600;
    m = seconds % 3600 / 60;
    s = seconds - 3600 * h - 60 * m;
    sprintf(atime, "%01d:%02d:%02d", (int)h, (int)m, (int)s);
    return atime;
}

char *fillstr(char *s, char ch, int n)
/*
**---------------------------------------------------------
**  Fills n bytes of s to character ch.
**  NOTE: does not check for overwriting s.
**---------------------------------------------------------
*/
{
    int i;
    for (i = 0; i <= n; i++) s[i] = ch;
    s[n + 1] = '\0';
    return (s);
}

int getnodetype(Network *net, int i)
/*
**---------------------------------------------------------
**  Determines type of node with index i
**  (junction = 0, reservoir = 1, tank = 2).
**---------------------------------------------------------
*/
{
    if (i <= net->Njuncs) return 0;
    if (net->Tank[i - net->Njuncs].A == 0.0) return 1;
    return 2;
}
