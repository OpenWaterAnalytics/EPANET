/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       inpfile.c
Description:  saves network data to an EPANET formatted text file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 10/29/2019
******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"

// Defined in enumstxt.h in EPANET.C
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

void saveauxdata(Project *pr, FILE *f)
/*
------------------------------------------------------------
  Writes auxilary data from original input file to new file.
------------------------------------------------------------
*/
{
    int sect, newsect;
    char *tok;
    char write;
    char line[MAXLINE + 1];
    char s[MAXLINE + 1];
    FILE *InFile = pr->parser.InFile;

    // Re-open the input file
    if (InFile == NULL)
    {
        InFile = fopen(pr->parser.InpFname, "rt");
        if (InFile == NULL) return;
    }
    rewind(InFile);
    sect = -1;

    // Read each line of the input file
    while (fgets(line, MAXLINE, InFile) != NULL)
    {
        strcpy(s, line);
        tok = strtok(s, SEPSTR);
        if (tok == NULL) continue;

        // Check if line begins with a new section heading
        if (*tok == '[')
        {
            newsect = findmatch(tok, SectTxt);
            if (newsect >= 0)
            {
                sect = newsect;
                if (sect == _END) break;

                // Write section heading to file
                switch (sect)
                {
                case _LABELS:
                case _BACKDROP:
                case _TAGS:
                    fprintf(f, "\n%s", line);
                }
            }
        }

        // Write line of auxilary data to file
        else
        {
            write = FALSE;
            switch (sect)
            {
            case _TAGS:
                if (*tok == ';' ||
                    (match("NODE", tok) && findnode(&pr->network, strtok(NULL, SEPSTR))) ||
                    (match("LINK", tok) && findlink(&pr->network, strtok(NULL, SEPSTR))))
                    write = TRUE;
                break;
            case _LABELS:
            case _BACKDROP:
                write = TRUE; break;
            default:
                break;
            }
            if (write) fprintf(f, "%s", line);
        }
    }
    fclose(InFile);
    InFile = NULL;
}

