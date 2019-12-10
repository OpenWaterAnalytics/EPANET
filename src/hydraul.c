/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       hydraul.c
 Description:  implements EPANET's hydraulic engine
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/05/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "funcs.h"
#include "text.h"

const double QZERO = 1.e-6;  // Equivalent to zero flow in cfs

// Imported functions
extern int  createsparse(Project *);
extern void freesparse(Project *);
extern int  hydsolve(Project *, int *, double *);

// Local functions
int     allocmatrix(Project *);
void    freematrix(Project *);
void    initlinkflow(Project *, int, char, double);
void    demands(Project *);
int     controls(Project *);
long    timestep(Project *);
void    controltimestep(Project *, long *);
void    ruletimestep(Project *, long *);
void    addenergy(Project *, long);
void    tanklevels(Project *, long);
void    resetpumpflow(Project *, int);

int  openhyd(Project *pr)
/*
 *--------------------------------------------------------------
 *  Input:   none
 *  Output:  returns error code
 *  Purpose: opens hydraulics solver system
 *--------------------------------------------------------------
*/
{
    int  i;
    int  errcode = 0;
    Slink *link;

    // Check for too few nodes & no fixed grade nodes
    if (pr->network.Nnodes < 2) errcode = 223;
    else if (pr->network.Ntanks == 0) errcode = 224;

    // Allocate memory for sparse matrix structures (see SMATRIX.C)
    ERRCODE(createsparse(pr));

    // Allocate memory for hydraulic variables
    ERRCODE(allocmatrix(pr));

    // Check for unconnected nodes
    if (!errcode) for (i = 1; i <= pr->network.Njuncs; i++)
    {
        if (pr->network.Adjlist[i] == NULL)
        {
            errcode = 233;
            break;
        }
    }

    // Initialize link flows
    if (!errcode) for (i = 1; i <= pr->network.Nlinks; i++)
    {
        link = &pr->network.Link[i];
        initlinkflow(pr, i, link->Status, link->Kc);
    }
    return errcode;
}

