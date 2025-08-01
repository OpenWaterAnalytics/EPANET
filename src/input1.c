/*
******************************************************************************
Project:      OWA EPANET
Version:      2.3
Module:       input1.c
Description:  retrieves network data from an EPANET input file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 04/19/2025
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

// Default values
#define MAXITER  200  // Default max. # hydraulic iterations
#define HACC 0.001    // Default hydraulics convergence ratio
#define HTOL 0.0005   // Default hydraulic head tolerance (ft)
#define QTOL 0.0001   // Default flow rate tolerance (cfs)
#define AGETOL 0.01   // Default water age tolerance (hrs)
#define CHEMTOL 0.01  // Default concentration tolerance
#define PAGESIZE 0    // Default uses no page breaks
#define SPGRAV 1.0    // Default specific gravity
#define EPUMP 75      // Default pump efficiency
#define DEFPATID "1"  // Default demand pattern ID
#define RQTOL 1E-7    // Default low flow resistance tolerance
#define CHECKFREQ 2   // Default status check frequency
#define MAXCHECK 10   // Default # iterations for status checks
#define DAMPLIMIT 0   // Default damping threshold

// Defined in ENUMSTXT.H
extern char *Fldname[];
extern char *RptFlowUnitsTxt[];
extern char *PressUnitsTxt[];
extern void reindextanks(Project *pr);


int getdata(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: reads in network data from disk file
**----------------------------------------------------------------
*/
{
    int errcode = 0;

    // Assign default data values & reporting options
    setdefaults(pr);
    initreport(&pr->report);

    // Read in network data
    rewind(pr->parser.InFile);
    errcode = readdata(pr);
    
    // Adjust data and convert it to internal units
    // (error code 200 means there are non-fatal errors in input file)
    if (errcode == 0 || errcode == 200)
    {
        reindextanks(pr);
        adjustdata(pr);
        inittanks(pr);
        initunits(pr);
        convertunits(pr);
    }
    return errcode;
}

