/*
*********************************************************************

HYDSOLVER.C --  Equilibrium hydraulic solver for the EPANET Program

This module contains a pipe network hydraulic solver that computes
flows and pressures within the network at a specific point in time.

The solver implements Todini's Global Gradient Algorithm.
*******************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include "text.h"
#include "types.h"
#include "funcs.h"

// Hydraulic balance error for network being analyzed
typedef struct {
    double maxheaderror;
    double maxflowerror;
    double maxflowchange;
    int    maxheadlink;
    int    maxflownode;
    int    maxflowlink;
} Hydbalance;

// External functions
//int   hydsolve(EN_Project *pr, int *iter, double *relerr);
//void  headlosscoeffs(EN_Project *pr);
//void  matrixcoeffs(EN_Project *pr);

extern int  valvestatus(EN_Project *pr);    //(see HYDSTATUS.C)
extern int  linkstatus(EN_Project *pr);     //(see HYDSTATUS.C)

// Local functions
static int      badvalve(EN_Project *pr, int);
static int      pswitch(EN_Project *pr);

static double   newflows(EN_Project *pr, Hydbalance *hbal);
static void     newlinkflows(EN_Project *pr, Hydbalance *hbal, double *qsum,
                double *dqsum);
static void     newemitterflows(EN_Project *pr, Hydbalance *hbal, double *qsum,
                double *dqsum);
static void     newdemandflows(EN_Project *pr, Hydbalance *hbal, double *qsum,
                double *dqsum);

static void     checkhydbalance(EN_Project *pr, Hydbalance *hbal);
static int      hasconverged(EN_Project *pr, double *relerr, Hydbalance *hbal);
static void     reporthydbal(EN_Project *pr, Hydbalance *hbal);


int  hydsolve(EN_Project *pr, int *iter, double *relerr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  *iter   = # of iterations to reach solution
**           *relerr = convergence error in solution
**           returns error code
**  Purpose: solves network nodal equations for heads and flows
**           using Todini's Gradient algorithm
**
*** Updated 9/7/00 ***
*** Updated 2.00.11 ***
*** Updated 2.00.12 ***
**  Notes:   Status checks on CVs, pumps and pipes to tanks are made
**           every hyd->CheckFreq iteration, up until MaxCheck iterations
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
    int    i;                     // Node index
    int    errcode = 0;           // Node causing solution error
    int    nextcheck;             // Next status check trial
    int    maxtrials;             // Max. trials for convergence
    double newerr;                // New convergence error
    int    valveChange;           // Valve status change flag
    int    statChange;            // Non-valve status change flag
    Hydbalance hydbal;            // Hydraulic balance errors

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    solver_t         *sol = &hyd->solver;
    report_options_t *rep = &pr->report;

    // Initialize status checking & relaxation factor
    nextcheck = hyd->CheckFreq;
    hyd->RelaxFactor = 1.0;

    // Repeat iterations until convergence or trial limit is exceeded.
    // (ExtraIter used to increase trials in case of status cycling.)
    if (pr->report.Statflag == FULL)
    {
        writerelerr(pr, 0, 0);
    }
    maxtrials = hyd->MaxIter;
    if (hyd->ExtraIter > 0)
    {
        maxtrials += hyd->ExtraIter;
    }
    *iter = 1;
    while (*iter <= maxtrials)
    {
        /*
        ** Compute coefficient matrices A & F and solve A*H = F
        ** where H = heads, A = Jacobian coeffs. derived from
        ** head loss gradients, & F = flow correction terms.
        ** Solution for H is returned in F from call to linsolve().
        */
        headlosscoeffs(pr);
        matrixcoeffs(pr);
        errcode = linsolve(pr, net->Njuncs);

        // Quit if memory allocation error
        if (errcode < 0) break;

        // Matrix ill-conditioning problem - if control valve causing problem,
        // fix its status & continue, otherwise quit with no solution.
        if (errcode > 0)
        {                      
            if (badvalve(pr, sol->Order[errcode])) continue;
            else break;
        }

        // Update current solution.
        // (Row[i] = row of solution matrix corresponding to node i)
        for (i = 1; i <= net->Njuncs; i++)
        {
            hyd->NodeHead[i] = sol->F[sol->Row[i]];   // Update heads
        }
        newerr = newflows(pr, &hydbal);               // Update flows
        *relerr = newerr;

        // Write convergence error to status report if called for
        if (rep->Statflag == FULL)
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
    if (errcode < 0) errcode = 101;              // Memory allocation error
    else if (errcode > 0)
    {
        writehyderr(pr, sol->Order[errcode]);    // Ill-conditioned matrix error
        errcode = 110;
    }

    // Replace junction demands with total outflow values
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->NodeDemand[i] = hyd->DemandFlows[i] + hyd->EmitterFlows[i];
    }

    // Save convergence info
    hyd->RelativeError = *relerr;
    hyd->MaxHeadError = hydbal.maxheaderror;
    hyd->MaxFlowChange = hydbal.maxflowchange;
    hyd->Iterations = *iter;

    return(errcode);
}


