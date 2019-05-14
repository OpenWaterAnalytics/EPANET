/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       output.c
Description:  binary file read/write routines
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 05/13/2019
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

// Local functions
static int  nodeoutput(Project *, int, REAL4 *, double);
static int  linkoutput(Project *, int, REAL4 *, double);
static int  savetimestat(Project *, REAL4 *, HdrType);
static int  savenetreacts(Project *, double, double, double, double);
static int  saveepilog(Project *);

// Functions to write/read x[1] to x[n] to/from binary file
size_t f_save(REAL4 *x, int n, FILE *file)
{
    return fwrite(x + 1, sizeof(REAL4), n, file);
}
size_t f_read(REAL4 *x, int n, FILE *file)
{
    return fread(x + 1, sizeof(REAL4), n, file);
}

int savenetdata(Project *pr)
/*
**---------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: saves input data in original units to binary
**            output file using fixed-sized (4-byte) records
**---------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Outfile *out = &pr->outfile;
    Report  *rpt = &pr->report;
    Quality *qual = &pr->quality;
    Parser  *parser = &pr->parser;
    Times   *time = &pr->times;

    int i, nmax;
    int errcode = 0;
    INT4 *ibuf;
    REAL4 *x;
    Snode *node;
    FILE  *outFile = out->OutFile;

    // Allocate buffer arrays
    nmax = MAX(net->Nnodes, net->Nlinks) + 1;
    nmax = MAX(nmax, 15);
    ibuf = (INT4 *)calloc(nmax, sizeof(INT4));
    x = (REAL4 *)calloc(nmax, sizeof(REAL4));
    ERRCODE(MEMCHECK(ibuf));
    ERRCODE(MEMCHECK(x));

    // Write prolog section of binary output file
    if (!errcode)
    {
        // Write integer variables to outFile
        ibuf[0] = MAGICNUMBER;
        ibuf[1] = 20012;  // keep version at 2.00.12 so that GUI will run
        ibuf[2] = net->Nnodes;
        ibuf[3] = net->Ntanks;
        ibuf[4] = net->Nlinks;
        ibuf[5] = net->Npumps;
        ibuf[6] = net->Nvalves;
        ibuf[7] = qual->Qualflag;
        ibuf[8] = qual->TraceNode;
        ibuf[9] = parser->Flowflag;
        ibuf[10] = parser->Pressflag;
        ibuf[11] = rpt->Tstatflag;
        ibuf[12] = (INT4)time->Rstart;
        ibuf[13] = (INT4)time->Rstep;
        ibuf[14] = (INT4)time->Dur;
        fwrite(ibuf, sizeof(INT4), 15, outFile);

        // Write string variables to outFile
        fwrite(pr->Title[0], sizeof(char), TITLELEN + 1, outFile);
        fwrite(pr->Title[1], sizeof(char), TITLELEN + 1, outFile);
        fwrite(pr->Title[2], sizeof(char), TITLELEN + 1, outFile);
        fwrite(parser->InpFname, sizeof(char), MAXFNAME + 1, outFile);
        fwrite(rpt->Rpt2Fname, sizeof(char), MAXFNAME + 1, outFile);
        fwrite(qual->ChemName, sizeof(char), MAXID + 1, outFile);
        fwrite(rpt->Field[QUALITY].Units, sizeof(char), MAXID + 1, outFile);

        // Write node ID information to outFile
        for (i = 1; i <= net->Nnodes; i++)
        {
            node = &net->Node[i];
            fwrite(node->ID, MAXID + 1, 1, outFile);
        }

        // Write link information to outFile
        // (Note: first transfer values to buffer array,
        // then fwrite buffer array at offset of 1 )
        for (i = 1; i <= net->Nlinks; i++)
        {
            fwrite(net->Link[i].ID, MAXID + 1, 1, outFile);
        }

        for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i].N1;
        fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

        for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i].N2;
        fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

        for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i].Type;
        fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

        // Write tank information to outFile
        for (i = 1; i <= net->Ntanks; i++) ibuf[i] = net->Tank[i].Node;
        fwrite(ibuf + 1, sizeof(INT4), net->Ntanks, outFile);

        for (i = 1; i <= net->Ntanks; i++) x[i] = (REAL4)net->Tank[i].A;
        f_save(x, net->Ntanks, outFile);

        // Save node elevations to outFile
        for (i = 1; i <= net->Nnodes; i++)
        {
            x[i] = (REAL4)(net->Node[i].El * pr->Ucf[ELEV]);
        }
        f_save(x, net->Nnodes, outFile);

        // Save link lengths & diameters to outFile
        for (i = 1; i <= net->Nlinks; i++)
        {
            x[i] = (REAL4)(net->Link[i].Len * pr->Ucf[ELEV]);
        }
        f_save(x, net->Nlinks, outFile);

        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].Type != PUMP)
            {
                x[i] = (REAL4)(net->Link[i].Diam * pr->Ucf[DIAM]);
            }
            else x[i] = 0.0f;
        }
        if (f_save(x, net->Nlinks, outFile) < (unsigned)net->Nlinks) errcode = 308;
    }

    // Free memory used for buffer arrays
    free(ibuf);
    free(x);
    return errcode;
}

int savehyd(Project *pr, long *htime)
/*
**--------------------------------------------------------------
**   Input:   *htime   = current time
**   Output:  returns error code
**   Purpose: saves current hydraulic solution to file HydFile
**            in binary format
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Outfile *out = &pr->outfile;
    Hydraul *hyd = &pr->hydraul;

    int i;
    INT4 t;
    int errcode = 0;
    REAL4 *x;
    FILE  *HydFile = out->HydFile;

    x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
    if (x == NULL) return 101;

    // Save current time (htime)
    t = (INT4)(*htime);
    fwrite(&t, sizeof(INT4), 1, HydFile);

    // Save current nodal demands (D)
    for (i = 1; i <= net->Nnodes; i++) x[i] = (REAL4)hyd->NodeDemand[i];
    fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);
    //f_save(x, net->Nnodes, HydFile);

    // Save current nodal heads
    for (i = 1; i <= net->Nnodes; i++) x[i] = (REAL4)hyd->NodeHead[i];
    fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);
    //f_save(x, net->Nnodes, HydFile);

    // Force flow in closed links to be zero then save flows
    for (i = 1; i <= net->Nlinks; i++)
    {
        if (hyd->LinkStatus[i] <= CLOSED) x[i] = 0.0f;
        else x[i] = (REAL4)hyd->LinkFlow[i];
    }
    fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);
    //f_save(x, net->Nlinks, HydFile);

    // Save link status
    for (i = 1; i <= net->Nlinks; i++) x[i] = (REAL4)hyd->LinkStatus[i];
    fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);
    //f_save(x, net->Nlinks, HydFile);

    // Save link settings & check for successful write-to-disk
    // (We assume that if any of the previous fwrites failed,
    // then this one will also fail.)
    for (i = 1; i <= net->Nlinks; i++) x[i] = (REAL4)hyd->LinkSetting[i];
    if (fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile) <
        (unsigned)net->Nlinks
       ) errcode = 308;
    //if (f_save(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) errcode = 308;
    free(x);
    fflush(HydFile);
    return errcode;
}

int savehydstep(Project *pr, long *hydstep)
/*
**--------------------------------------------------------------
**   Input:   *hydstep = next time step
**   Output:  returns error code
**   Purpose: saves next hydraulic timestep to file HydFile
**            in binary format
**--------------------------------------------------------------
*/
{
    Outfile *out = &pr->outfile;

    INT4 t;
    int errcode = 0;

    t = (INT4)(*hydstep);
    if (fwrite(&t, sizeof(INT4), 1, out->HydFile) < 1) errcode = 308;
    if (t == 0) fputc(EOFMARK, out->HydFile);
    fflush(out->HydFile);
    return errcode;
}