int saveinpfile(Project *pr, const char *fname)
/*
-------------------------------------------------
  Writes network data to text file.
-------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Report  *rpt = &pr->report;
    Outfile *out = &pr->outfile;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    int i, j, n;
    double d, kc, ke, km, ucf;
    char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1];
    Pdemand demand;
    Psource source;
    FILE *f;
    Slink *link;
    Stank *tank;
    Snode *node;
    Spump *pump;
    Scontrol *control;
    Scurve *curve;

    // Open the new text file
    if ((f = fopen(fname, "wt")) == NULL) return 302;

    // Write [TITLE] section
    fprintf(f, s_TITLE);
    for (i = 0; i < 3; i++)
    {
        if (strlen(pr->Title[i]) > 0) fprintf(f, "\n%s", pr->Title[i]);
    }

    // Write [JUNCTIONS] section
    // (Leave demands for [DEMANDS] section)
    fprintf(f, "\n\n");
    fprintf(f, s_JUNCTIONS);
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        fprintf(f, "\n %-31s %12.4f", node->ID, node->El * pr->Ucf[ELEV]);
        if (node->Comment) fprintf(f, "  ;%s", node->Comment);
    }

    // Write [RESERVOIRS] section
    fprintf(f, "\n\n");
    fprintf(f, s_RESERVOIRS);
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        if (tank->A == 0.0)
        {
            node = &net->Node[tank->Node];
            sprintf(s, " %-31s %12.4f", node->ID, node->El * pr->Ucf[ELEV]);
            if ((j = tank->Pat) > 0) sprintf(s1, " %s", net->Pattern[j].ID);
            else strcpy(s1, " ");
            fprintf(f, "\n%s %-31s", s, s1);
            if (node->Comment) fprintf(f, " ;%s", node->Comment);
        }
    }

    // Write [TANKS] section
    fprintf(f, "\n\n");
    fprintf(f, s_TANKS);
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        if (tank->A > 0.0)
        {
            node = &net->Node[tank->Node];
            sprintf(s, " %-31s %12.4f %12.4f %12.4f %12.4f %12.4f %12.4f",
                    node->ID, node->El * pr->Ucf[ELEV],
                    (tank->H0 - node->El) * pr->Ucf[ELEV],
                    (tank->Hmin - node->El) * pr->Ucf[ELEV],
                    (tank->Hmax - node->El) * pr->Ucf[ELEV],
                    sqrt(4.0 * tank->A / PI) * pr->Ucf[ELEV],
                    tank->Vmin * SQR(pr->Ucf[ELEV]) * pr->Ucf[ELEV]);
            if ((j = tank->Vcurve) > 0) sprintf(s1, "%s", net->Curve[j].ID);
            else if (tank->CanOverflow) strcpy(s1, "*");
            else strcpy(s1, " ");
            fprintf(f, "\n%s %-31s", s, s1);
            if (tank->CanOverflow) fprintf(f, "  YES  ");
            if (node->Comment) fprintf(f, " ;%s", node->Comment);
        }
    }

    // Write [PIPES] section
    fprintf(f, "\n\n");
    fprintf(f, s_PIPES);
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        if (link->Type <= PIPE)
        {
            d = link->Diam;
            kc = link->Kc;
            if (hyd->Formflag == DW)  kc = kc * pr->Ucf[ELEV] * 1000.0;
            km = link->Km * SQR(d) * SQR(d) / 0.02517;

            sprintf(s, " %-31s %-31s %-31s %12.4f %12.4f %12.4f %12.4f",
                    link->ID, net->Node[link->N1].ID, net->Node[link->N2].ID,
                    link->Len * pr->Ucf[LENGTH], d * pr->Ucf[DIAM], kc, km);

            if (link->Type == CVPIPE) sprintf(s2, "CV");
            else if (link->Status == CLOSED) sprintf(s2, "CLOSED");
            else strcpy(s2, " ");
            fprintf(f, "\n%s %-6s", s, s2);
            if (link->Comment) fprintf(f, " ;%s", link->Comment);
        }
    }

    // Write [PUMPS] section
    fprintf(f, "\n\n");
    fprintf(f, s_PUMPS);
    for (i = 1; i <= net->Npumps; i++)
    {
        n = net->Pump[i].Link;
        link = &net->Link[n];
        pump = &net->Pump[i];
        sprintf(s, " %-31s %-31s %-31s", link->ID, net->Node[link->N1].ID,
                net->Node[link->N2].ID);

        // Pump has constant power
        if (pump->Ptype == CONST_HP) sprintf(s1, "  POWER %.4f", link->Km);

        // Pump has a head curve
        else if ((j = pump->Hcurve) > 0)
        {
            sprintf(s1, "  HEAD %s", net->Curve[j].ID);
        }

        // Old format used for pump curve
        else
        {
            fprintf(f, "\n%s %12.4f %12.4f %12.4f          0.0 %12.4f", s,
                   -pump->H0 * pr->Ucf[HEAD],
                   (-pump->H0 - pump->R * pow(pump->Q0, pump->N)) * pr->Ucf[HEAD],
                   pump->Q0 * pr->Ucf[FLOW], pump->Qmax * pr->Ucf[FLOW]);
            continue;
        }
        strcat(s, s1);

        // Optional speed pattern
        if ((j = pump->Upat) > 0)
        {
            sprintf(s1, "  PATTERN  %s", net->Pattern[j].ID);
            strcat(s, s1);
        }

        // Optional speed setting
        if (link->Kc != 1.0)
        {
            sprintf(s1, "  SPEED %.4f", link->Kc);
            strcat(s, s1);
        }

        fprintf(f, "\n%s", s);
        if (link->Comment) fprintf(f, "  ;%s", link->Comment);

    }

    // Write [VALVES] section
    fprintf(f, "\n\n");
    fprintf(f, s_VALVES);
    for (i = 1; i <= net->Nvalves; i++)
    {
        n = net->Valve[i].Link;
        link = &net->Link[n];
        d = link->Diam;

        // Valve setting
        kc = link->Kc;
        if (kc == MISSING) kc = 0.0;
        switch (link->Type)
        {
          case FCV:
            kc *= pr->Ucf[FLOW];
            break;
          case PRV:
          case PSV:
          case PBV:
            kc *= pr->Ucf[PRESSURE];
            break;
          default:
            break;
        }
        km = link->Km * SQR(d) * SQR(d) / 0.02517;

        sprintf(s, " %-31s %-31s %-31s %12.4f %5s",
                link->ID, net->Node[link->N1].ID,
                net->Node[link->N2].ID, d * pr->Ucf[DIAM],
                LinkTxt[link->Type]);

        // For GPV, setting = head curve index
        if (link->Type == GPV && (j = ROUND(link->Kc)) > 0)
        {
            sprintf(s1, "%-31s %12.4f", net->Curve[j].ID, km);
        }
        else sprintf(s1, "%12.4f %12.4f", kc, km);
        fprintf(f, "\n%s %s", s, s1);
        if (link->Comment) fprintf(f, " ;%s", link->Comment);
    }


    // Write [DEMANDS] section
    fprintf(f, "\n\n");
    fprintf(f, s_DEMANDS);
    ucf = pr->Ucf[DEMAND];
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        for (demand = node->D; demand != NULL; demand = demand->next)
        {
            sprintf(s, " %-31s %14.6f", node->ID, ucf * demand->Base);
            if ((j = demand->Pat) > 0) sprintf(s1, " %-31s", net->Pattern[j].ID);
            else strcpy(s1, " ");
            fprintf(f, "\n%s %-31s", s, s1);
            if (demand->Name) fprintf(f, " ;%s", demand->Name);
        }
    }


    // Write [EMITTERS] section
    fprintf(f, "\n\n");
    fprintf(f, s_EMITTERS);
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        if (node->Ke == 0.0) continue;
        ke = pr->Ucf[FLOW] / pow(pr->Ucf[PRESSURE] * node->Ke, (1.0 / hyd->Qexp));
        fprintf(f, "\n %-31s %14.6f", node->ID, ke);
    }

    // Write [STATUS] section
    fprintf(f, "\n\n");
    fprintf(f, s_STATUS);
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        if (link->Type <= PUMP)
        {
            if (link->Status == CLOSED)
            {
                fprintf(f, "\n %-31s %s", link->ID, StatTxt[CLOSED]);
            }

            // Write pump speed here for pumps with old-style pump curve input
            else if (link->Type == PUMP)
            {
                n = findpump(net, i);
                pump = &net->Pump[n];
                if (pump->Hcurve == 0 && pump->Ptype != CONST_HP &&
                    link->Kc != 1.0)
                {
                    fprintf(f, "\n %-31s %-.4f", link->ID, link->Kc);
                }
            }
        }

        // Write fixed-status PRVs & PSVs (setting = MISSING)
        else if (link->Kc == MISSING)
        {
            if (link->Status == OPEN)
            {
                fprintf(f, "\n %-31s %s", link->ID, StatTxt[OPEN]);
            }
            if (link->Status == CLOSED)
            {
                fprintf(f, "\n%-31s %s", link->ID, StatTxt[CLOSED]);
            }
        }
    }

    // Write [PATTERNS] section
    // (Use 6 pattern factors per line)
    fprintf(f, "\n\n");
    fprintf(f, s_PATTERNS);
    for (i = 1; i <= net->Npats; i++)
    {
        if (net->Pattern[i].Comment) fprintf(f, "\n;%s", net->Pattern[i].Comment);
        for (j = 0; j < net->Pattern[i].Length; j++)
        {
            if (j % 6 == 0) fprintf(f, "\n %-31s", net->Pattern[i].ID);
            fprintf(f, " %12.4f", net->Pattern[i].F[j]);
        }
    }

    // Write [CURVES] section
    fprintf(f, "\n\n");
    fprintf(f, s_CURVES);
    for (i = 1; i <= net->Ncurves; i++)
    {
        if (net->Curve[i].Comment) fprintf(f, "\n;%s", net->Curve[i].Comment);
        for (j = 0; j < net->Curve[i].Npts; j++)
        {
            curve = &net->Curve[i];
            fprintf(f, "\n %-31s %12.4f %12.4f", curve->ID, curve->X[j], curve->Y[j]);
        }
    }

    // Write [CONTROLS] section
    fprintf(f, "\n\n");
    fprintf(f, s_CONTROLS);
    for (i = 1; i <= net->Ncontrols; i++)
    {
        // Check that controlled link exists
        control = &net->Control[i];
        if ((j = control->Link) <= 0) continue;
        link = &net->Link[j];

        // Get text of control's link status/setting
        if (control->Setting == MISSING)
        {
            sprintf(s, " LINK %s %s ", link->ID, StatTxt[control->Status]);
        }
        else
        {
            kc = control->Setting;
            switch (link->Type)
            {
              case PRV:
              case PSV:
              case PBV:
                kc *= pr->Ucf[PRESSURE];
                break;
              case FCV:
                kc *= pr->Ucf[FLOW];
                break;
              default:
                break;
            }
            sprintf(s, " LINK %s %.4f", link->ID, kc);
        }

        switch (control->Type)
        {
          // Print level control
          case LOWLEVEL:
          case HILEVEL:
            n = control->Node;
            node = &net->Node[n];
            kc = control->Grade - node->El;
            if (n > net->Njuncs) kc *= pr->Ucf[HEAD];
            else kc *= pr->Ucf[PRESSURE];
            fprintf(f, "\n%s IF NODE %s %s %.4f", s, node->ID,
                    ControlTxt[control->Type], kc);
            break;

          // Print timer control
          case TIMER:
            fprintf(f, "\n%s AT %s %.4f HOURS", s, ControlTxt[TIMER],
                    control->Time / 3600.);
            break;

          // Print time-of-day control
          case TIMEOFDAY:
            fprintf(f, "\n%s AT %s %s", s, ControlTxt[TIMEOFDAY],
                    clocktime(rpt->Atime, control->Time));
            break;
        }
    }

    // Write [RULES] section
    fprintf(f, "\n\n");
    fprintf(f, s_RULES);
    for (i = 1; i <= net->Nrules; i++)
    {
        fprintf(f, "\nRULE %s", pr->network.Rule[i].label);
        writerule(pr, f, i);  // see RULES.C
        fprintf(f, "\n");
    }

    // Write [QUALITY] section
    // (Skip nodes with default quality of 0)
    fprintf(f, "\n\n");
    fprintf(f, s_QUALITY);
    for (i = 1; i <= net->Nnodes; i++)
    {
        node = &net->Node[i];
        if (node->C0 == 0.0) continue;
        fprintf(f, "\n %-31s %14.6f", node->ID, node->C0 * pr->Ucf[QUALITY]);
    }

    // Write [SOURCES] section
    fprintf(f, "\n\n");
    fprintf(f, s_SOURCES);
    for (i = 1; i <= net->Nnodes; i++)
    {
        node = &net->Node[i];
        source = node->S;
        if (source == NULL) continue;
        sprintf(s, " %-31s %-8s %14.6f", node->ID, SourceTxt[source->Type],
                source->C0);
        if ((j = source->Pat) > 0)
        {
            sprintf(s1, "%s", net->Pattern[j].ID);
        }
        else strcpy(s1, "");
        fprintf(f, "\n%s %s", s, s1);
    }

    // Write [MIXING] section
    fprintf(f, "\n\n");
    fprintf(f, s_MIXING);
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        if (tank->A == 0.0) continue;
        fprintf(f, "\n %-31s %-8s %12.4f", net->Node[tank->Node].ID,
                MixTxt[tank->MixModel], (tank->V1max / tank->Vmax));
    }

    // Write [REACTIONS] section
    fprintf(f, "\n\n");
    fprintf(f, s_REACTIONS);

    // General parameters
    fprintf(f, "\n ORDER  BULK            %-.2f", qual->BulkOrder);
    fprintf(f, "\n ORDER  WALL            %-.0f", qual->WallOrder);
    fprintf(f, "\n ORDER  TANK            %-.2f", qual->TankOrder);
    fprintf(f, "\n GLOBAL BULK            %-.6f", qual->Kbulk * SECperDAY);
    fprintf(f, "\n GLOBAL WALL            %-.6f", qual->Kwall * SECperDAY);

    if (qual->Climit > 0.0)
    {
        fprintf(f, "\n LIMITING POTENTIAL     %-.6f", qual->Climit * pr->Ucf[QUALITY]);
    }
    if (qual->Rfactor != MISSING && qual->Rfactor != 0.0)
    {
        fprintf(f, "\n ROUGHNESS CORRELATION  %-.6f", qual->Rfactor);
    }

    // Pipe-specific parameters
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        if (link->Type > PIPE) continue;
        if (link->Kb != qual->Kbulk)
        {
            fprintf(f, "\n BULK   %-31s %-.6f", link->ID, link->Kb * SECperDAY);
        }
        if (link->Kw != qual->Kwall)
        {
            fprintf(f, "\n WALL   %-31s %-.6f", link->ID, link->Kw * SECperDAY);
        }
    }

    // Tank parameters
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        if (tank->A == 0.0) continue;
        if (tank->Kb != qual->Kbulk)
        {
            fprintf(f, "\n TANK   %-31s %-.6f", net->Node[tank->Node].ID,
                    tank->Kb * SECperDAY);
        }
    }

    // Write [ENERGY] section
    fprintf(f, "\n\n");
    fprintf(f, s_ENERGY);

    // General parameters
    if (hyd->Ecost != 0.0)
    {
        fprintf(f, "\n GLOBAL PRICE        %-.4f", hyd->Ecost);
    }
    if (hyd->Epat != 0)
    {
        fprintf(f, "\n GLOBAL PATTERN      %s", net->Pattern[hyd->Epat].ID);
    }
    fprintf(f, "\n GLOBAL EFFIC        %-.4f", hyd->Epump);
    fprintf(f, "\n DEMAND CHARGE       %-.4f", hyd->Dcost);

    // Pump-specific parameters
    for (i = 1; i <= net->Npumps; i++)
    {
        pump = &net->Pump[i];
        if (pump->Ecost > 0.0)
        {
            fprintf(f, "\n PUMP %-31s PRICE   %-.4f", net->Link[pump->Link].ID,
                    pump->Ecost);
        }
        if (pump->Epat > 0.0)
        {
            fprintf(f, "\n PUMP %-31s PATTERN %s", net->Link[pump->Link].ID,
                    net->Pattern[pump->Epat].ID);
        }
        if (pump->Ecurve > 0.0)
        {
            fprintf(f, "\n PUMP %-31s EFFIC   %s", net->Link[pump->Link].ID,
                    net->Curve[pump->Ecurve].ID);
        }
    }

    // Write [TIMES] section
    fprintf(f, "\n\n");
    fprintf(f, s_TIMES);
    fprintf(f, "\n DURATION            %s", clocktime(rpt->Atime, time->Dur));
    fprintf(f, "\n HYDRAULIC TIMESTEP  %s", clocktime(rpt->Atime, time->Hstep));
    fprintf(f, "\n QUALITY TIMESTEP    %s", clocktime(rpt->Atime, time->Qstep));
    fprintf(f, "\n REPORT TIMESTEP     %s", clocktime(rpt->Atime, time->Rstep));
    fprintf(f, "\n REPORT START        %s", clocktime(rpt->Atime, time->Rstart));
    fprintf(f, "\n PATTERN TIMESTEP    %s", clocktime(rpt->Atime, time->Pstep));
    fprintf(f, "\n PATTERN START       %s", clocktime(rpt->Atime, time->Pstart));
    fprintf(f, "\n RULE TIMESTEP       %s", clocktime(rpt->Atime, time->Rulestep));
    fprintf(f, "\n START CLOCKTIME     %s", clocktime(rpt->Atime, time->Tstart));
    fprintf(f, "\n STATISTIC           %s", TstatTxt[rpt->Tstatflag]);

    // Write [OPTIONS] section
    fprintf(f, "\n\n");
    fprintf(f, s_OPTIONS);
    fprintf(f, "\n UNITS               %s", FlowUnitsTxt[parser->Flowflag]);
    fprintf(f, "\n PRESSURE            %s", PressUnitsTxt[parser->Pressflag]);
    fprintf(f, "\n HEADLOSS            %s", FormTxt[hyd->Formflag]);
    switch (out->Hydflag)
    {
        case USE:
          fprintf(f, "\n HYDRAULICS USE      %s", out->HydFname);
          break;
        case SAVE:
          fprintf(f, "\n HYDRAULICS SAVE     %s", out->HydFname);
          break;
    }
    if (hyd->ExtraIter == -1)
    {
        fprintf(f, "\n UNBALANCED          STOP");
    }
    if (hyd->ExtraIter >= 0)
    {
        fprintf(f, "\n UNBALANCED          CONTINUE %d", hyd->ExtraIter);
    }

    switch (qual->Qualflag)
    {
        case CHEM:
          fprintf(f, "\n QUALITY             %s %s",
                  qual->ChemName, qual->ChemUnits);
          break;
        case TRACE:
          fprintf(f, "\n QUALITY             TRACE %-31s",
                  net->Node[qual->TraceNode].ID);
          break;
        case AGE:
          fprintf(f, "\n QUALITY             AGE");
          break;
        case NONE:
          fprintf(f, "\n QUALITY             NONE");
          break;
    }

    fprintf(f, "\n DEMAND MULTIPLIER   %-.4f", hyd->Dmult);
    fprintf(f, "\n EMITTER EXPONENT    %-.4f", 1.0 / hyd->Qexp);
    fprintf(f, "\n VISCOSITY           %-.6f", hyd->Viscos / VISCOS);
    fprintf(f, "\n DIFFUSIVITY         %-.6f", qual->Diffus / DIFFUS);
    fprintf(f, "\n SPECIFIC GRAVITY    %-.6f", hyd->SpGrav);
    fprintf(f, "\n TRIALS              %-d", hyd->MaxIter);
    fprintf(f, "\n ACCURACY            %-.8f", hyd->Hacc);
    fprintf(f, "\n TOLERANCE           %-.8f", qual->Ctol * pr->Ucf[QUALITY]);
    fprintf(f, "\n CHECKFREQ           %-d", hyd->CheckFreq);
    fprintf(f, "\n MAXCHECK            %-d", hyd->MaxCheck);
    fprintf(f, "\n DAMPLIMIT           %-.8f", hyd->DampLimit);
    if (hyd->HeadErrorLimit > 0.0)
    {
        fprintf(f, "\n HEADERROR           %-.8f",
                hyd->HeadErrorLimit * pr->Ucf[HEAD]);
    }
    if (hyd->FlowChangeLimit > 0.0)
    {
        fprintf(f, "\n FLOWCHANGE          %-.8f",
                hyd->FlowChangeLimit * pr->Ucf[FLOW]);
    }
    if (hyd->DemandModel == PDA)
    {
        fprintf(f, "\n DEMAND MODEL        PDA");
        fprintf(f, "\n MINIMUM PRESSURE    %-.4f", hyd->Pmin * pr->Ucf[PRESSURE]);
        fprintf(f, "\n REQUIRED PRESSURE   %-.4f", hyd->Preq * pr->Ucf[PRESSURE]);
        fprintf(f, "\n PRESSURE EXPONENT   %-.4f", hyd->Pexp);
    }

    // Write [REPORT] section
    fprintf(f, "\n\n");
    fprintf(f, s_REPORT);

    // General options
    fprintf(f, "\n PAGESIZE            %d", rpt->PageSize);
    fprintf(f, "\n STATUS              %s", RptFlagTxt[rpt->Statflag]);
    fprintf(f, "\n SUMMARY             %s", RptFlagTxt[rpt->Summaryflag]);
    fprintf(f, "\n ENERGY              %s", RptFlagTxt[rpt->Energyflag]);
    fprintf(f, "\n MESSAGES            %s", RptFlagTxt[rpt->Messageflag]);
    if (strlen(rpt->Rpt2Fname) > 0)
    {
        fprintf(f, "\n FILE                %s", rpt->Rpt2Fname);
    }

    // Node reporting
    switch (rpt->Nodeflag)
    {
        case 0:
          fprintf(f, "\n NODES               NONE");
          break;
        case 1:
          fprintf(f, "\n NODES               ALL");
          break;
        default:
          j = 0;
          for (i = 1; i <= net->Nnodes; i++)
          {
              node = &net->Node[i];
              if (node->Rpt == 1)
              {
                  if (j % 5 == 0) fprintf(f, "\n NODES               ");
                  fprintf(f, "%s ", node->ID);
                  j++;
             }
          }
    }

    // Link reporting
    switch (rpt->Linkflag)
    {
        case 0:
          fprintf(f, "\n LINKS               NONE");
          break;
        case 1:
          fprintf(f, "\n LINKS               ALL");
          break;
        default:
          j = 0;
          for (i = 1; i <= net->Nlinks; i++)
          {
              link = &net->Link[i];
              if (link->Rpt == 1)
              {
                  if (j % 5 == 0) fprintf(f, "\n LINKS               ");
                  fprintf(f, "%s ", link->ID);
                  j++;
              }
          }
    }

    // Field formatting options
    for (i = 0; i < FRICTION; i++)
    {
        SField *field = &rpt->Field[i];
        if (field->Enabled == TRUE)
        {
            fprintf(f, "\n %-20sPRECISION %d", field->Name, field->Precision);
            if (field->RptLim[LOW] < BIG)
            {
                fprintf(f, "\n %-20sBELOW %.6f", field->Name, field->RptLim[LOW]);
            }
            if (field->RptLim[HI] > -BIG)
            {
                fprintf(f, "\n %-20sABOVE %.6f", field->Name, field->RptLim[HI]);
            }
        }
        else fprintf(f, "\n %-20sNO",field->Name);
    }

    // Write [COORDINATES] section
    fprintf(f, "\n\n");
    fprintf(f, s_COORDS);
    for (i = 1; i <= net->Nnodes; i++)
    {
        node = &net->Node[i];
        if (node->X == MISSING || node->Y == MISSING) continue;
        fprintf(f, "\n %-31s %14.6f %14.6f", node->ID, node->X, node->Y);
    }

    // Write [VERTICES] section
    fprintf(f, "\n\n");
    fprintf(f, s_VERTICES);
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        if (link->Vertices != NULL)
        {
            for (j = 0; j < link->Vertices->Npts; j++)
                fprintf(f, "\n %-31s %14.6f %14.6f",
                    link->ID, link->Vertices->X[j], link->Vertices->Y[j]);
        }
    }

    // Save auxilary data to new input file
    fprintf(f, "\n");
    saveauxdata(pr, f);

    // Close the new input file
    fprintf(f, "\n%s\n", s_END);
    fclose(f);
    return 0;
}