int  badvalve(EN_Project *pr, int n)
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
    int i, k, n1, n2;

    EN_Network       *net  = &pr->network;
    hydraulics_t     *hyd  = &pr->hydraulics;
    report_options_t *rep  = &pr->report;
    time_options_t   *time = &pr->time_options;
    Slink *link;
    EN_LinkType t;

    for (i = 1; i <= net->Nvalves; i++)
    {
        k = net->Valve[i].Link;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        if (n == n1 || n == n2)
        {
            t = link->Type;
            if (t == EN_PRV || t == EN_PSV || t == EN_FCV)
            {
                if (hyd->LinkStatus[k] == ACTIVE)
                {
                    if (rep->Statflag == FULL)
                    {
                        sprintf(pr->Msg, FMT61, clocktime(rep->Atime, time->Htime), link->ID);
                        writeline(pr, pr->Msg);
                    }
                    if (link->Type == EN_FCV)
                    {
                        hyd->LinkStatus[k] = XFCV;
                    }
                    else
                    {
                        hyd->LinkStatus[k] = XPRESSURE;
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}


int  pswitch(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if status of any link changes, 0 if not
**  Purpose: adjusts settings of links controlled by junction
**           pressures after a hydraulic solution is found
**--------------------------------------------------------------
*/
{
    int   i,                 // Control statement index
          k,                 // Index of link being controlled
          n,                 // Node controlling link k
          reset,             // Flag on control conditions
          change,            // Flag for status or setting change
          anychange = 0;     // Flag for 1 or more control actions
    char  s;                 // Current link status

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;
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
            if (link->Type == EN_PIPE)
            {
                if (s != net->Control[i].Status) change = 1;
            }
            if (link->Type == EN_PUMP)
            {
                if (hyd->LinkSetting[k] != net->Control[i].Setting) change = 1;
            }
            if (link->Type >= EN_PRV)
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
                if (link->Type > EN_PIPE)
                {
                    hyd->LinkSetting[k] = net->Control[i].Setting;
                }
                if (rep->Statflag == FULL)
                {
                    writestatchange(pr, k, s, hyd->LinkStatus[k]);
                }
                anychange = 1;
            }
        }
    }
    return(anychange);
}


double newflows(EN_Project *pr, Hydbalance *hbal)
/*
**----------------------------------------------------------------
**  Input:   hbal = ptr. to hydraulic balance information
**  Output:  returns solution convergence error
**  Purpose: updates link, emitter & demand flows after new
**           nodal heads are computed.
**----------------------------------------------------------------
*/
{
    double  dqsum,                 // Network flow change
            qsum;                  // Network total flow

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;

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


void  newlinkflows(EN_Project *pr, Hydbalance *hbal, double *qsum, double *dqsum)
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
    double  dh,                    /* Link head loss       */
            dq;                    /* Link flow change     */
    int     k, n, n1, n2;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link;

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
        dq = sol->Y[k] - sol->P[k] * dh;

        // Adjust flow change by the relaxation factor
        dq *= hyd->RelaxFactor;

        // Prevent flow in constant HP pumps from going negative
        if (link->Type == EN_PUMP)
        {
            n = findpump(net, k);
            if (net->Pump[n].Ptype == CONST_HP && dq > hyd->LinkFlows[k])
            {
                dq = hyd->LinkFlows[k] / 2.0;
            }
        }

        // Update link flow and system flow summation
        hyd->LinkFlows[k] -= dq;
        *qsum += ABS(hyd->LinkFlows[k]);
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
            if (n1 > net->Njuncs) hyd->NodeDemand[n1] -= hyd->LinkFlows[k];
            if (n2 > net->Njuncs) hyd->NodeDemand[n2] += hyd->LinkFlows[k];
        }
    }
}


