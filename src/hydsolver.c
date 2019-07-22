/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       hydsolver.c
 Description:  computes flows and pressures throughout a pipe network using
               Todini's Global Gradient Algorithm
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 07/15/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "funcs.h"
#include "text.h"

// Hydraulic balance error for network being analyzed
typedef struct {
    double maxheaderror;
    double maxflowerror;
    double maxflowchange;
    int    maxheadlink;
    int    maxflownode;
    int    maxflowlink;
} Hydbalance;

// Exported functions
int  hydsolve(Project *, int *, double *);

// Imported functions
extern int  linsolve(Smatrix *, int);  //(see SMATRIX.C)
extern int  valvestatus(Project *);    //(see HYDSTATUS.C)
extern int  linkstatus(Project *);     //(see HYDSTATUS.C)

// Local functions
static int    badvalve(Project *, int);
static int    pswitch(Project *);

static double newflows(Project *, Hydbalance *);
static void   newlinkflows(Project *, Hydbalance *, double *, double *);
static void   newemitterflows(Project *, Hydbalance *, double *, double *);
static void   newdemandflows(Project *, Hydbalance *, double *, double *);

static void   checkhydbalance(Project *, Hydbalance *);
static int    hasconverged(Project *, double *, Hydbalance *);
static int    pdaconverged(Project *);
static void   reporthydbal(Project *, Hydbalance *);


int  hydsolve(Project *pr, int *iter, double *relerr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  *iter   = # of iterations to reach solution
**           *relerr = convergence error in solution
**           returns error code
**  Purpose: solves network nodal equations for heads and flows
**           using Todini's Gradient algorithm
**
**  Notes:   Status checks on CVs, pumps and pipes to tanks are made
**           every CheckFreq iteration, up until MaxCheck iterations
**           are reached. Status checks on control valves are made
**           every iteration if DampLimit = 0 or only when the
**           convergence error is at or below DampLimit. If DampLimit
**           is > 0 then future computed flow changes are only 60% of
**           their full value. A complete status check on all links
**           is made when convergence is achieved. If convergence is
**           not achieved in MaxIter trials and ExtraIter > 0 then
**           another ExtraIter trials are made with no status changes
**           made to any links and a warning message is generated.
**
**   This procedure calls linsolve() which appears in SMATRIX.C.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;
    Report  *rpt = &pr->report;

    int    i;                     // Node index
    int    errcode = 0;           // Node causing solution error
    int    nextcheck;             // Next status check trial
    int    maxtrials;             // Max. trials for convergence
    double newerr;                // New convergence error
    int    valveChange;           // Valve status change flag
    int    statChange;            // Non-valve status change flag
    Hydbalance hydbal;            // Hydraulic balance errors
    double fullDemand;            // Full demand for a node (cfs)

    // Initialize status checking & relaxation factor
    nextcheck = hyd->CheckFreq;
    hyd->RelaxFactor = 1.0;
    
    // Initialize convergence criteria and PDA results
    hydbal.maxheaderror = 0.0;
    hydbal.maxflowchange = 0.0;
    hyd->DeficientNodes = 0;
    hyd->DemandReduction = 0.0;

    // Repeat iterations until convergence or trial limit is exceeded.
    // (ExtraIter used to increase trials in case of status cycling.)
    if (rpt->Statflag == FULL) writerelerr(pr, 0, 0);
    maxtrials = hyd->MaxIter;
    if (hyd->ExtraIter > 0) maxtrials += hyd->ExtraIter;
    *iter = 1;
    while (*iter <= maxtrials)
    {
        // Compute coefficient matrices A & F and solve A*H = F
        // where H = heads, A = Jacobian coeffs. derived from
        // head loss gradients, & F = flow correction terms.
        // Solution for H is returned in F from call to linsolve().

        headlosscoeffs(pr);
        matrixcoeffs(pr);
        errcode = linsolve(sm, net->Njuncs);

        // Matrix ill-conditioning problem - if control valve causing problem,
        // fix its status & continue, otherwise quit with no solution.
        if (errcode > 0)
        {
            if (badvalve(pr, sm->Order[errcode])) continue;
            else break;
        }

        // Update current solution.
        // (Row[i] = row of solution matrix corresponding to node i)
        for (i = 1; i <= net->Njuncs; i++)
        {
            hyd->NodeHead[i] = sm->F[sm->Row[i]];   // Update heads
        }
        newerr = newflows(pr, &hydbal);             // Update flows
        *relerr = newerr;

        // Write convergence error to status report if called for
        if (rpt->Statflag == FULL)
        {
            writerelerr(pr, *iter, *relerr);
        }

        // Apply solution damping & check for change in valve status
        hyd->RelaxFactor = 1.0;
        valveChange = FALSE;
        if (hyd->DampLimit > 0.0)
        {
            if (*relerr <= hyd->DampLimit)
            {
                hyd->RelaxFactor = 0.6;
                valveChange = valvestatus(pr);
            }
        }
        else
        {
            valveChange = valvestatus(pr);
        }

        // Check for convergence
        if (hasconverged(pr, relerr, &hydbal))
        {
            // We have convergence - quit if we are into extra iterations
            if (*iter > hyd->MaxIter) break;

            // Quit if no status changes occur
            statChange = FALSE;
            if (valveChange)    statChange = TRUE;
            if (linkstatus(pr)) statChange = TRUE;
            if (pswitch(pr))    statChange = TRUE;
            if (!statChange)    break;

            // We have a status change so continue the iterations
            nextcheck = *iter + hyd->CheckFreq;
        }

        // No convergence yet - see if its time for a periodic status
        // check  on pumps, CV's, and pipes connected to tank
        else if (*iter <= hyd->MaxCheck && *iter == nextcheck)
        {
            linkstatus(pr);
            nextcheck += hyd->CheckFreq;
        }
        (*iter)++;
    }

    // Iterations ended - report any errors.
    if (errcode > 0)
    {
        writehyderr(pr, sm->Order[errcode]);    // Ill-conditioned matrix error
        errcode = 110;
    }

    // Store actual junction outflow in NodeDemand & full demand in DemandFlow
    for (i = 1; i <= net->Njuncs; i++)
    {
        fullDemand = hyd->NodeDemand[i];
        hyd->NodeDemand[i] = hyd->DemandFlow[i] + hyd->EmitterFlow[i];
        hyd->DemandFlow[i] = fullDemand;
    }

    // Save convergence info
    hyd->RelativeError = *relerr;
    hyd->MaxHeadError = hydbal.maxheaderror;
    hyd->MaxFlowChange = hydbal.maxflowchange;
    hyd->Iterations = *iter;
    return errcode;
}


