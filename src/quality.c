/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       quality.c
Description:  implements EPANET's water quality engine
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 05/15/2019
******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mempool.h"
#include "types.h"
#include "funcs.h"

// Stagnant flow tolerance
const double Q_STAGNANT = 0.005 / GPMperCFS;     // 0.005 gpm = 1.114e-5 cfs

// Exported functions
double  findsourcequal(Project *, int, double, long);

// Imported functions
extern char    setreactflag(Project *);
extern double  getucf(double);
extern void    ratecoeffs(Project *);
extern void    initsegs(Project *);
extern void    reversesegs(Project *, int);
extern int     sortnodes(Project *);
extern void    transport(Project *, long);

// Local functions
static double  sourcequal(Project *, Psource);
static void    evalmassbalance(Project *);
static double  findstoredmass(Project *);
static int     flowdirchanged(Project *);


int openqual(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: opens water quality solver
**--------------------------------------------------------------
*/
{
    Network  *net = &pr->network;
    Quality *qual = &pr->quality;

    int errcode = 0;
    int n;

    // Return if no quality analysis requested
    if (qual->Qualflag == NONE) return errcode;

    // Build nodal adjacency lists if they don't already exist
    if (net->Adjlist == NULL)
    {
        errcode = buildadjlists(net);
        if (errcode ) return errcode;
    }

    // Create a memory pool for water quality segments
    qual->OutOfMemory = FALSE;
    qual->SegPool = mempool_create();
    if (qual->SegPool == NULL) errcode = 101;

    // Allocate arrays for link flow direction & reaction rates
    n = net->Nlinks + 1;
    qual->FlowDir = (FlowDirection *)calloc(n, sizeof(FlowDirection));
    qual->PipeRateCoeff = (double *)calloc(n, sizeof(double));

    // Allocate arrays used for volume segments in links & tanks
    n = net->Nlinks + net->Ntanks + 1;
    qual->FirstSeg = (Pseg *)calloc(n, sizeof(Pseg));
    qual->LastSeg = (Pseg *)calloc(n, sizeof(Pseg));

    // Allocate memory for topologically sorted nodes
    qual->SortedNodes = (int *)calloc(n, sizeof(int));

    ERRCODE(MEMCHECK(qual->FlowDir));
    ERRCODE(MEMCHECK(qual->PipeRateCoeff));
    ERRCODE(MEMCHECK(qual->FirstSeg));
    ERRCODE(MEMCHECK(qual->LastSeg));
    ERRCODE(MEMCHECK(qual->SortedNodes));
    return errcode;
}


int initqual(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: re-initializes water quality solver
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    int i;
    int errcode = 0;

    // Re-position hydraulics file
    if (!hyd->OpenHflag)
    {
        fseek(pr->outfile.HydFile, pr->outfile.HydOffset, SEEK_SET);
    }

    // Set elapsed times to zero
    time->Qtime = 0;
    time->Htime = 0;
    time->Rtime = time->Rstart;
    pr->report.Nperiods = 0;

    // Initialize node quality
    for (i = 1; i <= net->Nnodes; i++)
    {
        if (qual->Qualflag == TRACE) qual->NodeQual[i] = 0.0;
        else                         qual->NodeQual[i] = net->Node[i].C0;
        if (net->Node[i].S != NULL) net->Node[i].S->Smass = 0.0;
    }
    if (qual->Qualflag == NONE) return errcode;

    // Initialize tank quality
    for (i = 1; i <= net->Ntanks; i++)
    {
        net->Tank[i].C = qual->NodeQual[net->Tank[i].Node];
    }

    // Initialize quality at trace node (if applicable)
    if (qual->Qualflag == TRACE) qual->NodeQual[qual->TraceNode] = 100.0;

    // Compute Schmidt number
    if (qual->Diffus > 0.0) qual->Sc = hyd->Viscos / qual->Diffus;
    else                    qual->Sc = 0.0;

    // Compute unit conversion factor for bulk react. coeff.
    qual->Bucf = getucf(qual->BulkOrder);
    qual->Tucf = getucf(qual->TankOrder);

    // Check if modeling a reactive substance
    qual->Reactflag = setreactflag(pr);

    // Reset memory pool used for pipe & tank segments
    qual->FreeSeg = NULL;
    mempool_reset(qual->SegPool);

    // Create initial set of pipe & tank segments
    initsegs(pr);

    // Initialize link flow direction indicator
    for (i = 1; i <= net->Nlinks; i++) qual->FlowDir[i] = ZERO_FLOW;

    // Initialize avg. reaction rates
    qual->Wbulk = 0.0;
    qual->Wwall = 0.0;
    qual->Wtank = 0.0;
    qual->Wsource = 0.0;

    // Initialize mass balance components
    qual->MassBalance.initial = findstoredmass(pr);
    qual->MassBalance.inflow = 0.0;
    qual->MassBalance.outflow = 0.0;
    qual->MassBalance.reacted = 0.0;
    qual->MassBalance.final = 0.0;
    qual->MassBalance.ratio = 0.0;
    return errcode;
}


int runqual(Project *pr, long *t)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  t = current simulation time (sec)
**   Returns: error code
**   Purpose: retrieves hydraulics for next hydraulic time step
**            (at time t) and saves current results to file
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    long hydtime = 0;       // Hydraulic solution time
    long hydstep = 0;       // Hydraulic time step
    int errcode = 0;

    // Update reported simulation time
    *t = time->Qtime;

    // Read hydraulic solution from hydraulics file
    if (time->Qtime == time->Htime)
    {
        // Read hydraulic results from file
        if (!hyd->OpenHflag)
        {
            if (!readhyd(pr, &hydtime)) return 307;
            if (!readhydstep(pr, &hydstep)) return 307;
            time->Htime = hydtime;
        }

        // Save current results to output file
        if (time->Htime >= time->Rtime)
        {
            if (pr->outfile.Saveflag)
            {
                errcode = saveoutput(pr);
                pr->report.Nperiods++;
            }
            time->Rtime += time->Rstep;
        }
        if (errcode) return errcode;

        // If simulating water quality
        if (qual->Qualflag != NONE && time->Qtime < time->Dur)
        {
            // ... compute reaction rate coeffs.
            if (qual->Reactflag && qual->Qualflag != AGE) ratecoeffs(pr);

            // ... topologically sort network nodes if flow directions change
            if (flowdirchanged(pr) == TRUE)
            {
                errcode = sortnodes(pr);
            }
        }
        if (!hyd->OpenHflag) time->Htime = hydtime + hydstep;
    }
    return errcode;
}