void inithyd(Project *pr, int initflag)
/*
**--------------------------------------------------------------
**  Input:   initflag > 0 if link flows should be re-initialized
**                    = 0 if not
**  Output:  none
**  Purpose: initializes hydraulics solver system
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Outfile *out = &pr->outfile;
    Times   *time = &pr->times;

    int i;
    Stank *tank;
    Slink *link;
    Spump *pump;

    // Initialize tanks
    for (i = 1; i <= net->Ntanks; i++)
    {
        tank = &net->Tank[i];
        tank->V = tank->V0;
        hyd->NodeHead[tank->Node] = tank->H0;
        hyd->NodeDemand[tank->Node] = 0.0;
        hyd->OldStatus[net->Nlinks+i] = TEMPCLOSED;
    }

    // Initialize emitter flows
    memset(hyd->EmitterFlow,0,(net->Nnodes+1)*sizeof(double));
    for (i = 1; i <= net->Nnodes; i++)
    {
        net->Node[i].ResultIndex = i;
        if (net->Node[i].Ke > 0.0) hyd->EmitterFlow[i] = 1.0;
    }

    // Initialize links
    for (i = 1; i <= net->Nlinks; i++)
    {
        link = &net->Link[i];
        link->ResultIndex = i;

        // Initialize status and setting
        hyd->LinkStatus[i] = link->Status;
        hyd->LinkSetting[i] = link->Kc;

        // Compute flow resistance
        resistcoeff(pr, i);

        // Start active control valves in ACTIVE position
        if (
            (link->Type == PRV || link->Type == PSV
            || link->Type == FCV) && (link->Kc != MISSING)
        ) hyd->LinkStatus[i] = ACTIVE;

        // Initialize flows if necessary
        if (hyd->LinkStatus[i] <= CLOSED)
        {
            hyd->LinkFlow[i] = QZERO;
        }
        else if (ABS(hyd->LinkFlow[i]) <= QZERO || initflag > 0)
        {
            initlinkflow(pr, i, hyd->LinkStatus[i], hyd->LinkSetting[i]);
        }

        // Save initial status
        hyd->OldStatus[i] = hyd->LinkStatus[i];
    }

    // Initialize pump energy usage
    for (i = 1; i <= net->Npumps; i++)
    {
        pump = &net->Pump[i];
        pump->Energy.Efficiency = 0.0;
        pump->Energy.TimeOnLine = 0.0;
        pump->Energy.KwHrs = 0.0;
        pump->Energy.KwHrsPerFlow = 0.0;
        pump->Energy.MaxKwatts = 0.0;
        pump->Energy.TotalCost = 0.0;
    }

    // Re-position hydraulics file
    if (pr->outfile.Saveflag)
    {
        fseek(out->HydFile,out->HydOffset,SEEK_SET);
    }

    // Initialize current time
    hyd->Haltflag = 0;
    time->Htime = 0;
    time->Hydstep = 0;
    time->Rtime = time->Rstep;
}


int   runhyd(Project *pr, long *t)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  t = pointer to current time (in seconds)
**  Returns: error code
**  Purpose: solves network hydraulics in a single time period
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;
    Report  *rpt = &pr->report;

    int   iter;          // Iteration count
    int   errcode;       // Error code
    double relerr;       // Solution accuracy
    
    // Find new demands & control actions
    *t = time->Htime;
    demands(pr);
    controls(pr);

    // Solve network hydraulic equations
    errcode = hydsolve(pr,&iter,&relerr);
    if (!errcode)
    {
        // Report new status & save results
        if (rpt->Statflag) writehydstat(pr,iter,relerr);

        // If system unbalanced and no extra trials
        // allowed, then activate the Haltflag
        if (relerr > hyd->Hacc && hyd->ExtraIter == -1)
        {
            hyd->Haltflag = 1;
        }

        // Report any warning conditions
        if (!errcode) errcode = writehydwarn(pr,iter,relerr);
   }
   return errcode;
}

int  nexthyd(Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  tstep = pointer to time step (in seconds)
**  Returns: error code
**  Purpose: finds length of next time step & updates tank
**           levels and rule-based contol actions
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;

    long  hydstep;         // Actual time step
    int   errcode = 0;     // Error code

    // Save current results to hydraulics file and
    // force end of simulation if Haltflag is active
    if (pr->outfile.Saveflag) errcode = savehyd(pr, &time->Htime);
    if (hyd->Haltflag) time->Htime = time->Dur;

    // Compute next time step & update tank levels
    *tstep = 0;
    hydstep = 0;
    if (time->Htime < time->Dur) hydstep = timestep(pr);
    if (pr->outfile.Saveflag) errcode = savehydstep(pr,&hydstep);

    // Compute pumping energy
    if (time->Dur == 0) addenergy(pr,0);
    else if (time->Htime < time->Dur) addenergy(pr,hydstep);

    // More time remains - update current time
    if (time->Htime < time->Dur)
    {
        time->Htime += hydstep;
        if (!pr->quality.OpenQflag)
        {
            if (time->Htime >= time->Rtime) time->Rtime += time->Rstep;
        }
    }

    // No more time remains - force completion of analysis
    else
    {
        time->Htime++;
        if (pr->quality.OpenQflag) time->Qtime++;
    }
    *tstep = hydstep;
    return errcode;
}


void  closehyd(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: closes hydraulics solver system
**--------------------------------------------------------------
*/
{
    freesparse(pr);
    freematrix(pr);
}


int  allocmatrix(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: allocates memory used for solution matrix coeffs.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int errcode = 0;

    hyd->P   = (double *) calloc(net->Nlinks+1,sizeof(double));
    hyd->Y   = (double *) calloc(net->Nlinks+1,sizeof(double));
    hyd->DemandFlow = (double *) calloc(net->Nnodes + 1, sizeof(double));
    hyd->EmitterFlow = (double *) calloc(net->Nnodes+1, sizeof(double));
    hyd->Xflow = (double *) calloc(MAX((net->Nnodes+1), (net->Nlinks+1)),
                                   sizeof(double));
    hyd->OldStatus = (StatusType *) calloc(net->Nlinks+net->Ntanks+1,
                                           sizeof(StatusType));
    ERRCODE(MEMCHECK(hyd->P));
    ERRCODE(MEMCHECK(hyd->Y));
    ERRCODE(MEMCHECK(hyd->DemandFlow));
    ERRCODE(MEMCHECK(hyd->EmitterFlow));
    ERRCODE(MEMCHECK(hyd->Xflow));
    ERRCODE(MEMCHECK(hyd->OldStatus));
    return errcode;
}