int saveenergy(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: saves energy usage by each pump to outFile
**            in binary format
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Outfile *out = &pr->outfile;
    Parser  *parser = &pr->parser;
    Times   *time = &pr->times;

    int    i;
    INT4   index;
    REAL4  x[6];                // work array
    double hdur,               // total simulation duration in hours
           t;                  // total pumping time duration
    Spump *pump;
    FILE  *outFile = out->OutFile;

    hdur = time->Dur / 3600.0;
    for (i = 1; i <= net->Npumps; i++)
    {
        pump = &net->Pump[i];
        if (hdur == 0.0) pump->Energy.TotalCost *= 24.0;
        else
        {
            // ... convert total hrs. online to fraction of total time online
            t = pump->Energy.TimeOnLine;  //currently holds total hrs. online
            pump->Energy.TimeOnLine = t / hdur;

            // ... convert cumulative values to time-averaged ones
            if (t > 0.0)
            {
                pump->Energy.Efficiency /= t;
                pump->Energy.KwHrsPerFlow /= t;
                pump->Energy.KwHrs /= t;
            }

            // ... convert total cost to cost per day
            pump->Energy.TotalCost *= 24.0 / hdur;
        }

        // ... express time online and avg. efficiency as percentages
        pump->Energy.TimeOnLine *= 100.0;
        pump->Energy.Efficiency *= 100.0;

        // ... compute KWH per Million Gallons or per Cubic Meter
        if (parser->Unitsflag == SI)
        {
            pump->Energy.KwHrsPerFlow *= (1000. / LPSperCFS / 3600.);
        }
        else pump->Energy.KwHrsPerFlow *= (1.0e6 / GPMperCFS / 60.);

        // ... save energy stats to REAL4 work array
        x[0] = (REAL4)pump->Energy.TimeOnLine;
        x[1] = (REAL4)pump->Energy.Efficiency;
        x[2] = (REAL4)pump->Energy.KwHrsPerFlow;
        x[3] = (REAL4)pump->Energy.KwHrs;
        x[4] = (REAL4)pump->Energy.MaxKwatts;
        x[5] = (REAL4)pump->Energy.TotalCost;

        // ... save energy results to output file
        index = pump->Link;
        if (fwrite(&index, sizeof(INT4), 1, outFile) < 1) return 308;
        if (fwrite(x, sizeof(REAL4), 6, outFile) < 6) return 308;
    }

    // ... compute and save demand charge
    hyd->Emax = hyd->Emax * hyd->Dcost;
    x[0] = (REAL4)hyd->Emax;
    if (fwrite(&x[0], sizeof(REAL4), 1, outFile) < 1) return 308;
    return (0);
}