void newemitterflows(EN_Project *pr, Hydbalance *hbal, double *qsum,
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
    int     i;
    double  hloss, hgrad, dh, dq;
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;

    // Examine each network junction
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junction if it does not have an emitter
        if (net->Node[i].Ke == 0.0) continue;

        // Find emitter head loss and gradient 
        emitheadloss(pr, i, &hloss, &hgrad);

        // Find emitter flow change
        dh = hyd->NodeHead[i] - net->Node[i].El;
        dq = (hloss - dh) / hgrad;
        hyd->EmitterFlows[i] -= dq;

        // Update system flow summation
        *qsum += ABS(hyd->EmitterFlows[i]);
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


void newdemandflows(EN_Project *pr, Hydbalance *hbal, double *qsum, double *dqsum)
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
    double  dp,         // pressure range over which demand can vary (ft)
            dq,         // change in demand flow (cfs)
            n;          // exponent in head loss v. demand  function
    int     k;
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;

    // Get demand function parameters
    if (hyd->DemandModel == DDA) return;
    demandparams(pr, &dp, &n);

    // Examine each junction
    for (k = 1; k <= net->Njuncs; k++)
    {
        // Skip junctions with no positive demand
        if (hyd->NodeDemand[k] <= 0.0) continue;

        // Find change in demand flow (see hydcoeffs.c)
        dq = demandflowchange(pr, k, dp, n);
        hyd->DemandFlows[k] -= dq;

        // Update system flow summation
        *qsum += ABS(hyd->DemandFlows[k]);
        *dqsum += ABS(dq);

        // Update identity of element with max. flow change
        if (ABS(dq) > hbal->maxflowchange)
        {
            hbal->maxflowchange = ABS(dq);
            hbal->maxflownode = k;
            hbal->maxflowlink = -1;
        }
    }
}


void  checkhydbalance(EN_Project *pr, Hydbalance *hbal)
/*
**--------------------------------------------------------------
**   Input:   hbal = hydraulic balance errors
**   Output:  none
**   Purpose: finds the link with the largest head imbalance
**--------------------------------------------------------------
*/
{
    int k, n1, n2;
    double dh, headerror, headloss;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link;
    hbal->maxheaderror = 0.0;
    hbal->maxheadlink = 1;
    headlosscoeffs(pr);
    for (k = 1; k <= net->Nlinks; k++)
    {
        if (hyd->LinkStatus[k] <= CLOSED) continue;
        if (sol->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
        headloss = sol->Y[k] / sol->P[k];
        headerror = ABS(dh - headloss);
        if (headerror > hbal->maxheaderror)
        {
            hbal->maxheaderror = headerror;
            hbal->maxheadlink = k;
        }
    }
}


int  hasconverged(EN_Project *pr, double *relerr, Hydbalance *hbal)
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
    hydraulics_t *hyd = &pr->hydraulics;

    if (*relerr > hyd->Hacc) return 0;
    checkhydbalance(pr, hbal);
    if (pr->report.Statflag == FULL)
    {
        reporthydbal(pr, hbal);
    }
    if (hyd->HeadErrorLimit > 0.0 &&
        hbal->maxheaderror > hyd->HeadErrorLimit) return 0;
    if (hyd->FlowChangeLimit > 0.0 &&
        hbal->maxflowchange > hyd->FlowChangeLimit) return 0;
    return 1;
}


void  reporthydbal(EN_Project *pr, Hydbalance *hbal)
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