void setdefaults(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: assigns default values to a project's variables
**----------------------------------------------------------------
*/
{
    Parser  *parser = &pr->parser;
    Report  *rpt = &pr->report;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;
    Outfile *out = &pr->outfile;

    strncpy(pr->Title[0], "", TITLELEN);
    strncpy(pr->Title[1], "", TITLELEN);
    strncpy(pr->Title[2], "", TITLELEN);
    strncpy(out->HydFname, "", MAXFNAME);
    strncpy(pr->MapFname, "", MAXFNAME);
    strncpy(qual->ChemName, t_CHEMICAL, MAXID);
    strncpy(qual->ChemUnits, u_MGperL, MAXID);
    strncpy(parser->DefPatID, DEFPATID, MAXID);

    pr->Warnflag = FALSE;       // Warning flag is off
    parser->Unitsflag = US;     // US unit system
    parser->Flowflag = GPM;     // Flow units are gpm
    parser->Pressflag = DEFAULTUNIT; // Pressure units set based on unit system
    out->Hydflag = SCRATCH;     // No external hydraulics file
    rpt->Tstatflag = SERIES;    // Generate time series output

    hyd->Formflag = HW;         // Use Hazen-Williams formula
    hyd->Htol = HTOL;           // Default head tolerance
    hyd->Qtol = QTOL;           // Default flow tolerance
    hyd->Hacc = HACC;           // Default hydraulic accuracy
    hyd->FlowChangeLimit = 0.0; // Default flow change limit
    hyd->HeadErrorLimit = 0.0;  // Default head error limit
    hyd->DemandModel = DDA;     // Demand driven analysis
    hyd->Pmin = 0.0;            // Minimum demand pressure (ft)
    hyd->Preq = MINPDIFF;       // Required demand pressure (ft)
    hyd->Pexp = 0.5;            // Pressure function exponent
    hyd->MaxIter = MAXITER;     // Default max. hydraulic trials
    hyd->ExtraIter = -1;        // Stop if network unbalanced
    hyd->Viscos = MISSING;      // Temporary viscosity
    hyd->SpGrav = SPGRAV;       // Default specific gravity
    hyd->Epat = 0;              // No energy price pattern
    hyd->Ecost = 0.0;           // Zero unit energy cost
    hyd->Dcost = 0.0;           // Zero energy demand charge
    hyd->Epump = EPUMP;         // Default pump efficiency
    hyd->Emax = 0.0;            // Zero peak energy usage
    hyd->Qexp = 2.0;            // Flow exponent for emitters
    hyd->EmitBackFlag = 1;      // Allow emitter backflow
    hyd->DefPat = 0;            // Default demand pattern index
    hyd->Dmult = 1.0;           // Demand multiplier
    hyd->RQtol = RQTOL;         // Default hydraulics parameters
    hyd->CheckFreq = CHECKFREQ;
    hyd->MaxCheck = MAXCHECK;
    hyd->DampLimit = DAMPLIMIT;

    qual->Qualflag = NONE;      // No quality simulation
    qual->Ctol = MISSING;       // No pre-set quality tolerance
    qual->TraceNode = 0;        // No source tracing
    qual->BulkOrder = 1.0;      // 1st-order bulk reaction rate
    qual->WallOrder = 1.0;      // 1st-order wall reaction rate
    qual->TankOrder = 1.0;      // 1st-order tank reaction rate
    qual->Kbulk = 0.0;          // No global bulk reaction
    qual->Kwall = 0.0;          // No global wall reaction
    qual->Climit = 0.0;         // No limiting potential quality
    qual->Diffus = MISSING;     // Temporary diffusivity
    qual->Rfactor = 0.0;        // No roughness-reaction factor
    qual->MassBalance.ratio = 0.0;

    time->Dur = 0;              // 0 sec duration (steady state)
    time->Tstart = 0;           // Starting time of day
    time->Pstart = 0;           // Starting pattern period
    time->Hstep = 3600;         // 1 hr hydraulic time step
    time->Qstep = 0;            // No pre-set quality time step
    time->Pstep = 3600;         // 1 hr time pattern period
    time->Rstep = 3600;         // 1 hr reporting period
    time->Rulestep = 0;         // No pre-set rule time step
    time->Rstart = 0;           // Start reporting at time 0
}

void initreport(Report *rpt)
/*
**----------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes reporting options
**----------------------------------------------------------------------
*/
{
    int i;
    strncpy(rpt->Rpt2Fname, "", MAXFNAME);

    // Initialize general reporting options
    rpt->PageSize = PAGESIZE;      // Default page size for report
    rpt->Summaryflag = TRUE;       // Write summary report
    rpt->Messageflag = TRUE;       // Report error/warning messages
    rpt->Statflag = FALSE;         // No hydraulic status reports
    rpt->Energyflag = FALSE;       // No energy usage report
    rpt->Nodeflag = 0;             // No reporting on nodes
    rpt->Linkflag = 0;             // No reporting on links

    // Initialize options for each reported variable field
    for (i = 0; i < MAXVAR; i++)
    {
        strncpy(rpt->Field[i].Name, Fldname[i], MAXID);
        rpt->Field[i].Enabled = FALSE;        // Not included in report
        rpt->Field[i].Precision = 2;          // 2 decimal precision
        rpt->Field[i].RptLim[LOW] = SQR(BIG); // No reporting limits
        rpt->Field[i].RptLim[HI] = -SQR(BIG);
    }
    rpt->Field[FRICTION].Precision = 3;

    // Set default set of variables reported on
    for (i = DEMAND; i <= QUALITY; i++)
    {
        rpt->Field[i].Enabled = TRUE;
    }
    for (i = FLOW; i <= HEADLOSS; i++)
    {
        rpt->Field[i].Enabled = TRUE;
    }
}

void adjustdata(Project *pr)
/*
**----------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: adjusts project data after input file has been processed
**----------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;
    Parser  *parser = &pr->parser;
    Report  *rpt = &pr->report;

    int i;
    double ucf;     // Unit conversion factor
    Slink *link;
    Stank *tank;

    // Use 1 hr pattern & report time step if none specified
    if (time->Pstep <= 0) time->Pstep = 3600;
    if (time->Rstep == 0) time->Rstep = time->Pstep;

    // Hydraulic time step cannot be greater than pattern or report time step
    if (time->Hstep <= 0) time->Hstep = 3600;
    if (time->Hstep > time->Pstep) time->Hstep = time->Pstep;
    if (time->Hstep > time->Rstep) time->Hstep = time->Rstep;

    // Report start time cannot be greater than simulation duration
    if (time->Rstart > time->Dur) time->Rstart = 0;

    // If no quality timestep, then make it 1/10 of hydraulic timestep
    if (time->Qstep == 0) time->Qstep = time->Hstep / 10;

    // If no rule time step then make it 1/10 of hydraulic time step
    // but not greater than hydraulic time step
    if (time->Rulestep == 0) time->Rulestep = time->Hstep / 10;
    time->Rulestep = MIN(time->Rulestep, time->Hstep);

    // Quality timestep cannot exceed hydraulic timestep
    time->Qstep = MIN(time->Qstep, time->Hstep);

    // If no quality tolerance, then use default values
    if (qual->Ctol == MISSING)
    {
        if (qual->Qualflag == AGE) qual->Ctol = AGETOL;
        else qual->Ctol = CHEMTOL;
    }

    // Determine units system based on flow units
    switch (parser->Flowflag)
    {
      case LPS: // Liters/sec
      case LPM: // Liters/min
      case MLD: // megaliters/day
      case CMH: // cubic meters/hr
      case CMD: // cubic meters/day
      case CMS: // cubic meters/second
        parser->Unitsflag = SI;
        break;
      default:
        parser->Unitsflag = US;
    }

    // Revise pressure units depending on flow units
    if (parser->Pressflag == DEFAULTUNIT)
    {
        if (parser->Unitsflag == SI) parser->Pressflag = METERS;
        else parser->Pressflag = PSI;
    }
    
    // Store value of viscosity & diffusivity
    ucf = 1.0;
    if (parser->Unitsflag == SI) ucf = SQR(MperFT);
    if (hyd->Viscos == MISSING)
    {
        hyd->Viscos = VISCOS;               // No viscosity supplied
    }
    else if (hyd->Viscos > 1.e-3)
    {
        hyd->Viscos = hyd->Viscos * VISCOS; // Multiplier supplied
    }
    else hyd->Viscos = hyd->Viscos / ucf;   // Actual value supplied
    if (qual->Diffus == MISSING)
    {
        qual->Diffus = DIFFUS;              // No viscosity supplied
    }
    else if (qual->Diffus > 1.e-4)
    {
        qual->Diffus = qual->Diffus * DIFFUS; // Multiplier supplied
    }
    else qual->Diffus = qual->Diffus / ucf;   //  Actual value supplied

    // Set exponent in head loss equation and adjust flow-resistance tolerance.
    if (hyd->Formflag == HW) hyd->Hexp = 1.852;
    else hyd->Hexp = 2.0;

    // See if default reaction coeffs. apply
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        if (link->Type > PIPE) continue;
        if (link->Kb == MISSING) link->Kb = qual->Kbulk;   // Bulk coeff.
        if (link->Kw == MISSING)                           // Wall coeff.
        {
            // Rfactor is the pipe roughness correlation factor
            if (qual->Rfactor == 0.0) link->Kw = qual->Kwall;
            else if ((link->Kc > 0.0) && (link->Diam > 0.0))
            {
                if (hyd->Formflag == HW) link->Kw = qual->Rfactor / link->Kc;
                if (hyd->Formflag == DW)
                {
                    link->Kw = qual->Rfactor / ABS(log(link->Kc / link->Diam));
                }
                if (hyd->Formflag == CM) link->Kw = qual->Rfactor * link->Kc;
            }
            else link->Kw = 0.0;
        }
    }
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        if (tank->Kb == MISSING) tank->Kb = qual->Kbulk;
    }
 
    // Set default pattern index
    i = findpattern(net, parser->DefPatID);
    if (i > 0)
        hyd->DefPat = i;

    // Remove QUALITY as a reporting variable if no WQ analysis
    if (qual->Qualflag == NONE) rpt->Field[QUALITY].Enabled = FALSE;
}

void inittanks(Project *pr)
/*
**---------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes volumes in non-cylindrical tanks
**---------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int i, j, n = 0;
    double a;
    int errcode = 0;
    char errmsg[MAXMSG+1] = "";
    Stank *tank;
    Scurve *curve;

    for (j = 1; j <= net->Ntanks; j++)
    {
        tank = &net->Tank[j];
        if (tank->A == 0.0) continue;  // Skip reservoirs

        // See if tank has a volume curve
        i = tank->Vcurve;
        if (i > 0)
        {
            curve = &net->Curve[i];
            n = curve->Npts - 1;

            // Find min., max., and initial volumes from curve
            tank->Vmin = interp(curve->Npts, curve->X, curve->Y, tank->Hmin);
            tank->Vmax = interp(curve->Npts, curve->X, curve->Y, tank->Hmax);
            tank->V0 = interp(curve->Npts, curve->X, curve->Y, tank->H0);

            // Find a "nominal" diameter for tank
            a = (curve->Y[n] - curve->Y[0]) / (curve->X[n] - curve->X[0]);
            tank->A = sqrt(4.0 * a / PI);
        }
    }
}

void initunits(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: determines unit conversion factors
**--------------------------------------------------------------
*/
{
    Parser  *parser = &pr->parser;
    Report  *rpt = &pr->report;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    double dcf,    // distance conversion factor
           ccf,    // concentration conversion factor
           qcf,    // flow conversion factor
           hcf,    // head conversion factor
           pcf,    // pressure conversion factor
           wcf;    // energy conversion factor

    if (parser->Unitsflag == SI)  // SI units
    {
        strcpy(rpt->Field[DEMAND].Units, RptFlowUnitsTxt[parser->Flowflag]);
        strcpy(rpt->Field[ELEV].Units, u_METERS);
        strcpy(rpt->Field[HEAD].Units, u_METERS);
        strcpy(rpt->Field[LENGTH].Units, u_METERS);
        strcpy(rpt->Field[DIAM].Units, u_MMETERS);
        strcpy(rpt->Field[FLOW].Units, RptFlowUnitsTxt[parser->Flowflag]);
        strcpy(rpt->Field[VELOCITY].Units, u_MperSEC);
        strcpy(rpt->Field[HEADLOSS].Units, u_per1000M);
        strcpy(rpt->Field[FRICTION].Units, "");
        strcpy(rpt->Field[POWER].Units, u_KW);

        dcf = 1000.0 * MperFT;
        qcf = LPSperCFS;
        if (parser->Flowflag == LPM) qcf = LPMperCFS;
        if (parser->Flowflag == MLD) qcf = MLDperCFS;
        if (parser->Flowflag == CMH) qcf = CMHperCFS;
        if (parser->Flowflag == CMD) qcf = CMDperCFS;
        if (parser->Flowflag == CMS) qcf = CMSperCFS;

        hcf = MperFT;
        wcf = KWperHP;
    }
    else  // US units
    {
        strcpy(rpt->Field[DEMAND].Units, RptFlowUnitsTxt[parser->Flowflag]);
        strcpy(rpt->Field[ELEV].Units, u_FEET);
        strcpy(rpt->Field[HEAD].Units, u_FEET);
        strcpy(rpt->Field[LENGTH].Units, u_FEET);
        strcpy(rpt->Field[DIAM].Units, u_INCHES);
        strcpy(rpt->Field[FLOW].Units, RptFlowUnitsTxt[parser->Flowflag]);
        strcpy(rpt->Field[VELOCITY].Units, u_FTperSEC);
        strcpy(rpt->Field[HEADLOSS].Units, u_per1000FT);
        strcpy(rpt->Field[FRICTION].Units, "");
        strcpy(rpt->Field[POWER].Units, u_HP);

        dcf = 12.0;
        qcf = 1.0;
        if (parser->Flowflag == GPM) qcf = GPMperCFS;
        if (parser->Flowflag == MGD) qcf = MGDperCFS;
        if (parser->Flowflag == IMGD) qcf = IMGDperCFS;
        if (parser->Flowflag == AFD)  qcf = AFDperCFS;
        hcf = 1.0;
        wcf = 1.0;
    }

    strcpy(rpt->Field[PRESSURE].Units, PressUnitsTxt[parser->Pressflag]);
    pcf = PSIperFT * hyd->SpGrav; // Default to PSI
    if (parser->Pressflag == METERS) pcf = MperFT;
    if (parser->Pressflag == KPA)    pcf = KPAperPSI * PSIperFT * hyd->SpGrav;
    if (parser->Pressflag == BAR)    pcf = BARperPSI * PSIperFT * hyd->SpGrav;
    if (parser->Pressflag == FEET)   pcf = 1.0;

    strcpy(rpt->Field[QUALITY].Units, "");
    ccf = 1.0;
    if (qual->Qualflag == CHEM)
    {
        ccf = 1.0 / LperFT3;
        strncpy(rpt->Field[QUALITY].Units, qual->ChemUnits, MAXID);
        strncpy(rpt->Field[REACTRATE].Units, qual->ChemUnits, MAXID);
        strcat(rpt->Field[REACTRATE].Units, t_PERDAY);
    }
    else if (qual->Qualflag == AGE) strcpy(rpt->Field[QUALITY].Units, u_HOURS);
    else if (qual->Qualflag == TRACE) strcpy(rpt->Field[QUALITY].Units, u_PERCENT);

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

    // Report time in minutes if hyd. time step < 1/2 hr.
    if (time->Hstep < 1800)
    {
        pr->Ucf[TIME] = 1.0 / 60.0;
        strcpy(rpt->Field[TIME].Units, u_MINUTES);
    }
    else
    {
        pr->Ucf[TIME] = 1.0 / 3600.0;
        strcpy(rpt->Field[TIME].Units, u_HOURS);
    }
}

void convertunits(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: converts units of input data
**--------------------------------------------------------------
*/
{
    Network  *net = &pr->network;
    Hydraul  *hyd = &pr->hydraul;
    Quality  *qual = &pr->quality;
    Parser   *parser = &pr->parser;

    int i, j, k;
    double ucf, ecf;     // Unit conversion factor
    Pdemand demand; // Pointer to demand record
    Snode *node;
    Stank *tank;
    Slink *link;
    Scontrol *control;

    // Convert nodal elevations & initial WQ
    // (WQ source units are converted in QUALITY.C
    for (i = 1; i <= net->Nnodes; i++)
    {
        node = &net->Node[i];
        node->El /= pr->Ucf[ELEV];
        node->C0 /= pr->Ucf[QUALITY];
    }

    // Convert demands
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        for (demand = node->D; demand != NULL; demand = demand->next)
        {
            demand->Base /= pr->Ucf[DEMAND];
        }
    }
    
    // Convert PDA pressure limits
    hyd->Pmin /= pr->Ucf[PRESSURE];
    hyd->Preq /= pr->Ucf[PRESSURE];

    // Convert emitter discharge coeffs. to head loss coeff.
    ecf = (parser->Unitsflag == US) ? (PSIperFT * hyd->SpGrav) : (MperFT);

    ucf = pow(pr->Ucf[FLOW], hyd->Qexp) / ecf;
    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        if (node->Ke > 0.0) node->Ke = ucf / pow(node->Ke, hyd->Qexp);
    }

    // Initialize tank variables (convert tank levels to elevations)
    for (j = 1; j <= net->Ntanks; j++)
    {
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
    }

    // Convert hydraulic convergence criteria
    hyd->FlowChangeLimit /= pr->Ucf[FLOW];
    hyd->HeadErrorLimit  /= pr->Ucf[HEAD];

    // Convert water quality concentration options
    qual->Climit /= pr->Ucf[QUALITY];
    qual->Ctol /= pr->Ucf[QUALITY];

    // Convert global reaction coeffs.
    qual->Kbulk /= SECperDAY;
    qual->Kwall /= SECperDAY;

    // Convert units of link parameters
    for (k = 1; k <= net->Nlinks; k++)
    {
        link = &net->Link[k];
        if (link->Type <= PIPE)
        {
            // Convert D-W roughness from millifeet (or mm) to ft
            if (hyd->Formflag == DW) link->Kc /= (1000.0 * pr->Ucf[ELEV]);
            link->Diam /= pr->Ucf[DIAM];
            link->Len /= pr->Ucf[LENGTH];

            // Convert minor loss coeff. from V^2/2g basis to Q^2 basis
            link->Km = 0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam);

            // Convert units on reaction coeffs.
            link->Kb /= SECperDAY;
            link->Kw /= SECperDAY;

            // Convert leakage parameters
            link->LeakArea /= pr->Ucf[LENGTH];
            link->LeakExpan /= pr->Ucf[LENGTH];
        }

        else if (link->Type == PUMP)
        {
            link->Km /= pr->Ucf[POWER];
        }
        
        else
        {
            // For flow control valves, convert flow setting
            // while for other valves convert pressure setting
            link->Diam /= pr->Ucf[DIAM];
            link->Km = 0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam);
            if (link->Kc != MISSING) switch (link->Type)
            {
                case FCV:
                  link->Kc /= pr->Ucf[FLOW];
                  break;
                case PRV:
                case PSV:
                case PBV:
                  link->Kc /= pr->Ucf[PRESSURE];
                  break;
                default:
                  break;
            }
        }
        link->InitSetting = link->Kc;
    }

    // Convert units on control settings
    for (i = 1; i <= net->Ncontrols; i++)
    {
        control = &net->Control[i];
        if ((k = control->Link) == 0) continue;
        link = &net->Link[k];
        if ((j = control->Node) > 0)
        {
            node = &net->Node[j];
            // control is based on tank level
            if (j > net->Njuncs)
            {
                control->Grade = node->El + control->Grade / pr->Ucf[ELEV];
            }
            // control is based on nodal pressure
            else control->Grade = node->El + control->Grade / pr->Ucf[PRESSURE];
        }

        // Convert units on valve settings
        if (control->Setting != MISSING)
        {
            switch (link->Type)
            {
              case PRV:
              case PSV:
              case PBV:
                control->Setting /= pr->Ucf[PRESSURE];
                break;
              case FCV:
                control->Setting /= pr->Ucf[FLOW];
              default:
                break;
            }
        }
    }
}