int readhyd(Project *pr, long *hydtime)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Outfile *out = &pr->outfile;

    int i;
    INT4 t;
    int result = 1;
    REAL4 *x;
    FILE *HydFile = out->HydFile;

    x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
    if (x == NULL) return 0;

    if (fread(&t, sizeof(INT4), 1, HydFile) < 1) result = 0;
    *hydtime = t;

    if (f_read(x, net->Nnodes, HydFile) < (unsigned)net->Nnodes) result = 0;
    else for (i = 1; i <= net->Nnodes; i++) hyd->NodeDemand[i] = x[i];

    if (f_read(x, net->Nnodes, HydFile) < (unsigned)net->Nnodes) result = 0;
    else for (i = 1; i <= net->Nnodes; i++) hyd->NodeHead[i] = x[i];

    if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
    else for (i = 1; i <= net->Nlinks; i++) hyd->LinkFlow[i] = x[i];

    if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
    else for (i = 1; i <= net->Nlinks; i++) hyd->LinkStatus[i] = (char)x[i];

    if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
    else for (i = 1; i <= net->Nlinks; i++) hyd->LinkSetting[i] = x[i];

    free(x);
    return result;
}

int readhydstep(Project *pr, long *hydstep)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  *hydstep = next hydraulic time step (sec)
**   Returns: 1 if successful, 0 if not
**   Purpose: reads hydraulic time step from file HydFile
**--------------------------------------------------------------
*/
{
    FILE *hydFile = pr->outfile.HydFile;
    INT4 t;

    if (fread(&t, sizeof(INT4), 1, hydFile) < 1) return 0;
    *hydstep = t;
    return 1;
}

