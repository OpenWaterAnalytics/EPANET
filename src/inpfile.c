/*
*********************************************************************

INPFILE.C -- Save Input Function for EPANET Program

VERSION:    2.00
DATE:       5/8/00
            3/1/01
            11/19/01
            6/24/02
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL

This module contains the function saveinpfile() which saves the
data describing a piping network to a file in EPANET's text format.

********************************************************************
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
#include "vars.h"

/* Defined in enumstxt.h in EPANET.C */
extern char *LinkTxt[];
extern char *FormTxt[];
extern char *StatTxt[];
extern char *FlowUnitsTxt[];
extern char *PressUnitsTxt[];
extern char *ControlTxt[];
extern char *SourceTxt[];
extern char *MixTxt[];
extern char *TstatTxt[];
extern char *RptFlagTxt[];
extern char *SectTxt[];

void saveauxdata(parser_data_t *parser, FILE *f) 
/*
------------------------------------------------------------
  Writes auxilary data from original input file to new file.
------------------------------------------------------------
*/
{
  int sect, newsect;
  char *tok;
  char line[MAXLINE + 1];
  char s[MAXLINE + 1];
  FILE *InFile = parser->InFile;
  char Coordflag = parser->Coordflag;
  
  sect = -1;
  if (InFile == NULL) {
    return;
  }
  rewind(InFile);
  while (fgets(line, MAXLINE, InFile) != NULL) {
    /* Check if line begins with a new section heading */
    strcpy(s, line);
    tok = strtok(s, SEPSTR);
    if (tok != NULL && *tok == '[') {
      newsect = findmatch(tok, SectTxt);
      if (newsect >= 0) {
        sect = newsect;
        if (sect == _END)
          break;
        switch (sect) {
        // case _RULES:
        case _COORDS:
          if (Coordflag == FALSE) {
            fprintf(f, "%s", line);
          }
          break;
        case _VERTICES:
        case _LABELS:
        case _BACKDROP:
        case _TAGS:
          fprintf(f, "%s", line); 
        }
        continue;
      } else
        continue;
    }

    /* Write lines appearing in the section to file */
    switch (sect) {
    // case _RULES:
    case _COORDS:
      if (Coordflag == FALSE) {
        fprintf(f, "%s", line);
      }
      break;
    case _VERTICES:
    case _LABELS:
    case _BACKDROP:
    case _TAGS:
      fprintf(f, "%s", line); 
    }
  }
}

////  This function was heavily modified.  //// 