void  freematrix(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: frees memory used for solution matrix coeffs.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    free(hyd->P);
    free(hyd->Y);
    free(hyd->DemandFlow);
    free(hyd->EmitterFlow);
    free(hyd->Xflow);
    free(hyd->OldStatus);
}


void  initlinkflow(Project *pr, int i, char s, double k)
/*
**--------------------------------------------------------------------
**  Input:   i = link index
**           s = link status
**           k = link setting (i.e., pump speed)
**  Output:  none
**  Purpose: sets initial flow in link to QZERO if link is closed,
**           to design flow for a pump, or to flow at velocity of
**           1 fps for other links.
**--------------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Network *n = &pr->network;

    Slink *link = &n->Link[i];

    if (s == CLOSED)
    {
        hyd->LinkFlow[i] = QZERO;
    }
    else if (link->Type == PUMP)
    {
        hyd->LinkFlow[i] = k * n->Pump[findpump(n,i)].Q0;
    }
    else
    {
        hyd->LinkFlow[i] = PI * SQR(link->Diam)/4.0;
    }
}


void  setlinkstatus(Project *pr, int index, char value, StatusType *s, double *k)
/*----------------------------------------------------------------
**  Input:   index  = link index
**           value  = 0 (CLOSED) or 1 (OPEN)
**           s      = pointer to link status
**           k      = pointer to link setting
**  Output:  none
**  Purpose: sets link status to OPEN or CLOSED
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    Slink *link = &net->Link[index];
    LinkType t = link->Type;

    // Status set to open
    if (value == 1)
    {
        // Adjust link setting for pumps & valves
        if (t == PUMP)
        {
            *k = 1.0;
            // Check if a re-opened pump needs its flow reset            
            if (*s == CLOSED) resetpumpflow(pr, index);
        }
        if (t > PUMP &&  t != GPV) *k = MISSING;
        *s = OPEN;
     }

     // Status set to closed
     else if (value == 0)
     {
         // Adjust link setting for pumps & valves
         if (t == PUMP) *k = 0.0;
         if (t > PUMP && t != GPV) *k = MISSING;
         *s = CLOSED;
     }
}


void  setlinksetting(Project *pr, int index, double value, StatusType *s,
                     double *k)
/*----------------------------------------------------------------
**  Input:   index  = link index
**           value  = pump speed or valve setting
**           s      = pointer to link status
**           k      = pointer to link setting
**  Output:  none
**  Purpose: sets pump speed or valve setting, adjusting link
**           status and flow when necessary
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    Slink *link = &net->Link[index];
    LinkType t = link->Type;

    // For a pump, status is OPEN if speed > 0, CLOSED otherwise
    if (t == PUMP)
    {
        *k = value;
        if (value > 0 && *s <= CLOSED)
        {
            // Check if a re-opened pump needs its flow reset
            resetpumpflow(pr, index);
            *s = OPEN;
        }
        if (value == 0 && *s > CLOSED) *s = CLOSED;
    }

    // For FCV, activate it
    else if (t == FCV)
    {
        *k = value;
        *s = ACTIVE;
    }

    // Open closed control valve with fixed status (setting = MISSING)
    else
    {
        if (*k == MISSING && *s <= CLOSED) *s = OPEN;
        *k = value;
    }
}


void  demands(Project *pr)
/*
**--------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: computes demands at nodes during current time period
**--------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;

    int  i ,j, n;
    long k, p;
    double djunc, sum;
    Pdemand demand;

    // Determine total elapsed number of pattern periods
    p = (time->Htime + time->Pstart) / time->Pstep;

    // Update demand at each node according to its assigned pattern
    hyd->Dsystem = 0.0;          // System-wide demand
    for (i = 1; i <= net->Njuncs; i++)
    {
        sum = 0.0;
        for (demand = net->Node[i].D; demand != NULL; demand = demand->next)
        {
            // pattern period (k) = (elapsed periods) modulus (periods per pattern)
            j = demand->Pat;
            k = p % (long)net->Pattern[j].Length;
            djunc = (demand->Base) * net->Pattern[j].F[k] * hyd->Dmult;
            if (djunc > 0.0) hyd->Dsystem += djunc;
            sum += djunc;
        }
        hyd->NodeDemand[i] = sum;

        // Initialize pressure dependent demand
        hyd->DemandFlow[i] = sum;
    }

    // Update head at fixed grade nodes with time patterns
    for (n = 1; n <= net->Ntanks; n++)
    {
        Stank *tank = &net->Tank[n];
        if (tank->A == 0.0)
        {
            j = tank->Pat;
            if (j > 0)
            {
                k = p % (long) net->Pattern[j].Length;
                i = tank->Node;
                hyd->NodeHead[i] = net->Node[i].El * net->Pattern[j].F[k];
            }
        }
    }

    // Update status of pumps with utilization patterns
    for (n = 1; n <= net->Npumps; n++)
    {
        Spump *pump = &net->Pump[n];
        j = pump->Upat;
        if (j > 0)
        {
            i = pump->Link;
            k = p % (long) net->Pattern[j].Length;
            setlinksetting(pr, i, net->Pattern[j].F[k], &hyd->LinkStatus[i],
                           &hyd->LinkSetting[i]);
        }
    }
}


int  controls(Project *pr)
/*
**---------------------------------------------------------------------
**  Input:   none
**  Output:  number of links whose setting changes
**  Purpose: implements simple controls based on time or tank levels
**---------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;

    int i, k, n, reset, setsum;
    double h, vplus;
    double v1, v2;
    double k1, k2;
    char  s1, s2;
    Slink *link;
    Scontrol *control;

    // Examine each control statement
    setsum = 0;
    for (i=1; i <= net->Ncontrols; i++)
    {
        // Make sure that link is defined
        control = &net->Control[i];
        reset = 0;
        if ( (k = control->Link) <= 0) continue;
        link = &net->Link[k];

        // Link is controlled by tank level
        if ((n = control->Node) > 0 && n > net->Njuncs)
        {
            h = hyd->NodeHead[n];
            vplus = ABS(hyd->NodeDemand[n]);
            v1 = tankvolume(pr,n - net->Njuncs,h);
            v2 = tankvolume(pr,n - net->Njuncs, control->Grade);
            if (control->Type == LOWLEVEL && v1 <= v2 + vplus) reset = 1;
            if (control->Type == HILEVEL && v1 >= v2 - vplus)  reset = 1;
        }

        // Link is time-controlled
        if (control->Type == TIMER)
        {
            if (control->Time == time->Htime) reset = 1;
        }

        //* Link is time-of-day controlled
        if (control->Type == TIMEOFDAY)
        {
            if ((time->Htime + time->Tstart) % SECperDAY == control->Time)
            {
            reset = 1;
            }
        }

        // Update link status & pump speed or valve setting
        if (reset == 1)
        {
            if (hyd->LinkStatus[k] <= CLOSED) s1 = CLOSED;
            else s1 = OPEN;
            s2 = control->Status;
            k1 = hyd->LinkSetting[k];
            k2 = k1;
            if (link->Type > PIPE) k2 = control->Setting;
            
            // Check if a re-opened pump needs its flow reset
            if (link->Type == PUMP && s1 == CLOSED && s2 == OPEN)
                resetpumpflow(pr, k);
                
            if (s1 != s2 || k1 != k2)
            {
                hyd->LinkStatus[k] = s2;
                hyd->LinkSetting[k] = k2;
                if (pr->report.Statflag) writecontrolaction(pr,k,i);
                setsum++;
            }
        }
    }
    return setsum;
}


long  timestep(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  returns time step until next change in hydraulics
**  Purpose: computes time step to advance hydraulic simulation
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Times   *time = &pr->times;

    long n, t, tstep;

    // Normal time step is hydraulic time step
    tstep = time->Hstep;

    // Revise time step based on time until next demand period
    // (n = next pattern period, t = time till next period)
    n = ((time->Htime + time->Pstart) / time->Pstep) + 1;
    t = n * time->Pstep - time->Htime;
    if (t > 0 && t < tstep) tstep = t;

    // Revise time step based on time until next reporting period
    t = time->Rtime - time->Htime;
    if (t > 0 && t < tstep) tstep = t;

    // Revise time step based on smallest time to fill or drain a tank
    tanktimestep(pr, &tstep);

    // Revise time step based on smallest time to activate a control
    controltimestep(pr, &tstep);

    // Evaluate rule-based controls (which will also update tank levels)
    if (net->Nrules > 0) ruletimestep(pr, &tstep);
    else tanklevels(pr, tstep);
    return tstep;
}


int  tanktimestep(Project *pr, long *tstep)
/*
**-----------------------------------------------------------------
**  Input:   *tstep = current time step
**  Output:  *tstep = modified current time step
**  Purpose: revises time step based on shortest time to fill or
**           drain a tank
**-----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int     i, n, tankIdx = 0;
    double  h, q, v;
    long    t;
    Stank   *tank;

    // Examine each tank
    for (i = 1; i <= net->Ntanks; i++)
    {
        // Skip reservoirs
        tank = &net->Tank[i];
        if (tank->A == 0.0) continue;

        // Get current tank grade (h) & inflow (q)
        n = tank->Node;
        h = hyd->NodeHead[n];
        q = hyd->NodeDemand[n];
        if (ABS(q) <= QZERO) continue;

        // Find volume to fill/drain tank
        if      (q > 0.0 && h < tank->Hmax) v = tank->Vmax - tank->V;
        else if (q < 0.0 && h > tank->Hmin) v = tank->Vmin - tank->V;
        else continue;

        // Find time to fill/drain tank
        t = (long)ROUND(v / q);
        if (t > 0 && t < *tstep)
        {
            *tstep = t;
            tankIdx = n;
        }
    }
    return tankIdx;
}


void  controltimestep(Project *pr, long *tstep)
/*
**------------------------------------------------------------------
**  Input:   *tstep = current time step
**  Output:  *tstep = modified current time step
**  Purpose: revises time step based on shortest time to activate
**           a simple control
**------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int    i, j, k, n;
    double h, q, v;
    long   t, t1, t2;
    Slink  *link;
    Scontrol *control;

    // Examine each control
    for (i = 1; i <= net->Ncontrols; i++)
    {
        t = 0;
        control = &net->Control[i];

        // Control depends on a tank level
        if ( (n = control->Node) > 0)
        {
            // Skip node if not a tank or reservoir
            if ((j = n - net->Njuncs) <= 0) continue;

            // Find current head and flow into tank
            h = hyd->NodeHead[n];
            q = hyd->NodeDemand[n];
            if (ABS(q) <= QZERO) continue;

            // Find time to reach upper or lower control level
           if ( (h < control->Grade && control->Type == HILEVEL && q > 0.0)
           ||   (h > control->Grade && control->Type == LOWLEVEL && q < 0.0) )
           {
               v = tankvolume(pr, j, control->Grade) - net->Tank[j].V;
               t = (long)ROUND(v/q);
           }
        }

        // Control is based on elapsed time
        if (control->Type == TIMER)
        {
            if (control->Time > pr->times.Htime)
            {
                t = control->Time - pr->times.Htime;
            }
        }

        // Control is based on time of day
        if (control->Type == TIMEOFDAY)
        {
            t1 = (pr->times.Htime + pr->times.Tstart) % SECperDAY;
            t2 = control->Time;
            if (t2 >= t1) t = t2 - t1;
            else          t = SECperDAY - t1 + t2;
        }

        // Revise the current estimated next time step
        if (t > 0 && t < *tstep)
        {
            // Check if rule actually changes link status or setting
            k = control->Link;
            link = &net->Link[k];
            if ( (link->Type > PIPE && hyd->LinkSetting[k] != control->Setting)
            ||   (hyd->LinkStatus[k] != control->Status) ) *tstep = t;
        }
    }
}


void  ruletimestep(Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**  Input:   *tstep = current time step (sec)
**  Output:  *tstep = modified time step
**  Purpose: updates next time step by checking if any rules
**           will fire before then; also updates tank levels.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Times   *time = &pr->times;

    long tnow,      // Start of time interval for rule evaluation
         tmax,      // End of time interval for rule evaluation
         dt,        // Normal time increment for rule evaluation
         dt1;       // Actual time increment for rule evaluation

    // Find interval of time for rule evaluation
    tnow = time->Htime;
    tmax = tnow + *tstep;

    // If no rules, then time increment equals current time step
    if (net->Nrules == 0)
    {
        dt = *tstep;
        dt1 = dt;
    }

    // Otherwise, time increment equals rule evaluation time step and
    // first actual increment equals time until next even multiple of
    // Rulestep occurs.
    else
    {
        dt = time->Rulestep;
        dt1 = time->Rulestep - (tnow % time->Rulestep);
    }

    // Make sure time increment is no larger than current time step
    dt = MIN(dt, *tstep);
    dt1 = MIN(dt1, *tstep);
    if (dt1 == 0) dt1 = dt;

    // Step through time, updating tank levels, until either
    // a rule fires or we reach the end of evaluation period.
    //
    // Note: we are updating the global simulation time (Htime)
    //       here because it is used by functions in RULES.C
    //       to evaluate rules when checkrules() is called.
    //       It is restored to its original value after the
    //       rule evaluation process is completed (see below).
    //       Also note that dt1 will equal dt after the first
    //       time increment is taken.
    //
    do
    {
        time->Htime += dt1;                // Update simulation clock
        tanklevels(pr, dt1);                // Find new tank levels
        if (checkrules(pr, dt1)) break;     // Stop if any rule fires
        dt = MIN(dt, tmax - time->Htime);  // Update time increment
        dt1 = dt;                           // Update actual increment
    } while (dt > 0);                       // Stop if no time left

    // Compute an updated simulation time step (*tstep)
    // and return simulation time to its original value
    *tstep = time->Htime - tnow;
    time->Htime = tnow;
}


void  addenergy(Project *pr, long hstep)
/*
**-------------------------------------------------------------
**  Input:   hstep = time step (sec)
**  Output:  none
**  Purpose: accumulates pump energy usage
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;

    int    i, j, k;
    long   m, n;
    double c0, c,            // Energy cost (cost/kwh)
           f0,               // Energy cost factor
           dt,               // Time interval (hr)
           e,                // Pump efficiency (fraction)
           q,                // Pump flow (cfs)
           p,                // Pump energy (kw)
           psum = 0.0;       // Total energy (kw)
    Spump  *pump;

    // Determine current time interval in hours
    if (time->Dur == 0) dt = 1.0;
    else if (time->Htime < time->Dur)
    {
        dt = (double) hstep / 3600.0;
    }
    else dt = 0.0;
    if (dt == 0.0) return;
    n = (time->Htime + time->Pstart) / time->Pstep;

    // Compute default energy cost at current time
    c0 = hyd->Ecost;
    f0 = 1.0;
    if (hyd->Epat > 0)
    {
        m = n % (long)net->Pattern[hyd->Epat].Length;
        f0 = net->Pattern[hyd->Epat].F[m];
    }

    // Examine each pump
    for (j = 1; j <= net->Npumps; j++)
    {
        // Skip closed pumps
        pump = &net->Pump[j];
        k = pump->Link;
        if (hyd->LinkStatus[k] <= CLOSED) continue;
        q = MAX(QZERO, ABS(hyd->LinkFlow[k]));

        // Find pump-specific energy cost
        if (pump->Ecost > 0.0) c = pump->Ecost;
        else c = c0;
        if ( (i = pump->Epat) > 0)
        {
            m = n % (long)net->Pattern[i].Length;
            c *= net->Pattern[i].F[m];
        }
        else c *= f0;

        // Find pump energy & efficiency
        getenergy(pr, k, &p, &e);
        psum += p;

        // Update pump's cumulative statistics
        pump->Energy.TimeOnLine += dt;
        pump->Energy.Efficiency += e * dt;
        pump->Energy.KwHrsPerFlow += p / q * dt;
        pump->Energy.KwHrs += p * dt;
        pump->Energy.MaxKwatts = MAX(pump->Energy.MaxKwatts, p);
        pump->Energy.TotalCost += c * p * dt;
    }

    // Update maximum kw value
    hyd->Emax = MAX(hyd->Emax, psum);
}


void  getenergy(Project *pr, int k, double *kw, double *eff)
/*
**----------------------------------------------------------------
**  Input:   k    = link index
**  Output:  *kw  = kwatt energy used
**           *eff = efficiency (pumps only)
**  Purpose: computes flow energy associated with link k
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int    i,       // efficiency curve index
           j;       // pump index
    double dh,      // head across pump (ft)
           q,       // flow through pump (cfs)
           e;       // pump efficiency
    double q4eff;   // flow at nominal pump speed of 1.0
    double speed;   // current speed setting
    Scurve *curve;
    Slink  *link = &net->Link[k];

    // No energy if link is closed
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        *kw = 0.0;
        *eff = 0.0;
        return;
    }

    // Determine flow and head difference
    q = ABS(hyd->LinkFlow[k]);
    dh = ABS(hyd->NodeHead[link->N1] - hyd->NodeHead[link->N2]);

    // For pumps, find effic. at current flow
    if (link->Type == PUMP)
    {
        j = findpump(net, k);
        e = hyd->Epump;
        speed = hyd->LinkSetting[k];
        if ((i = net->Pump[j].Ecurve) > 0)
        {
            q4eff = q / speed * pr->Ucf[FLOW];
            curve = &net->Curve[i];
            e = interp(curve->Npts,curve->X, curve->Y, q4eff);

            // Sarbu and Borza pump speed adjustment
            e = 100.0 - ((100.0-e) * pow(1.0/speed, 0.1));
        }
        e = MIN(e, 100.0);
        e = MAX(e, 1.0);
        e /= 100.0;
    }
    else e = 1.0;

    // Compute energy
    *kw = dh * q * hyd->SpGrav / 8.814 / e * KWperHP;
    *eff = e;
}


void  tanklevels(Project *pr, long tstep)
/*
**----------------------------------------------------------------
**  Input:   tstep = current time step
**  Output:  none
**  Purpose: computes new water levels in tanks after current
**           time step
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int    i, n;
    double dv;

    for (i = 1; i <= net->Ntanks; i++)
    {
        Stank *tank = &net->Tank[i];
        if (tank->A == 0.0) continue;    // Skip reservoirs

        // Update the tank's volume & water elevation
        n = tank->Node;
        dv = hyd->NodeDemand[n] * tstep;
        tank->V += dv;

        // Check if tank full/empty within next second
        if (tank->V + hyd->NodeDemand[n] >= tank->Vmax)
        {
            tank->V = tank->Vmax;
        }
        else if (tank->V - hyd->NodeDemand[n] <= tank->Vmin)
        {
            tank->V = tank->Vmin;
        }
        hyd->NodeHead[n] = tankgrade(pr, i, tank->V);
    }
}


double  tankvolume(Project *pr, int i, double h)
/*
**--------------------------------------------------------------------
**  Input:   i = tank index
**           h = water elevation in tank
**  Output:  returns water volume in tank
**  Purpose: finds water volume in tank i corresponding to elev. h.
**--------------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int j;
    double y, v;
    Stank *tank = &net->Tank[i];
    Scurve *curve;

    // Use level*area if no volume curve
    j = tank->Vcurve;
    if (j == 0) return(tank->Vmin + (h - tank->Hmin) * tank->A);

    // If curve exists, interpolate on h to find volume v
    // remembering that volume curve is in original units.
    else
    {
        curve = &net->Curve[j];
        y = (h - net->Node[tank->Node].El) * pr->Ucf[HEAD];
        v = interp(curve->Npts, curve->X, curve->Y, y) / pr->Ucf[VOLUME];
        return v;
    }
}


double  tankgrade(Project *pr, int i, double v)
/*
**-------------------------------------------------------------------
**  Input:   i = tank index
**           v = volume in tank
**  Output:  returns water level in tank
**  Purpose: finds water level in tank i corresponding to volume v.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;

    int j;
    double y, h;
    Stank *tank = &net->Tank[i];

    // Use area if no volume curve
    j = tank->Vcurve;
    if (j == 0) return(tank->Hmin + (v - tank->Vmin) / tank->A);

    // If curve exists, interpolate on volume (originally the Y-variable
    // but used here as the X-variable) to find new level above bottom.
    // Remember that volume curve is stored in original units.
    else
    {
        Scurve *curve = &net->Curve[j];
        y = interp(curve->Npts, curve->Y, curve->X, v * pr->Ucf[VOLUME]);
        h = net->Node[tank->Node].El + y / pr->Ucf[HEAD];
        return h;
    }
}

void resetpumpflow(Project *pr, int i)
/*
**-------------------------------------------------------------------
**  Input:   i = link index
**  Output:  none
**  Purpose: resets flow in a constant HP pump to its initial value.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Spump *pump = &net->Pump[findpump(net, i)];
    if (pump->Ptype == CONST_HP)
        pr->hydraul.LinkFlow[i] = pump->Q0; 
}