int saveoutput(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: writes simulation results to output file
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int j;
    int errcode = 0;
    REAL4 *x;

    x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
    if (x == NULL) return 101;

    // Write out node results, then link results
    for (j = DEMAND; j <= QUALITY; j++) ERRCODE(nodeoutput(pr, j, x, pr->Ucf[j]));
    for (j = FLOW; j <= FRICTION; j++) ERRCODE(linkoutput(pr, j, x, pr->Ucf[j]));
    free(x);
    return errcode;
}

int nodeoutput(Project *pr, int j, REAL4 *x, double ucf)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Outfile *out = &pr->outfile;

    int i;
    FILE *outFile = out->TmpOutFile;

    // Load computed results (in proper units) into buffer x
    switch (j)
    {
      case DEMAND:
        for (i = 1; i <= net->Nnodes; i++)
        {
            x[i] = (REAL4)(hyd->NodeDemand[i] * ucf);
        }
        break;

      case HEAD:
        for (i = 1; i <= net->Nnodes; i++)
        {
            x[i] = (REAL4)(hyd->NodeHead[i] * ucf);
        }
        break;

      case PRESSURE:
        for (i = 1; i <= net->Nnodes; i++)
        {
            x[i] = (REAL4)((hyd->NodeHead[i] - net->Node[i].El) * ucf);
        }
        break;

      case QUALITY:
        for (i = 1; i <= net->Nnodes; i++)
        {
            x[i] = (REAL4)(qual->NodeQual[i] * ucf);
        }
    }

    // Write x[1] to x[net->Nnodes] to output file
    if (f_save(x, net->Nnodes, outFile) < (unsigned)net->Nnodes) return 308;
    return 0;
}

int linkoutput(Project *pr, int j, REAL4 *x, double ucf)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Outfile *out = &pr->outfile;

    int i;
    double a, h, q, f, setting;
    FILE *outFile = out->TmpOutFile;

    // Load computed results (in proper units) into buffer x
    switch (j)
    {
      case FLOW:
        for (i = 1; i <= net->Nlinks; i++)
        {
            x[i] = (REAL4)(hyd->LinkFlow[i] * ucf);
        }
        break;

      case VELOCITY:
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].Type == PUMP) x[i] = 0.0f;
            else
            {
                q = ABS(hyd->LinkFlow[i]);
                a = PI * SQR(net->Link[i].Diam) / 4.0;
                x[i] = (REAL4)(q / a * ucf);
            }
        }
        break;

      case HEADLOSS:
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (hyd->LinkStatus[i] <= CLOSED) x[i] = 0.0f;
            else
            {
                h = hyd->NodeHead[net->Link[i].N1] -
                    hyd->NodeHead[net->Link[i].N2];
                if (net->Link[i].Type != PUMP) h = ABS(h);
                if (net->Link[i].Type <= PIPE)
                {
                    x[i] = (REAL4)(1000.0 * h / net->Link[i].Len);
                }
                else x[i] = (REAL4)(h * ucf);
            }
        }
        break;

      case LINKQUAL:
        for (i = 1; i <= net->Nlinks; i++)
        {
            x[i] = (REAL4)(avgqual(pr,i) * ucf);
        }
        break;

      case STATUS:
        for (i = 1; i <= net->Nlinks; i++)
        {
            x[i] = (REAL4)hyd->LinkStatus[i];
        }
        break;

      case SETTING:
        for (i = 1; i <= net->Nlinks; i++)
        {
            setting = hyd->LinkSetting[i];
            if (setting != MISSING) switch (net->Link[i].Type)
            {
              case CVPIPE:
              case PIPE:
                x[i] = (REAL4)setting; break;
              case PUMP:
                x[i] = (REAL4)setting; break;
              case PRV:
              case PSV:
              case PBV:
                x[i] = (REAL4)(setting * pr->Ucf[PRESSURE]); break;
              case FCV:
                x[i] = (REAL4)(setting * pr->Ucf[FLOW]); break;
              case TCV:
                x[i] = (REAL4)setting; break;
              default: x[i] = 0.0f;
            }
            else x[i] = 0.0f;
        }
        break;

      case REACTRATE: // Overall reaction rate in mass/L/day
        if (qual->Qualflag == NONE)
        {
            memset(x, 0, (net->Nlinks + 1) * sizeof(REAL4));
        }
        else for (i = 1; i <= net->Nlinks; i++)
        {
            x[i] = (REAL4)(qual->PipeRateCoeff[i] * ucf);
        }
        break;

      case FRICTION:  // Friction factor
        // f = 2ghd/(Lu^2) where f = friction factor
        // u = velocity, g = grav. accel., h = head loss
        //loss, d = diam., & L = pipe length
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].Type <= PIPE && ABS(hyd->LinkFlow[i]) > TINY)
            {
                h = ABS(hyd->NodeHead[net->Link[i].N1] -
                        hyd->NodeHead[net->Link[i].N2]);
                f = 39.725 * h * pow(net->Link[i].Diam, 5) /
                    net->Link[i].Len / SQR(hyd->LinkFlow[i]);
                x[i] = (REAL4)f;
            }
            else x[i] = 0.0f;
        }
        break;
    }

    // Write x[1] to x[net->Nlinks] to output file
    if (f_save(x, net->Nlinks, outFile) < (unsigned)net->Nlinks) return 308;
    return 0;
}