int nextqual(Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  tstep = time step (sec) over which quality was updated
**   Returns: error code
**   Purpose: updates water quality in network until next hydraulic
**            event occurs (after tstep secs.)
**--------------------------------------------------------------
*/
{
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    long hydstep;            // Time step until next hydraulic event
    long dt, qtime;
    int errcode = 0;

    // Find time step till next hydraulic event
    *tstep = 0;
    hydstep = 0;
    if (time->Htime <= time->Dur) hydstep = time->Htime - time->Qtime;

    // Perform water quality routing over this time step
    if (qual->Qualflag != NONE && hydstep > 0)
    {
        // Repeat over each quality time step until tstep is reached
        qtime = 0;
        while (!qual->OutOfMemory && qtime < hydstep)
        {
            dt = MIN(time->Qstep, hydstep - qtime);
            qtime += dt;
            transport(pr, dt);
        }
        if (qual->OutOfMemory) errcode = 101;
    }

    // Update mass balance ratio
    evalmassbalance(pr);

    // Update current time
    if (!errcode) *tstep = hydstep;
    time->Qtime += hydstep;

    // If no more time steps remain
    if (!errcode && *tstep == 0)
    {
        // ... report overall mass balance
        if (qual->Qualflag != NONE && pr->report.Statflag)
        {
            writemassbalance(pr);
        }

        // ... write the final portion of the binary output file
        if (pr->outfile.Saveflag) errcode = savefinaloutput(pr);
    }
    return errcode;
}


int stepqual(Project *pr, long *tleft)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  tleft = time left in simulation
**   Returns: error code
**   Purpose: updates quality conditions over a single
**            quality time step
**--------------------------------------------------------------
*/
{
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    long dt, hstep, t, tstep;
    int errcode = 0;

    tstep = time->Qstep;
    do
    {
        // Set local time step to quality time step
        dt = tstep;

        // Find time step until next hydraulic event
        hstep = time->Htime - time->Qtime;

        // If next hydraulic event occurs before end of local time step
        if (hstep < dt)
        {
            // ... adjust local time step to next hydraulic event
            dt = hstep;

            // ... transport quality over local time step
            if (qual->Qualflag != NONE) transport(pr, dt);
            time->Qtime += dt;

            // ... quit if running quality concurrently with hydraulics
            if (pr->hydraul.OpenHflag) break;

            // ... otherwise call runqual() to update hydraulics
            errcode = runqual(pr, &t);
            time->Qtime = t;
        }

        // Otherwise transport quality over current local time step
        else
        {
            if (qual->Qualflag != NONE) transport(pr, dt);
            time->Qtime += dt;
        }

        // Reduce quality time step by local time step
        tstep -= dt;
        if (qual->OutOfMemory) errcode = 101;

    } while (!errcode && tstep > 0);

    // Update mass balance ratio
    evalmassbalance(pr);

    // Update total simulation time left
    *tleft = time->Dur - time->Qtime;

    // If no more time steps remain
    if (!errcode && *tleft == 0)
    {
        // ... report overall mass balance
        if (qual->Qualflag != NONE && pr->report.Statflag)
        {
            writemassbalance(pr);
        }

        // ... write the final portion of the binary output file
        if (pr->outfile.Saveflag) errcode = savefinaloutput(pr);
    }
    return errcode;
}


int closequal(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns error code
**   Purpose: closes water quality solver
**--------------------------------------------------------------
*/
{
    Quality *qual = &pr->quality;
    int errcode = 0;

    if (qual->Qualflag != NONE)
    {
        if (qual->SegPool) mempool_delete(qual->SegPool);
        FREE(qual->FirstSeg);
        FREE(qual->LastSeg);
        FREE(qual->PipeRateCoeff);
        FREE(qual->FlowDir);
        FREE(qual->SortedNodes);
    }
    return errcode;
}


double avgqual(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  returns quality concentration
**   Purpose: computes current average quality in link k
**--------------------------------------------------------------
*/
{
    Network  *net = &pr->network;
    Quality  *qual = &pr->quality;

    double vsum = 0.0, msum = 0.0;
    Pseg seg;

    if (qual->Qualflag == NONE) return 0.0;

    // Sum up the quality and volume in each segment of the link
    if (qual->FirstSeg != NULL)
    {
        seg = qual->FirstSeg[k];
        while (seg != NULL)
        {
            vsum += seg->v;
            msum += (seg->c) * (seg->v);
            seg = seg->prev;
        }
    }

    // Compute average quality if link has volume
    if (vsum > 0.0) return (msum / vsum);

    // Otherwise use the average quality of the link's end nodes
    else
    {
        return ((qual->NodeQual[net->Link[k].N1] +
            qual->NodeQual[net->Link[k].N2]) / 2.);
    }
}


double findsourcequal(Project *pr, int n, double volout, long tstep)
/*
**---------------------------------------------------------------------
**   Input:   n = node index
**            volout = volume of node outflow over time step
**            tstep = current quality time step
**   Output:  returns concentration added by an external quality source.
**   Purpose: computes contribution (if any) of mass addition from an
**            external quality source at a node.
**---------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Times   *time = &pr->times;

    double massadded = 0.0, c;
    Psource source;

    // Sources only apply to CHEMICAL analyses
    if (qual->Qualflag != CHEM) return 0.0;

    // Return 0 if node is not a quality source or has no outflow
    source = net->Node[n].S;
    if (source == NULL)    return 0.0;
    if (source->C0 == 0.0) return 0.0;
    if (volout / tstep <= Q_STAGNANT) return 0.0;

    // Added source concentration depends on source type
    c = sourcequal(pr, source);
    switch (source->Type)
    {
        // Concentration Source:
        case CONCEN:
        if (net->Node[n].Type == JUNCTION)
        {
            // ... source requires a negative demand at the node
            if (hyd->NodeDemand[n] < 0.0)
            {
                c = -c * hyd->NodeDemand[n] * tstep / volout;
            }
            else c = 0.0;
        }
        break;

        // Mass Inflow Booster Source:
        case MASS:
            // ... convert source input from mass/sec to concentration
            c = c * tstep / volout;
            break;

        // Setpoint Booster Source:
        // Source quality is difference between source strength
        // & node quality
        case SETPOINT:
            c = MAX(c - qual->NodeQual[n], 0.0);
            break;

        // Flow-Paced Booster Source:
        // Source quality equals source strength
        case FLOWPACED:
            break;
    }

    // Source mass added over time step = source concen. * outflow volume
    massadded = c * volout;

    // Update source's total mass added
    source->Smass += massadded;

    // Update Wsource
    if (time->Htime >= time->Rstart)
    {
        qual->Wsource += massadded;
    }
    return c;
}


double sourcequal(Project *pr, Psource source)
/*
**--------------------------------------------------------------
**   Input:   source = a water quality source object
**   Output:  returns strength of quality source
**   Purpose: determines source strength in current time period
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Times   *time = &pr->times;

    int i;
    long k;
    double c;

    // Get source concentration (or mass flow) in original units
    c = source->C0;

    // Convert mass flow rate from min. to sec.
    // and convert concen. from liters to cubic feet
    if (source->Type == MASS) c /= 60.0;
    else                      c /= pr->Ucf[QUALITY];

    // Apply time pattern if assigned
    i = source->Pat;
    if (i == 0)  return c;
    k = ((time->Qtime + time->Pstart) / time->Pstep) %
        (long)net->Pattern[i].Length;
    return (c * net->Pattern[i].F[k]);
}


void  evalmassbalance(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes the overall mass balance ratio of a
**            quality constituent.
**--------------------------------------------------------------
*/
{
    Quality *qual = &pr->quality;

    double massin;
    double massout;
    double massreacted;

    if (qual->Qualflag == NONE) qual->MassBalance.ratio = 1.0;
    else
    {
        qual->MassBalance.final = findstoredmass(pr);
        massin = qual->MassBalance.initial + qual->MassBalance.inflow;
        massout = qual->MassBalance.outflow + qual->MassBalance.final;
        massreacted = qual->MassBalance.reacted;
        if (massreacted > 0.0) massout += massreacted;
        else                   massin -= massreacted;
        if (massin == 0.0) qual->MassBalance.ratio = 1.0;
        else               qual->MassBalance.ratio = massout / massin;
    }
}


double  findstoredmass(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns total constituent mass stored in the network
**   Purpose: finds the current mass of a constituent stored in
**            all pipes and tanks.
**--------------------------------------------------------------
*/
{
    Network  *net = &pr->network;
    Quality  *qual = &pr->quality;

    int    i, k;
    double totalmass = 0.0;
    Pseg   seg;

    // Mass residing in each pipe
    for (k = 1; k <= net->Nlinks; k++)
    {
        // Sum up the quality and volume in each segment of the link
        seg = qual->FirstSeg[k];
        while (seg != NULL)
        {
            totalmass += (seg->c) * (seg->v);
            seg = seg->prev;
        }
    }

    // Mass residing in each tank
    for (i = 1; i <= net->Ntanks; i++)
    {
        // ... skip reservoirs
        if (net->Tank[i].A == 0.0) continue;

        // ... add up mass in each volume segment
        else
        {
            k = net->Nlinks + i;
            seg = qual->FirstSeg[k];
            while (seg != NULL)
            {
                totalmass += seg->c * seg->v;
                seg = seg->prev;
            }
        }
    }
    return totalmass;
}

int flowdirchanged(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns TRUE if flow direction changes in any link
**   Purpose: finds new flow directions for each network link.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;

    int k;
    int result = FALSE;
    int newdir;
    int olddir;
    double q;

    // Examine each network link
    for (k = 1; k <= pr->network.Nlinks; k++)
    {
        // Determine sign (+1 or -1) of new flow rate
        olddir = qual->FlowDir[k];
        q = (hyd->LinkStatus[k] <= CLOSED) ? 0.0 : hyd->LinkFlow[k];
        newdir = SGN(q);

        // Indicate if flow is negligible
        if (fabs(q) < Q_STAGNANT) newdir = 0;

        // Reverse link's volume segments if flow direction changes sign
        if (newdir * olddir < 0) reversesegs(pr, k);

        // If flow direction changes either sign or magnitude then set
        // result to true (e.g., if a link's positive flow becomes
        // negligible then the network still needs to be re-sorted)
        if (newdir != olddir) result = TRUE;

        // ... replace old flow direction with the new direction
        qual->FlowDir[k] = newdir;
    }
    return result;
}