int saveinpfile(EN_Project *pr, char *fname)
/*
-------------------------------------------------
  Writes network data to text file.
-------------------------------------------------
*/
{
  
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  report_options_t *rep = &pr->report;
  out_file_t *out = &pr->out_files;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  time_options_t *time = &pr->time_options;
  
  int i, j, n;
  int errcode;
  double d, kc, ke, km, ucf;
  char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1];
  Pdemand demand;
  Psource source;
  FILE *f;

  /* Open the new text file */

  if ((f = fopen(fname, "wt")) == NULL) {
    return (308);
  }

  /* Write [TITLE] section */

  fprintf(f, "[TITLE]");
  for (i = 0; i < 3; i++) {
    if (strlen(pr->Title[i]) > 0) {
      fprintf(f, "\n%s", pr->Title[i]);
    }
  }

  /* Write [JUNCTIONS] section */
  /* (Leave demands for [DEMANDS] section) */

  fprintf(f, "\n\n[JUNCTIONS]");
  for (i = 1; i <= net->Njuncs; i++) {
    Snode *node = &net->Node[i];
    fprintf(f, "\n %-31s %12.4f", node->ID, node->El * pr->Ucf[ELEV]);
  }
  /* Write [RESERVOIRS] section */

  fprintf(f, "\n\n[RESERVOIRS]");
  for (i = 1; i <= net->Ntanks; i++) {
    Stank *tank = &net->Tank[i];
    if (tank->A == 0.0) {
      Snode *node = &net->Node[tank->Node];
      sprintf(s, " %-31s %12.4f", node->ID, node->El * pr->Ucf[ELEV]);
      if ((j = tank->Pat) > 0) {
        sprintf(s1, " %-31s", net->Pattern[j].ID);
      }
      else {
        strcpy(s1, "");
      }
      fprintf(f, "\n%s %s", s, s1);
    }
  }

  /* Write [TANKS] section */

  fprintf(f, "\n\n[TANKS]");
  for (i = 1; i <= net->Ntanks; i++) {
    Stank *tank = &net->Tank[i];
    if (tank->A > 0.0) {
      Snode *node = &net->Node[tank->Node];
      sprintf(s, " %-31s %12.4f %12.4f %12.4f %12.4f %12.4f %12.4f", node->ID,
              node->El * pr->Ucf[ELEV], (tank->H0 - node->El) * pr->Ucf[ELEV],
              (tank->Hmin - node->El) * pr->Ucf[ELEV],
              (tank->Hmax - node->El) * pr->Ucf[ELEV],
              sqrt(4.0 * tank->A / PI) * pr->Ucf[ELEV],
              tank->Vmin * SQR(pr->Ucf[ELEV]) * pr->Ucf[ELEV]);
      if ((j = tank->Vcurve) > 0)
        sprintf(s1, "%-31s", net->Curve[j].ID);
      else
        strcpy(s1, "");
      fprintf(f, "\n%s %s", s, s1);
    }
  }

  /* Write [PIPES] section */

  fprintf(f, "\n\n[PIPES]");
  for (i = 1; i <= net->Nlinks; i++) {
    Slink *link = &net->Link[i];
    if (link->Type <= EN_PIPE) {
      d = link->Diam;
      kc = link->Kc;
      if (hyd->Formflag == DW)
        kc = kc * pr->Ucf[ELEV] * 1000.0;
      km = link->Km * SQR(d) * SQR(d) / 0.02517;
      sprintf(s, " %-31s %-31s %-31s %12.4f %12.4f", link->ID,
              net->Node[link->N1].ID, net->Node[link->N2].ID,
              link->Len * pr->Ucf[LENGTH], d * pr->Ucf[DIAM]);
      if (hyd->Formflag == DW)
        sprintf(s1, "%12.4f %12.4f", kc, km);
      else
        sprintf(s1, "%12.4f %12.4f", kc, km);
      if (link->Type == EN_CVPIPE)
        sprintf(s2, "CV");
      else if (link->Stat == CLOSED)
        sprintf(s2, "CLOSED");
      else
        strcpy(s2, "");
      fprintf(f, "\n%s %s %s", s, s1, s2);
    }
  }

  /* Write [PUMPS] section */

  fprintf(f, "\n\n[PUMPS]");
  for (i = 1; i <= net->Npumps; i++) {
    n = net->Pump[i].Link;
    Slink *link = &net->Link[n];
    Spump *pump = &net->Pump[i];
    sprintf(s, " %-31s %-31s %-31s", link->ID, net->Node[link->N1].ID, net->Node[link->N2].ID);

    /* Pump has constant power */
    if (pump->Ptype == CONST_HP)
      sprintf(s1, "  POWER %.4f", link->Km);

    /* Pump has a head curve */
    else if ((j = pump->Hcurve) > 0)
      sprintf(s1, "  HEAD %s", net->Curve[j].ID);

    /* Old format used for pump curve */
    else {
      fprintf(f, "\n%s %12.4f %12.4f %12.4f          0.0 %12.4f", s,
              -pump->H0 * pr->Ucf[HEAD],
              (-pump->H0 - pump->R * pow(pump->Q0, pump->N)) *
                  pr->Ucf[HEAD],
              pump->Q0 * pr->Ucf[FLOW], pump->Qmax * pr->Ucf[FLOW]);
      continue;
    }
    strcat(s, s1);

    if ((j = pump->Upat) > 0)
      sprintf(s1, "   PATTERN  %s", net->Pattern[j].ID);
    else
      strcpy(s1, "");
    strcat(s, s1);

    if (link->Kc != 1.0)
      sprintf(s1, "  SPEED %.4f", link->Kc);
    else
      strcpy(s1, "");
    strcat(s, s1);

    fprintf(f, "\n%s", s);
  }

  /* Write [VALVES] section */

  fprintf(f, "\n\n[VALVES]");
  for (i = 1; i <= net->Nvalves; i++) {
    n = net->Valve[i].Link;
    Slink *link = &net->Link[n];
    d = link->Diam;
    kc = link->Kc;
    if (kc == MISSING)
      kc = 0.0;
    switch (link->Type) {
      case EN_FCV:
        kc *= pr->Ucf[FLOW];
        break;
      case EN_PRV:
      case EN_PSV:
      case EN_PBV:
        kc *= pr->Ucf[PRESSURE];
        break;
      default:
        break;
        // not a valve... pipe/pump/cv
    }
    km = link->Km * SQR(d) * SQR(d) / 0.02517;

    sprintf(s, " %-31s %-31s %-31s %12.4f %5s", link->ID, net->Node[link->N1].ID,
            net->Node[link->N2].ID, d * pr->Ucf[DIAM], LinkTxt[link->Type]);

    if (link->Type == EN_GPV && (j = ROUND(link->Kc)) > 0)
      sprintf(s1, "%-31s %12.4f", net->Curve[j].ID, km);
    else
      sprintf(s1, "%12.4f %12.4f", kc, km);

    fprintf(f, "\n%s %s", s, s1);
  }

  /* Write [DEMANDS] section */

  fprintf(f, "\n\n[DEMANDS]");
  ucf = pr->Ucf[DEMAND];
  for (i = 1; i <= net->Njuncs; i++) {
    Snode *node = &net->Node[i];
    for (demand = node->D; demand != NULL; demand = demand->next) {
      sprintf(s, " %-31s %14.6f", node->ID, ucf * demand->Base);
      if ((j = demand->Pat) > 0)
        sprintf(s1, "   %s", net->Pattern[j].ID);
      else
        strcpy(s1, "");
      fprintf(f, "\n%s %s", s, s1);
    }
  }

  /* Write [EMITTERS] section */

  fprintf(f, "\n\n[EMITTERS]");
  for (i = 1; i <= net->Njuncs; i++) {
    Snode *node = &net->Node[i];
    if (node->Ke == 0.0)
      continue;
    ke = pr->Ucf[FLOW] / pow(pr->Ucf[PRESSURE] * node->Ke, (1.0 / hyd->Qexp));
    fprintf(f, "\n %-31s %14.6f", node->ID, ke);
  }

  /* Write [STATUS] section */

  fprintf(f, "\n\n[STATUS]");
  for (i = 1; i <= net->Nlinks; i++) {
    Slink *link = &net->Link[i];
    if (link->Type <= EN_PUMP) {
      if (link->Stat == CLOSED)
        fprintf(f, "\n %-31s %s", link->ID, StatTxt[CLOSED]);

      /* Write pump speed here for pumps with old-style pump curve input */
      else if (link->Type == EN_PUMP) {
        n = findpump(net, i);
        Spump *pump = &net->Pump[n];
        if (pump->Hcurve == 0 && pump->Ptype != CONST_HP &&
            link->Kc != 1.0)
          fprintf(f, "\n %-31s %-.4f", link->ID, link->Kc);
      }
    }

    /* Write fixed-status PRVs & PSVs (setting = MISSING) */
    else if (link->Kc == MISSING) {
      if (link->Stat == OPEN)
        fprintf(f, "\n %-31s %s", link->ID, StatTxt[OPEN]);
      if (link->Stat == CLOSED)
        fprintf(f, "\n%-31s %s", link->ID, StatTxt[CLOSED]);
    }
  }

  /* Write [PATTERNS] section */
  /* (Use 6 pattern factors per line) */

  fprintf(f, "\n\n[PATTERNS]");
  for (i = 1; i <= net->Npats; i++) {
    for (j = 0; j < net->Pattern[i].Length; j++) {
      if (j % 6 == 0)
        fprintf(f, "\n %-31s", net->Pattern[i].ID);
      fprintf(f, " %12.4f", net->Pattern[i].F[j]);
    }
  }

  /* Write [CURVES] section */

  fprintf(f, "\n\n[CURVES]");
  for (i = 1; i <= net->Ncurves; i++) {
    for (j = 0; j < net->Curve[i].Npts; j++) {
      Scurve *curve = &net->Curve[i];
      fprintf(f, "\n %-31s %12.4f %12.4f", curve->ID, curve->X[j], curve->Y[j]);
    }
  }

  /* Write [CONTROLS] section */

  fprintf(f, "\n\n[CONTROLS]");
  for (i = 1; i <= net->Ncontrols; i++) {
    Scontrol *control = &net->Control[i];
    /* Check that controlled link exists */
    if ((j = control->Link) <= 0)
      continue;
    Slink *link = &net->Link[j];

    /* Get text of control's link status/setting */
    if (control->Setting == MISSING)
      sprintf(s, " LINK %s %s ", link->ID, StatTxt[control->Status]);
    else {
      kc = control->Setting;
      switch (link->Type) {
        case EN_PRV:
        case EN_PSV:
        case EN_PBV:
          kc *= pr->Ucf[PRESSURE];
          break;
        case EN_FCV:
          kc *= pr->Ucf[FLOW];
          break;
        default:
          break;
      }
      sprintf(s, " LINK %s %.4f", link->ID, kc);
    }

    switch (control->Type) {
    /* Print level control */
    case LOWLEVEL:
    case HILEVEL:
      n = control->Node;
        Snode *node = &net->Node[n];
      kc = control->Grade - node->El;
      if (n > net->Njuncs)
        kc *= pr->Ucf[HEAD];
      else
        kc *= pr->Ucf[PRESSURE];
      fprintf(f, "\n%s IF NODE %s %s %.4f", s, node->ID,
              ControlTxt[control->Type], kc);
      break;

    /* Print timer control */
    case TIMER:
      fprintf(f, "\n%s AT %s %.4f HOURS", s, ControlTxt[TIMER],
              control->Time / 3600.);
      break;

    /* Print time-of-day control */
    case TIMEOFDAY:
      fprintf(f, "\n%s AT %s %s", s, ControlTxt[TIMEOFDAY],
              clocktime(rep->Atime, control->Time));
      break;
    }
  }

  /* Write [RULES] section */

  fprintf(f, "\n\n[RULES]");
  for (i = 1; i <= net->Nrules; i++) {
    fprintf(f, "\nRULE %s", pr->rules.Rule[i].label);
    errcode = writeRuleinInp(f, i);
    fprintf(f, "\n");
  }

  /* Write [QUALITY] section */
  /* (Skip nodes with default quality of 0) */

  fprintf(f, "\n\n[QUALITY]");
  for (i = 1; i <= net->Nnodes; i++) {
    Snode *node = &net->Node[i];
    if (node->C0 == 0.0)
      continue;
    fprintf(f, "\n %-31s %14.6f", node->ID, node->C0 * pr->Ucf[QUALITY]);
  }

  /* Write [SOURCES] section */

  fprintf(f, "\n\n[SOURCES]");
  for (i = 1; i <= net->Nnodes; i++) {
    Snode *node = &net->Node[i];
    source = node->S;
    if (source == NULL)
      continue;
    sprintf(s, " %-31s %-8s %14.6f", node->ID, SourceTxt[source->Type], source->C0);
    if ((j = source->Pat) > 0)
      sprintf(s1, "%s", net->Pattern[j].ID);
    else
      strcpy(s1, "");
    fprintf(f, "\n%s %s", s, s1);
  }

  /* Write [MIXING] section */

  fprintf(f, "\n\n[MIXING]");
  for (i = 1; i <= net->Ntanks; i++) {
    Stank *tank = &net->Tank[i];
    if (tank->A == 0.0)
      continue;
    fprintf(f, "\n %-31s %-8s %12.4f", net->Node[tank->Node].ID,
            MixTxt[tank->MixModel], (tank->V1max / tank->Vmax));
  }

  /* Write [REACTIONS] section */

  fprintf(f, "\n\n[REACTIONS]");
  fprintf(f, "\n ORDER  BULK            %-.2f", qu->BulkOrder);
  fprintf(f, "\n ORDER  WALL            %-.0f", qu->WallOrder);
  fprintf(f, "\n ORDER  TANK            %-.2f", qu->TankOrder);
  fprintf(f, "\n GLOBAL BULK            %-.6f", qu->Kbulk * SECperDAY);
  fprintf(f, "\n GLOBAL WALL            %-.6f", qu->Kwall * SECperDAY);
  if (qu->Climit > 0.0)
    fprintf(f, "\n LIMITING POTENTIAL     %-.6f", qu->Climit);
  if (qu->Rfactor != MISSING && qu->Rfactor != 0.0)
    fprintf(f, "\n ROUGHNESS CORRELATION  %-.6f", qu->Rfactor);
  for (i = 1; i <= net->Nlinks; i++) {
    Slink *link = &net->Link[i];
    if (link->Type > EN_PIPE)
      continue;
    if (link->Kb != qu->Kbulk)
      fprintf(f, "\n BULK   %-31s %-.6f", link->ID, link->Kb * SECperDAY);
    if (link->Kw != qu->Kwall)
      fprintf(f, "\n WALL   %-31s %-.6f", link->ID, link->Kw * SECperDAY);
  }
  for (i = 1; i <= net->Ntanks; i++) {
    Stank *tank = &net->Tank[i];
    if (tank->A == 0.0)
      continue;
    if (tank->Kb != qu->Kbulk)
      fprintf(f, "\n TANK   %-31s %-.6f", net->Node[tank->Node].ID,
              tank->Kb * SECperDAY);
  }

  /* Write [ENERGY] section */

  fprintf(f, "\n\n[ENERGY]");
  if (hyd->Ecost != 0.0)
    fprintf(f, "\n GLOBAL PRICE        %-.4f", hyd->Ecost);
  if (hyd->Epat != 0)
    fprintf(f, "\n GLOBAL PATTERN      %s", net->Pattern[hyd->Epat].ID);
  fprintf(f, "\n GLOBAL EFFIC        %-.4f", hyd->Epump);
  fprintf(f, "\n DEMAND CHARGE       %-.4f", hyd->Dcost);
  for (i = 1; i <= net->Npumps; i++) {
    Spump *pump = &net->Pump[i];
    if (pump->Ecost > 0.0)
      fprintf(f, "\n PUMP %-31s PRICE   %-.4f", net->Link[pump->Link].ID,
              pump->Ecost);
    if (pump->Epat > 0.0)
      fprintf(f, "\n PUMP %-31s PATTERN %s", net->Link[pump->Link].ID,
              net->Pattern[pump->Epat].ID);
    if (pump->Ecurve > 0.0)
      fprintf(f, "\n PUMP %-31s EFFIC   %s", net->Link[pump->Link].ID,
              net->Curve[pump->Ecurve].ID);
  }

  /* Write [TIMES] section */

  fprintf(f, "\n\n[TIMES]");
  fprintf(f, "\n DURATION            %s", clocktime(rep->Atime, time->Dur));
  fprintf(f, "\n HYDRAULIC TIMESTEP  %s", clocktime(rep->Atime, time->Hstep));
  fprintf(f, "\n QUALITY TIMESTEP    %s", clocktime(rep->Atime, qu->Qstep));
  fprintf(f, "\n REPORT TIMESTEP     %s", clocktime(rep->Atime, time->Rstep));
  fprintf(f, "\n REPORT START        %s", clocktime(rep->Atime, time->Rstart));
  fprintf(f, "\n PATTERN TIMESTEP    %s", clocktime(rep->Atime, time->Pstep));
  fprintf(f, "\n PATTERN START       %s", clocktime(rep->Atime, time->Pstart));
  fprintf(f, "\n RULE TIMESTEP       %s", clocktime(rep->Atime, time->Rulestep));
  fprintf(f, "\n START CLOCKTIME     %s", clocktime(rep->Atime, time->Tstart));
  fprintf(f, "\n STATISTIC           %s", TstatTxt[rep->Tstatflag]);

  /* Write [OPTIONS] section */

  fprintf(f, "\n\n[OPTIONS]");
  fprintf(f, "\n UNITS               %s", FlowUnitsTxt[par->Flowflag]);
  fprintf(f, "\n PRESSURE            %s", PressUnitsTxt[par->Pressflag]);
  fprintf(f, "\n HEADLOSS            %s", FormTxt[hyd->Formflag]);
  if (hyd->DefPat >= 1 && hyd->DefPat <= net->Npats)
    fprintf(f, "\n PATTERN             %s", net->Pattern[hyd->DefPat].ID);
  if (out->Hydflag == USE)
    fprintf(f, "\n HYDRAULICS USE      %s", out->HydFname);
  if (out->Hydflag == SAVE)
    fprintf(f, "\n HYDRAULICS SAVE     %s", out->HydFname);
  if (hyd->ExtraIter == -1)
    fprintf(f, "\n UNBALANCED          STOP");
  if (hyd->ExtraIter >= 0)
    fprintf(f, "\n UNBALANCED          CONTINUE %d", hyd->ExtraIter);
  if (qu->Qualflag == CHEM)
    fprintf(f, "\n QUALITY             %s %s", qu->ChemName, qu->ChemUnits);
  if (qu->Qualflag == TRACE)
    fprintf(f, "\n QUALITY             TRACE %-31s", net->Node[qu->TraceNode].ID);
  if (qu->Qualflag == AGE)
    fprintf(f, "\n QUALITY             AGE");
  if (qu->Qualflag == NONE)
    fprintf(f, "\n QUALITY             NONE");
  fprintf(f, "\n DEMAND MULTIPLIER   %-.4f", hyd->Dmult);
  fprintf(f, "\n EMITTER EXPONENT    %-.4f", 1.0 / hyd->Qexp);
  fprintf(f, "\n VISCOSITY           %-.6f", hyd->Viscos / VISCOS);
  fprintf(f, "\n DIFFUSIVITY         %-.6f", qu->Diffus / DIFFUS);
  fprintf(f, "\n SPECIFIC GRAVITY    %-.6f", hyd->SpGrav);
  fprintf(f, "\n TRIALS              %-d", hyd->MaxIter);
  fprintf(f, "\n ACCURACY            %-.8f", hyd->Hacc);
  fprintf(f, "\n TOLERANCE           %-.8f", qu->Ctol * pr->Ucf[QUALITY]);
  fprintf(f, "\n CHECKFREQ           %-d", hyd->CheckFreq);
  fprintf(f, "\n MAXCHECK            %-d", hyd->MaxCheck);
  fprintf(f, "\n DAMPLIMIT           %-.8f", hyd->DampLimit);

  /* Write [REPORT] section */

  fprintf(f, "\n\n[REPORT]");
  fprintf(f, "\n PAGESIZE            %d", rep->PageSize);
  fprintf(f, "\n STATUS              %s", RptFlagTxt[rep->Statflag]);
  fprintf(f, "\n SUMMARY             %s", RptFlagTxt[rep->Summaryflag]);
  fprintf(f, "\n ENERGY              %s", RptFlagTxt[rep->Energyflag]);
  fprintf(f, "\n MESSAGES            %s", RptFlagTxt[rep->Messageflag]);
  if (strlen(rep->Rpt2Fname) > 0)
    fprintf(f, "\n FILE                %s", rep->Rpt2Fname);
  switch (rep->Nodeflag) {
  case 0:
    fprintf(f, "\n NODES               NONE");
    break;
  case 1:
    fprintf(f, "\n NODES               ALL");
    break;
  default:
    j = 0;
    for (i = 1; i <= net->Nnodes; i++) {
      Snode *node = &net->Node[i];
      if (node->Rpt == 1) {
        if (j % 5 == 0)
          fprintf(f, "\n NODES               ");
        fprintf(f, "%s ", node->ID);
        j++;
      }
    }
  }
  switch (rep->Linkflag) {
  case 0:
    fprintf(f, "\n LINKS               NONE");
    break;
  case 1:
    fprintf(f, "\n LINKS               ALL");
    break;
  default:
    j = 0;
    for (i = 1; i <= net->Nlinks; i++) {
      Slink *link = &net->Link[i];
      if (link->Rpt == 1) {
        if (j % 5 == 0)
          fprintf(f, "\n LINKS               ");
        fprintf(f, "%s ", link->ID);
        j++;
      }
    }
  }
  for (i = 0; i < FRICTION; i++) {
    SField *field = &rep->Field[i];
    if (field->Enabled == TRUE) {
      fprintf(f, "\n %-20sPRECISION %d", field->Name, field->Precision);
      if (field->RptLim[LOW] < BIG)
        fprintf(f, "\n %-20sBELOW %.6f", field->Name, field->RptLim[LOW]);
      if (field->RptLim[HI] > -BIG)
        fprintf(f, "\n %-20sABOVE %.6f", field->Name, field->RptLim[HI]);
    } else
      fprintf(f, "\n %-20sNO",field->Name);
  }
  fprintf(f, "\n\n");

  /* Write [COORDINATES] section */

  if (par->Coordflag == TRUE) {
    fprintf(f, "\n\n[COORDINATES]");
    for (i = 1; i <= net->Nnodes; i++) {
      Snode *node = &net->Node[i];
      Scoord *coord = &net->Coord[i];
      if (coord->HaveCoords == TRUE) {
        fprintf(f, "\n %-31s %14.6f %14.6f", node->ID, coord->X, coord->Y);
      }
    }
    fprintf(f, "\n\n");
  }

  /* Save auxilary data to new input file */

  saveauxdata(par,f);

  /* Close the new input file */

  fprintf(f, "\n[END]");
  fclose(f);
  return (0);
}