int savefinaloutput(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: saves time series statistics, reaction rates &
**            epilog to output file.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Outfile *out = &pr->outfile;
    Report  *rpt = &pr->report;
    Quality *qual = &pr->quality;

    int errcode = 0;
    REAL4 *x;
    FILE *outFile = out->OutFile;

    // Save time series statistic if computed
    if (rpt->Tstatflag != SERIES && out->TmpOutFile != NULL)
    {
        x = (REAL4 *)calloc(MAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
        if (x == NULL) return 101;
        ERRCODE(savetimestat(pr, x, NODEHDR));
        ERRCODE(savetimestat(pr, x, LINKHDR));
        if (!errcode) rpt->Nperiods = 1;
        fclose(out->TmpOutFile);
        out->TmpOutFile = NULL;
        free(x);
    }

    // Save avg. reaction rates & file epilog
    if (outFile != NULL)
    {
        ERRCODE(savenetreacts(pr, qual->Wbulk, qual->Wwall, qual->Wtank,
                              qual->Wsource));
        ERRCODE(saveepilog(pr));
    }
    return errcode;
}

int savetimestat(Project *pr, REAL4 *x, HdrType objtype)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Outfile *out = &pr->outfile;
    Report  *rpt = &pr->report;

    int n, n1, n2;
    int i, j, p, errcode = 0;
    long startbyte, skipbytes;
    float *stat1, *stat2, xx;
    FILE *outFile = out->OutFile;

    // Compute number of bytes in temp output file to skip over (skipbytes)
    // when moving from one time period to the next for a particular variable
    if (objtype == NODEHDR)
    {
        // For nodes, we start at 0 and skip over node output for all
        // node variables minus 1 plus link output for all link variables.
        startbyte = 0;
        skipbytes = (net->Nnodes * (QUALITY - DEMAND) +
                     net->Nlinks * (FRICTION - FLOW + 1)) * sizeof(REAL4);
        n = net->Nnodes;
        n1 = DEMAND;
        n2 = QUALITY;
    }
    else
    {
        // For links, we start at the end of all node variables and skip
        // over node output for all node variables plus link output for
        // all link variables minus 1
        startbyte = net->Nnodes * (QUALITY - DEMAND + 1) * sizeof(REAL4);
        skipbytes = (net->Nnodes * (QUALITY - DEMAND + 1) +
                     net->Nlinks * (FRICTION - FLOW)) * sizeof(REAL4);
        n = net->Nlinks;
        n1 = FLOW;
        n2 = FRICTION;
    }
    stat1 = (float *)calloc(n + 1, sizeof(float));
    stat2 = (float *)calloc(n + 1, sizeof(float));
    ERRCODE(MEMCHECK(stat1));
    ERRCODE(MEMCHECK(stat2));

    // Process each output reporting variable
    if (!errcode) for (j = n1; j <= n2; j++)
    {
        // Initialize stat arrays
        if (rpt->Tstatflag == AVG) memset(stat1, 0, (n + 1) * sizeof(float));
        else for (i = 1; i <= n; i++)
        {
            stat1[i] = -MISSING;
            stat2[i] = MISSING;
        }

        // Position temp output file at start of output
        fseek(out->TmpOutFile, startbyte + (j - n1) * n * sizeof(REAL4),
              SEEK_SET);

        // Process each time period
        for (p = 1; p <= rpt->Nperiods; p++)
        {
            // Get output results for time period & update stats
            f_read(x, n, out->TmpOutFile);
            for (i = 1; i <= n; i++)
            {
                xx = x[i];
                if (objtype == LINKHDR)
                {
                    if (j == FLOW) xx = ABS(xx);
                    if (j == STATUS)
                    {
                        if (xx >= OPEN) xx = 1.0;
                        else            xx = 0.0;
                    }
                }
                if (rpt->Tstatflag == AVG) stat1[i] += xx;
                else
                {
                    stat1[i] = MIN(stat1[i], xx);
                    stat2[i] = MAX(stat2[i], xx);
                }
            }

            // Advance file to next period
            if (p < rpt->Nperiods) fseek(out->TmpOutFile, skipbytes, SEEK_CUR);
        }

        // Compute resultant stat & save to regular output file
        switch (rpt->Tstatflag)
        {
          case AVG:
            for (i = 1; i <= n; i++) x[i] = stat1[i] / (float)rpt->Nperiods;
            break;
          case MIN:
            for (i = 1; i <= n; i++) x[i] = stat1[i];
            break;
          case MAX:
            for (i = 1; i <= n; i++) x[i] = stat2[i];
            break;
          case RANGE:
            for (i = 1; i <= n; i++) x[i] = stat2[i] - stat1[i];
            break;
        }
        if (objtype == LINKHDR && j == STATUS)
        {
            for (i = 1; i <= n; i++)
            {
                if (x[i] < 0.5f) x[i] = CLOSED;
                else             x[i] = OPEN;
            }
        }
        if (f_save(x, n, outFile) < (unsigned)n) errcode = 308;

        // Update internal output variables where applicable
        if (objtype == NODEHDR) switch (j)
        {
          case DEMAND:
            for (i = 1; i <= n; i++) hyd->NodeDemand[i] = x[i] / pr->Ucf[DEMAND];
            break;
          case HEAD:
            for (i = 1; i <= n; i++) hyd->NodeHead[i] = x[i] / pr->Ucf[HEAD];
            break;
          case QUALITY:
            for (i = 1; i <= n; i++) qual->NodeQual[i] = x[i] / pr->Ucf[QUALITY];
            break;
        }
        else if (j == FLOW)
        {
            for (i = 1; i <= n; i++) hyd->LinkFlow[i] = x[i] / pr->Ucf[FLOW];
        }
    }

    // Free allocated memory
    free(stat1);
    free(stat2);
    return errcode;
}