int  badvalve(Project *pr, int n)
/*
**-----------------------------------------------------------------
**  Input:   n = node index
**  Output:  returns 1 if node n belongs to an active control valve,
**           0 otherwise
**  Purpose: determines if a node belongs to an active control valve
**           whose setting causes an inconsistent set of eqns. If so,
**           the valve status is fixed open and a warning condition
**           is generated.
**-----------------------------------------------------------------
*/
{
    Network *net  = &pr->network;
    Hydraul *hyd  = &pr->hydraul;
    Report  *rpt  = &pr->report;
    Times   *time = &pr->times;

    int i, k, n1, n2;
    Slink *link;
    LinkType t;

    for (i = 1; i <= net->Nvalves; i++)
    {
        k = net->Valve[i].Link;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        if (n == n1 || n == n2)
        {
            t = link->Type;
            if (t == PRV || t == PSV || t == FCV)
            {
                if (hyd->LinkStatus[k] == ACTIVE)
                {
                    if (rpt->Statflag == FULL)
                    {
                        sprintf(pr->Msg, FMT61,
                                clocktime(rpt->Atime, time->Htime), link->ID);
                        writeline(pr, pr->Msg);
                    }
                    if (link->Type == FCV) hyd->LinkStatus[k] = XFCV;
                    else                   hyd->LinkStatus[k] = XPRESSURE;
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}


int  pswitch(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if status of any link changes, 0 if not
**  Purpose: adjusts settings of links controlled by junction
**           pressures after a hydraulic solution is found
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;

    int   i,                 // Control statement index
          k,                 // Index of link being controlled
          n,                 // Node controlling link k
          reset,             // Flag on control conditions
          change,            // Flag for status or setting change
          anychange = 0;     // Flag for 1 or more control actions
    char  s;                 // Current link status
    Slink *link;

    // Check each control statement
    for (i = 1; i <= net->Ncontrols; i++)
    {
        reset = 0;
        k = net->Control[i].Link;
        if (k <= 0) continue;

        // Determine if control based on a junction, not a tank
        n = net->Control[i].Node;
        if (n > 0 && n <= net->Njuncs)
        {
            // Determine if control conditions are satisfied
            if (net->Control[i].Type == LOWLEVEL &&
                hyd->NodeHead[n] <= net->Control[i].Grade + hyd->Htol)
            {
                reset = 1;
            }
            if (net->Control[i].Type == HILEVEL &&
                hyd->NodeHead[n] >= net->Control[i].Grade - hyd->Htol)
            {
                reset = 1;
            }
        }

        // Determine if control forces a status or setting change
        if (reset == 1)
        {
            link = &net->Link[k];
            change = 0;
            s = hyd->LinkStatus[k];
            if (link->Type == PIPE)
            {
                if (s != net->Control[i].Status) change = 1;
            }
            if (link->Type == PUMP)
            {
                if (hyd->LinkSetting[k] != net->Control[i].Setting) change = 1;
            }
            if (link->Type >= PRV)
            {
                if (hyd->LinkSetting[k] != net->Control[i].Setting) change = 1;
                else if (hyd->LinkSetting[k] == MISSING && s != net->Control[i].Status)
                {
                    change = 1;
                }
            }

            // If a change occurs, update status & setting
            if (change)
            {
                hyd->LinkStatus[k] = net->Control[i].Status;
                if (link->Type > PIPE)
                {
                    hyd->LinkSetting[k] = net->Control[i].Setting;
                }
                if (rpt->Statflag == FULL)
                {
                    writestatchange(pr, k, s, hyd->LinkStatus[k]);
                }
                anychange = 1;
            }
        }
    }
    return anychange;
}


double newflows(Project *pr, Hydbalance *hbal)
/*
**----------------------------------------------------------------
**  Input:   hbal = ptr. to hydraulic balance information
**  Output:  returns solution convergence error
**  Purpose: updates link, emitter & demand flows after new
**           nodal heads are computed.
**----------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    double  dqsum,                 // Network flow change
            qsum;                  // Network total flow

    // Initialize sum of flows & corrections
    qsum = 0.0;
    dqsum = 0.0;
    hbal->maxflowchange = 0.0;
    hbal->maxflowlink = 1;
    hbal->maxflownode = -1;

    // Update flows in all real and virtual links
    newlinkflows(pr, hbal, &qsum, &dqsum);
    newemitterflows(pr, hbal, &qsum, &dqsum);
    newdemandflows(pr, hbal, &qsum, &dqsum);

    // Return ratio of total flow corrections to total flow
    if (qsum > hyd->Hacc) return (dqsum / qsum);
    else return dqsum;
}


void  newlinkflows(Project *pr, Hydbalance *hbal, double *qsum, double *dqsum)
/*
**----------------------------------------------------------------
**  Input:   hbal = ptr. to hydraulic balance information
**           qsum = sum of current system flows
**           dqsum = sum of system flow changes
**  Output:  updates hbal, qsum and dqsum
**  Purpose: updates link flows after new nodal heads computed
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    double  dh,                    /* Link head loss       */
            dq;                    /* Link flow change     */
    int     k, n, n1, n2;
    Slink   *link;

    // Initialize net inflows (i.e., demands) at fixed grade nodes
    for (n = net->Njuncs + 1; n <= net->Nnodes; n++)
    {
        hyd->NodeDemand[n] = 0.0;
    }

    // Examine each link
    for (k = 1; k <= net->Nlinks; k++)
    {
        // Get link and its end nodes
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;

        // Apply flow update formula:
        //   dq = Y - P * (new head loss)
        //    P = 1 / (previous head loss gradient)
        //    Y = P * (previous head loss)
        // where P & Y were computed in hlosscoeff() in hydcoeffs.c

        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
        dq = hyd->Y[k] - hyd->P[k] * dh;

        // Adjust flow change by the relaxation factor
        dq *= hyd->RelaxFactor;

        // Prevent flow in constant HP pumps from going negative
        if (link->Type == PUMP)
        {
            n = findpump(net, k);
            if (net->Pump[n].Ptype == CONST_HP && dq > hyd->LinkFlow[k])
            {
                dq = hyd->LinkFlow[k] / 2.0;
            }
        }

        // Update link flow and system flow summation
        hyd->LinkFlow[k] -= dq;
        *qsum += ABS(hyd->LinkFlow[k]);
        *dqsum += ABS(dq);

        // Update identity of element with max. flow change
        if (ABS(dq) > hbal->maxflowchange)
        {
            hbal->maxflowchange = ABS(dq);
            hbal->maxflowlink = k;
            hbal->maxflownode = -1;
        }

        // Update net flows to fixed grade nodes
        if (hyd->LinkStatus[k] > CLOSED)
        {
            if (n1 > net->Njuncs) hyd->NodeDemand[n1] -= hyd->LinkFlow[k];
            if (n2 > net->Njuncs) hyd->NodeDemand[n2] += hyd->LinkFlow[k];
        }
    }
}


void newemitterflows(Project *pr, Hydbalance *hbal, double *qsum,
                     double *dqsum)
/*
**----------------------------------------------------------------
**  Input:   hbal = ptr. to hydraulic balance information
**           qsum = sum of current system flows
**           dqsum = sum of system flow changes
**  Output:  updates hbal, qsum and dqsum
**  Purpose: updates nodal emitter flows after new nodal heads computed
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int     i;
    double  hloss, hgrad, dh, dq;

    // Examine each network junction
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junction if it does not have an emitter
        if (net->Node[i].Ke == 0.0) continue;

        // Find emitter head loss and gradient
        emitterheadloss(pr, i, &hloss, &hgrad);

        // Find emitter flow change
        dh = hyd->NodeHead[i] - net->Node[i].El;
        dq = (hloss - dh) / hgrad;
        dq *= hyd->RelaxFactor;
        hyd->EmitterFlow[i] -= dq;

        // Update system flow summation
        *qsum += ABS(hyd->EmitterFlow[i]);
        *dqsum += ABS(dq);

        // Update identity of element with max. flow change
        if (ABS(dq) > hbal->maxflowchange)
        {
            hbal->maxflowchange = ABS(dq);
            hbal->maxflownode = i;
            hbal->maxflowlink = -1;
        }
    }
}


void newdemandflows(Project *pr, Hydbalance *hbal, double *qsum, double *dqsum)
/*
**----------------------------------------------------------------
**  Input:   hbal = ptr. to hydraulic balance information
**           qsum = sum of current system flows
**           dqsum = sum of system flow changes
**  Output:  updates hbal, qsum and dqsum
**  Purpose: updates nodal pressure dependent demand flows after
**           new nodal heads computed
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    double  dp,         // pressure range over which demand can vary (ft)
            dq,         // change in demand flow (cfs)
            n,          // exponent in head loss v. demand  function
            hloss,      // current head loss through outflow junction (ft)
            hgrad,      // head loss gradient with respect to flow (ft/cfs)
            dh;         // new head loss through outflow junction (ft)
    int     i;
    
    // Get demand function parameters
    if (hyd->DemandModel == DDA) return;
    dp = MAX((hyd->Preq - hyd->Pmin), MINPDIFF);
    n = 1.0 / hyd->Pexp;

    // Examine each junction
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions with no positive demand
        if (hyd->NodeDemand[i] <= 0.0) continue;
        
        // Find change in demand flow (see hydcoeffs.c)
        demandheadloss(pr, i, dp, n, &hloss, &hgrad);
        dh = hyd->NodeHead[i] - net->Node[i].El - hyd->Pmin;
        dq = (hloss - dh) / hgrad;
        dq *= hyd->RelaxFactor;
        hyd->DemandFlow[i] -= dq;

        // Update system flow summation
        *qsum += ABS(hyd->DemandFlow[i]);
        *dqsum += ABS(dq);

        // Update identity of element with max. flow change
        if (ABS(dq) > hbal->maxflowchange)
        {
            hbal->maxflowchange = ABS(dq);
            hbal->maxflownode = i;
            hbal->maxflowlink = -1;
        }
    }
}


void  checkhydbalance(Project *pr, Hydbalance *hbal)
/*
**--------------------------------------------------------------
**   Input:   hbal = hydraulic balance errors
**   Output:  none
**   Purpose: finds the link with the largest head imbalance
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int k, n1, n2;
    double dh, headerror, headloss;
    Slink *link;

    hbal->maxheaderror = 0.0;
    hbal->maxheadlink = 1;
    headlosscoeffs(pr);
    for (k = 1; k <= net->Nlinks; k++)
    {
        if (hyd->LinkStatus[k] <= CLOSED) continue;
        if (hyd->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
        headloss = hyd->Y[k] / hyd->P[k];
        headerror = ABS(dh - headloss);
        if (headerror > hbal->maxheaderror)
        {
            hbal->maxheaderror = headerror;
            hbal->maxheadlink = k;
        }
    }
}


int  hasconverged(Project *pr, double *relerr, Hydbalance *hbal)
/*
**--------------------------------------------------------------
**   Input:   relerr = current total relative flow change
**            hbal   = current hydraulic balance errors
**   Output:  returns 1 if system has converged or 0 if not
**   Purpose: checks various criteria to see if system has
**            become hydraulically balanced
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    
    // Check that total relative flow change is small enough
    if (*relerr > hyd->Hacc) return 0;

    // Find largest head loss error and absolute flow change
    checkhydbalance(pr, hbal);
    if (pr->report.Statflag == FULL)
    {
        reporthydbal(pr, hbal);
    }
    
    // Check that head loss error and flow change criteria are met
    if (hyd->HeadErrorLimit > 0.0 &&
        hbal->maxheaderror > hyd->HeadErrorLimit) return 0;
    if (hyd->FlowChangeLimit > 0.0 &&
        hbal->maxflowchange > hyd->FlowChangeLimit) return 0;
        
    // Check for pressure driven analysis convergence
    if (hyd->DemandModel == PDA) return pdaconverged(pr);
    return 1;
}

int pdaconverged(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns 1 if PDA converged, 0 if not
**   Purpose: checks if pressure driven analysis has converged
**            and updates total demand deficit
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    const double TOL = 0.001;
    int i, converged = 1;
    double totalDemand = 0.0, totalReduction = 0.0;
    
    hyd->DeficientNodes = 0;
    hyd->DemandReduction = 0.0;
    
    // Add up number of junctions with demand deficits
    for (i = 1; i <= pr->network.Njuncs; i++)
    {
        // Skip nodes whose required demand is non-positive
        if (hyd->NodeDemand[i] <= 0.0) continue;
        
        // Check for negative demand flow or positive demand flow at negative pressure
        if (hyd->DemandFlow[i] < -TOL) converged = 0;
        if (hyd->DemandFlow[i] > TOL &&
            hyd->NodeHead[i] - pr->network.Node[i].El - hyd->Pmin < -TOL)
            converged = 0;
            
        // Accumulate total required demand and demand deficit
        if (hyd->DemandFlow[i] + 0.0001 < hyd->NodeDemand[i])
        {
            hyd->DeficientNodes++;
            totalDemand += hyd->NodeDemand[i];
            totalReduction += hyd->NodeDemand[i] - hyd->DemandFlow[i];
        }
    }
    if (totalDemand > 0.0)
        hyd->DemandReduction = totalReduction / totalDemand * 100.0;
    return converged;
}


void  reporthydbal(Project *pr, Hydbalance *hbal)
/*
**--------------------------------------------------------------
**   Input:   hbal   = current hydraulic balance errors
**   Output:  none
**   Purpose: identifies links with largest flow change and
**            largest head loss error.
**--------------------------------------------------------------
*/
{
    double qchange = hbal->maxflowchange * pr->Ucf[FLOW];
    double herror = hbal->maxheaderror * pr->Ucf[HEAD];
    int    qlink = hbal->maxflowlink;
    int    qnode = hbal->maxflownode;
    int    hlink = hbal->maxheadlink;
    if (qlink >= 1)
    {
        sprintf(pr->Msg, FMT66, qchange, pr->network.Link[qlink].ID);
        writeline(pr, pr->Msg);
    }
    else if (qnode >= 1)
    {
        sprintf(pr->Msg, FMT67, qchange, pr->network.Node[qnode].ID);
        writeline(pr, pr->Msg);
    }
    if (hlink >= 1)
    {
        sprintf(pr->Msg, FMT68, herror, pr->network.Link[hlink].ID);
        writeline(pr, pr->Msg);
    }
}