int savenetreacts(Project *pr, double wbulk, double wwall, double wtank, double wsource)
/*
**-----------------------------------------------------
**  Writes average network-wide reaction rates (in
**  mass/hr) to binary output file.
**-----------------------------------------------------
*/
{
    Outfile *out = &pr->outfile;
    Times   *time = &pr->times;

    int errcode = 0;
    double t;
    REAL4 w[4];
    FILE *outFile = out->OutFile;

    if (time->Dur > 0) t = (double)time->Dur / 3600.;
    else t = 1.;
    w[0] = (REAL4)(wbulk / t);
    w[1] = (REAL4)(wwall / t);
    w[2] = (REAL4)(wtank / t);
    w[3] = (REAL4)(wsource / t);
    if (fwrite(w, sizeof(REAL4), 4, outFile) < 4) errcode = 308;
    return errcode;
}

int saveepilog(Project *pr)
/*
**-------------------------------------------------
**  Writes Nperiods, Warnflag, & Magic Number to
**  end of binary output file.
**-------------------------------------------------
*/
{
    Outfile *out = &pr->outfile;
    Report  *rpt = &pr->report;

    int errcode = 0;
    INT4 i;
    FILE *outFile = out->OutFile;

    i = rpt->Nperiods;
    if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
    i = pr->Warnflag;
    if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
    i = MAGICNUMBER; if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
    return errcode;
}
