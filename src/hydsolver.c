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

// Error in network being hydraulically balanced
typedef struct {
    double maxheaderror;
    double maxflowerror;
    double maxflowchange;
    int    maxheadlink;
    int    maxflownode;
    int    maxflowlink;
} Hydbalance;

// External functions
//int  hydsolve(EN_Project *pr, int *iter, double *relerr);

// Local functions
static int      badvalve(EN_Project *pr, int);
static int      valvestatus(EN_Project *pr);
static int      linkstatus(EN_Project *pr);
static StatType cvstatus(EN_Project *pr, StatType, double, double);
static StatType pumpstatus(EN_Project *pr, int, double);
static StatType prvstatus(EN_Project *pr, int, StatType, double, double, double);
static StatType psvstatus(EN_Project *pr, int, StatType, double, double, double);
static StatType fcvstatus(EN_Project *pr, int, StatType, double, double);
static void     tankstatus(EN_Project *pr, int, int, int);
static int      pswitch(EN_Project *pr);

static double   newflows(EN_Project *pr, Hydbalance *hbal);
static double   emitflowchange(EN_Project *pr, int i);
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
**           not achieved in hyd->MaxIter trials and hyd->ExtraIter > 0 then
**           another hyd->ExtraIter trials are made with no status changes
**           made to any links and a warning message is generated.
**
**   This procedure calls linsolve() which appears in SMATRIX.C.
**-------------------------------------------------------------------
*/
{
    int    i;                     /* Node index */
    int    errcode = 0;           /* Node causing solution error */
    int    nextcheck;             /* Next status check trial */
    int    maxtrials;             /* Max. trials for convergence */
    double newerr;                /* New convergence error */
    int    valveChange;           /* Valve status change flag */
    int    statChange;            /* Non-valve status change flag */
    Hydbalance hydbal;            /* Hydraulic balance errors */

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    solver_t         *sol = &hyd->solver;
    report_options_t *rep = &pr->report;

    /* Initialize status checking & relaxation factor */
    nextcheck = hyd->CheckFreq;
    hyd->RelaxFactor = 1.0;

    /* Repeat iterations until convergence or trial limit is exceeded. */
    /* (hyd->ExtraIter used to increase trials in case of status cycling.)  */
    if (pr->report.Statflag == FULL) {
        writerelerr(pr, 0, 0);
    }
    maxtrials = hyd->MaxIter;
    if (hyd->ExtraIter > 0) {
        maxtrials += hyd->ExtraIter;
    }
    *iter = 1;
    while (*iter <= maxtrials) {
        /*
        ** Compute coefficient matrices A & F and solve A*H = F
        ** where H = heads, A = Jacobian coeffs. derived from
        ** head loss gradients, & F = flow correction terms.
        ** Solution for H is returned in F from call to linsolve().
        */
        for (i = 1; i <= net->Nlinks; i++) {
            hlosscoeff(pr, i);
        }
        matrixcoeffs(pr);
        errcode = linsolve(pr, net->Njuncs);

        /* Take action depending on error code */
        if (errcode < 0) {
            break;                              /* Memory allocation problem */
        }
        if (errcode > 0) {                      /* Ill-conditioning problem */
            /* If control valve causing problem, fix its status & continue, */
            /* otherwise end the iterations with no solution.               */
            if (badvalve(pr, sol->Order[errcode])) {
                continue;
            }
            else break;
        }

        /* Update current solution. */
        /* (Row[i] = row of solution matrix corresponding to node i). */
        for (i = 1; i <= net->Njuncs; i++) {
            hyd->NodeHead[i] = sol->F[sol->Row[i]];   /* Update heads */
        }
        newerr = newflows(pr, &hydbal);               /* Update flows */
        *relerr = newerr;

        /* Write convergence error to status report if called for */
        if (rep->Statflag == FULL) {
            writerelerr(pr, *iter, *relerr);
        }

        /* Apply solution damping & check for change in valve status */
        hyd->RelaxFactor = 1.0;
        valveChange = FALSE;
        if (hyd->DampLimit > 0.0) {
            if (*relerr <= hyd->DampLimit) {
                hyd->RelaxFactor = 0.6;
                valveChange = valvestatus(pr);
            }
        }
        else {
            valveChange = valvestatus(pr);
        }

        /* Check for convergence */
        if (hasconverged(pr, relerr, &hydbal)) {
            /* We have convergence. Quit if we are into extra iterations. */
            if (*iter > hyd->MaxIter) {
                break;
            }

            /* Quit if no status changes occur. */
            statChange = FALSE;
            if (valveChange) {
                statChange = TRUE;
            }
            if (linkstatus(pr)) {
                statChange = TRUE;
            }
            if (pswitch(pr)) {
                statChange = TRUE;
            }
            if (!statChange) {
                break;
            }

            /* We have a status change so continue the iterations */
            nextcheck = *iter + hyd->CheckFreq;
        }

        /* No convergence yet. See if its time for a periodic status */
        /* check  on pumps, CV's, and pipes connected to tanks.      */
        else if (*iter <= hyd->MaxCheck && *iter == nextcheck)
        {
            linkstatus(pr);
            nextcheck += hyd->CheckFreq;
        }
        (*iter)++;
    }

    /* Iterations ended. Report any errors. */
    if (errcode < 0) errcode = 101;                /* Memory allocation error */
    else if (errcode > 0)
    {
        writehyderr(pr, sol->Order[errcode]);      /* Ill-conditioned eqns. error */
        errcode = 110;
    }

    /* Add any emitter flows to junction demands */
    for (i = 1; i <= net->Njuncs; i++) {
        hyd->NodeDemand[i] += hyd->EmitterFlows[i];
    }
    return(errcode);
}                        /* End of netsolve */


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

    for (i = 1; i <= net->Nvalves; i++) {
        k = net->Valve[i].Link;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        if (n == n1 || n == n2) {
            EN_LinkType t = link->Type;
            if (t == EN_PRV || t == EN_PSV || t == EN_FCV) {
                if (hyd->LinkStatus[k] == ACTIVE) {
                    if (rep->Statflag == FULL) {
                        sprintf(pr->Msg, FMT61, clocktime(rep->Atime, time->Htime), link->ID);
                        writeline(pr, pr->Msg);
                    }
                    if (link->Type == EN_FCV) {
                        hyd->LinkStatus[k] = XFCV;
                    }
                    else {
                        hyd->LinkStatus[k] = XPRESSURE;
                    }
                    return(1);
                }
            }
            return(0);
        }
    }
    return(0);
}


int  valvestatus(EN_Project *pr)
/*
**-----------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if any pressure or flow control valve
**           changes status, 0 otherwise
**  Purpose: updates status for PRVs & PSVs whose status
**           is not fixed to OPEN/CLOSED
**-----------------------------------------------------------------
*/
{
    int   change = FALSE,            /* Status change flag      */
          i, k,                      /* Valve & link indexes    */
          n1, n2;                    /* Start & end nodes       */
    StatType  status;                /* Valve status settings   */
    double hset;                     /* Valve head setting      */
    Slink *link;

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;

    for (i = 1; i <= net->Nvalves; i++)                /* Examine each valve   */
    {
        k = net->Valve[i].Link;                        /* Link index of valve  */
        link = &net->Link[k];
        if (hyd->LinkSetting[k] == MISSING) {
            continue;                                  /* Valve status fixed   */
        }
        n1 = link->N1;                                 /* Start & end nodes    */
        n2 = link->N2;
        status = hyd->LinkStatus[k];                   /* Save current status  */

//      if (s != CLOSED                                /* No change if flow is */  
//      && ABS(hyd->LinkFlows[k]) < hyd->Qtol) continue;  /* negligible.          */  

        switch (link->Type)                     /* Evaluate new status: */
        {
        case EN_PRV:
            hset = net->Node[n2].El + hyd->LinkSetting[k];
            hyd->LinkStatus[k] = prvstatus(pr, k, status, hset, hyd->NodeHead[n1], hyd->NodeHead[n2]);
            break;
        case EN_PSV:
            hset = net->Node[n1].El + hyd->LinkSetting[k];
            hyd->LinkStatus[k] = psvstatus(pr, k, status, hset, hyd->NodeHead[n1], hyd->NodeHead[n2]);
            break;

////  FCV status checks moved back into the linkstatus() function ////           
//         case FCV:  S[k] = fcvstatus(k,s,hyd->NodeHead[n1],hyd->NodeHead[n2]); 
//                    break;                                                     

        default:
            continue;
        }

        /*** Updated 9/7/00 ***/
        /* Do not reset flow in valve if its status changes. */
        /* This strategy improves convergence. */

        /* Check for status change */
        if (status != hyd->LinkStatus[k])
        {
            if (rep->Statflag == FULL) {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }
            change = TRUE;
        }
    }
    return(change);
}                       /* End of valvestatus() */


/*** Updated 9/7/00 ***/
int  linkstatus(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if any link changes status, 0 otherwise
**  Purpose: determines new status for pumps, CVs, FCVs & pipes
**           to tanks.
**--------------------------------------------------------------
*/
{
    int    change = FALSE,             /* Status change flag      */
           k,                          /* Link index              */
           n1,                         /* Start node index        */
           n2;                         /* End node index          */
    double dh;                         /* Head difference         */
    StatType  status;                  /* Current status          */

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;
    Slink *link;

    /* Examine each Slink */
    for (k = 1; k <= net->Nlinks; k++)
    {
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];

        /* Re-open temporarily closed links (status = XHEAD or TEMPCLOSED) */
        status = hyd->LinkStatus[k];
        if (status == XHEAD || status == TEMPCLOSED) {
            hyd->LinkStatus[k] = OPEN;
        }

        /* Check for status changes in CVs and pumps */
        if (link->Type == EN_CVPIPE) {
            hyd->LinkStatus[k] = cvstatus(pr, hyd->LinkStatus[k], dh, hyd->LinkFlows[k]);
        }
        if (link->Type == EN_PUMP && hyd->LinkStatus[k] >= OPEN && hyd->LinkSetting[k] > 0.0) {
            hyd->LinkStatus[k] = pumpstatus(pr, k, -dh);
        }

        /* Check for status changes in non-fixed FCVs */
        if (link->Type == EN_FCV && hyd->LinkSetting[k] != MISSING) {
            hyd->LinkStatus[k] = fcvstatus(pr, k, status, hyd->NodeHead[n1], hyd->NodeHead[n2]);                               //
        }

        /* Check for flow into (out of) full (empty) tanks */
        if (n1 > net->Njuncs || n2 > net->Njuncs) {
            tankstatus(pr, k, n1, n2);
        }

        /* Note change in link status; do not revise link flow */
        if (status != hyd->LinkStatus[k]) {
            change = TRUE;
            if (rep->Statflag == FULL) {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }

            //if (S[k] <= CLOSED) hyd->LinkFlows[k] = QZERO;                                   
            //else setlinkflow(k, dh);                                            
        }
    }
    return(change);
}                        /* End of linkstatus */


StatType  cvstatus(EN_Project *pr, StatType s, double dh, double q)
/*
**--------------------------------------------------
**  Input:   s  = current status
**           dh = headloss
**           q  = flow
**  Output:  returns new link status
**  Purpose: updates status of a check valve.
**--------------------------------------------------
*/
{
    hydraulics_t *hyd = &pr->hydraulics;

    /* Prevent reverse flow through CVs */
    if (ABS(dh) > hyd->Htol)
    {
        if (dh < -hyd->Htol) {
            return(CLOSED);
        }
        else if (q < -hyd->Qtol) {
            return(CLOSED);
        }
        else {
            return(OPEN);
        }
    }
    else
    {
        if (q < -hyd->Qtol) {
            return(CLOSED);
        }
        else {
            return(s);
        }
    }
}


StatType  pumpstatus(EN_Project *pr, int k, double dh)
/*
**--------------------------------------------------
**  Input:   k  = link index
**           dh = head gain
**  Output:  returns new pump status
**  Purpose: updates status of an open pump.
**--------------------------------------------------
*/
{
    int   p;
    double hmax;
    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network *net = &pr->network;

    /* Prevent reverse flow through pump */
    p = findpump(net, k);
    if (net->Pump[p].Ptype == CONST_HP) {
        hmax = BIG;
    }
    else {
        hmax = SQR(hyd->LinkSetting[k]) * net->Pump[p].Hmax;
    }
    if (dh > hmax + hyd->Htol) {
        return(XHEAD);
    }

    /*** Flow higher than pump curve no longer results in a status change ***/
    /* Check if pump cannot deliver flow */
    //if (hyd->LinkFlows[k] > K[k]*Pump[p].Qmax + hyd->Qtol) return(XFLOW);                       
    return(OPEN);
}


StatType  prvstatus(EN_Project *pr, int k, StatType s, double hset, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index
**           s    = current status
**           hset = valve head setting
**           h1   = head at upstream node
**           h2   = head at downstream node
**  Output:  returns new valve status
**  Purpose: updates status of a pressure reducing valve.
**-----------------------------------------------------------
*/
{
    StatType  status;     /* New valve status */
    double hml;        /* Minor headloss   */
    hydraulics_t *hyd = &pr->hydraulics;

    double htol = hyd->Htol;
    Slink *link = &pr->network.Link[k];

    status = s;
    if (hyd->LinkSetting[k] == MISSING)
        return(status);       /* Status fixed by user */
    hml = link->Km*SQR(hyd->LinkFlows[k]); /* Head loss when open  */

   /*** Status rules below have changed. ***/

    switch (s)
    {
    case ACTIVE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        else if (h1 - hml < hset - htol) {
            status = OPEN;
        }
        else {
            status = ACTIVE;
        }
        break;
    case OPEN:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        else if (h2 >= hset + htol) {
            status = ACTIVE;
        }
        else {
            status = OPEN;
        }
        break;
    case CLOSED:
        if (h1 >= hset + htol && h2 < hset - htol) {
            status = ACTIVE;
        }
        else if (h1 < hset - htol && h1 > h2 + htol) {
            status = OPEN;
        }
        else {
            status = CLOSED;
        }
        break;
    case XPRESSURE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        break;
    default:
        break;
    }
    return(status);
}


StatType  psvstatus(EN_Project *pr, int k, StatType s, double hset, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index
**           s    = current status
**           hset = valve head setting
**           h1   = head at upstream node
**           h2   = head at downstream node
**  Output:  returns new valve status
**  Purpose: updates status of a pressure sustaining valve.
**-----------------------------------------------------------
*/
{
    StatType  status;       /* New valve status */
    double hml;          /* Minor headloss   */
    hydraulics_t *hyd = &pr->hydraulics;

    double htol = hyd->Htol;

    Slink *link = &pr->network.Link[k];

    status = s;
    if (hyd->LinkSetting[k] == MISSING) {
        return(status);       /* Status fixed by user */
    }
    hml = link->Km*SQR(hyd->LinkFlows[k]);                /* Head loss when open  */

/*** Status rules below have changed. ***/

    switch (s)
    {
    case ACTIVE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        else if (h2 + hml > hset + htol) {
            status = OPEN;
        }
        else {
            status = ACTIVE;
        }
        break;
    case OPEN:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        else if (h1 < hset - htol) {
            status = ACTIVE;
        }
        else {
            status = OPEN;
        }
        break;
    case CLOSED:
        if (h2 > hset + htol && h1 > h2 + htol) {
            status = OPEN;
        }
        else if (h1 >= hset + htol && h1 > h2 + htol) {
            status = ACTIVE;
        }
        else {
            status = CLOSED;
        }
        break;
    case XPRESSURE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) {
            status = CLOSED;
        }
        break;
    default:
        break;
    }
    return(status);
}


StatType  fcvstatus(EN_Project *pr, int k, StatType s, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index
**           s    = current status
**           h1   = head at upstream node
**           h2   = head at downstream node
**  Output:  returns new valve status
**  Purpose: updates status of a flow control valve.
**
**    Valve status changes to XFCV if flow reversal.
**    If current status is XFCV and current flow is
**    above setting, then valve becomes active.
**    If current status is XFCV, and current flow
**    positive but still below valve setting, then
**    status remains same.
**-----------------------------------------------------------
*/
{
    StatType  status;        /* New valve status */
    hydraulics_t *hyd = &pr->hydraulics;

    status = s;
    if (h1 - h2 < -hyd->Htol) {
        status = XFCV;
    }
    else if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = XFCV;
    }
    else if (s == XFCV && hyd->LinkFlows[k] >= hyd->LinkSetting[k]) {
        status = ACTIVE;
    }
    return(status);
}


/*** Updated 9/7/00 ***/
/*** Updated 11/19/01 ***/
void  tankstatus(EN_Project *pr, int k, int n1, int n2)
/*
**----------------------------------------------------------------
**  Input:   k  = link index
**           n1 = start node of link
**           n2 = end node of link
**  Output:  none
**  Purpose: closes link flowing into full or out of empty tank
**----------------------------------------------------------------
*/
{
    int   i, n;
    double h, q;
    Stank *tank;

    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network *net = &pr->network;
    Slink *link = &net->Link[k];

    /* Make node n1 be the tank */
    q = hyd->LinkFlows[k];
    i = n1 - net->Njuncs;
    if (i <= 0) {
        i = n2 - net->Njuncs;
        if (i <= 0) {
            return;
        }
        n = n1;
        n1 = n2;
        n2 = n;
        q = -q;
    }
    h = hyd->NodeHead[n1] - hyd->NodeHead[n2];
    tank = &net->Tank[i];
    /* Skip reservoirs & closed links */
    if (tank->A == 0.0 || hyd->LinkStatus[k] <= CLOSED) {
        return;
    }

    /* If tank full, then prevent flow into it */
    if (hyd->NodeHead[n1] >= tank->Hmax - hyd->Htol)
    {

        /* Case 1: Link is a pump discharging into tank */
        if (link->Type == EN_PUMP) {
            if (link->N2 == n1)
                hyd->LinkStatus[k] = TEMPCLOSED;
        }

        /* Case 2: Downstream head > tank head */
        /* (i.e., an open outflow check valve would close) */
        else if (cvstatus(pr, OPEN, h, q) == CLOSED) {
            hyd->LinkStatus[k] = TEMPCLOSED;
        }
    }

    /* If tank empty, then prevent flow out of it */
    if (hyd->NodeHead[n1] <= tank->Hmin + hyd->Htol) {

        /* Case 1: Link is a pump discharging from tank */
        if (link->Type == EN_PUMP) {
            if (link->N1 == n1) {
                hyd->LinkStatus[k] = TEMPCLOSED;
            }
        }

        /* Case 2: Tank head > downstream head */
        /* (i.e., a closed outflow check valve would open) */
        else if (cvstatus(pr, CLOSED, h, q) == OPEN) {
            hyd->LinkStatus[k] = TEMPCLOSED;
        }
    }
}                        /* End of tankstatus */


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
    int   i,                 /* Control statement index */
          k,                 /* Link being controlled */
          n,                 /* Node controlling Slink */
          reset,             /* Flag on control conditions */
          change,            /* Flag for status or setting change */
          anychange = 0;     /* Flag for 1 or more changes */
    char  s;                 /* Current link status */

    EN_Network       *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;
    Slink *link;

    /* Check each control statement */
    for (i = 1; i <= net->Ncontrols; i++)
    {
        reset = 0;
        if ((k = net->Control[i].Link) <= 0) {
            continue;
        }

        /* Determine if control based on a junction, not a tank */
        if ((n = net->Control[i].Node) > 0 && n <= net->Njuncs) {
            /* Determine if control conditions are satisfied */
            if (net->Control[i].Type == LOWLEVEL
                && hyd->NodeHead[n] <= net->Control[i].Grade + hyd->Htol) {
                reset = 1;
            }
            if (net->Control[i].Type == HILEVEL
                && hyd->NodeHead[n] >= net->Control[i].Grade - hyd->Htol) {
                reset = 1;
            }
        }

        /* Determine if control forces a status or setting change */
        if (reset == 1)
        {
            link = &net->Link[k];
            change = 0;
            s = hyd->LinkStatus[k];
            if (link->Type == EN_PIPE) {
                if (s != net->Control[i].Status) {
                    change = 1;
                }
            }
            if (link->Type == EN_PUMP) {
                if (hyd->LinkSetting[k] != net->Control[i].Setting) {
                    change = 1;
                }
            }
            if (link->Type >= EN_PRV) {
                if (hyd->LinkSetting[k] != net->Control[i].Setting) {
                    change = 1;
                }
                else if (hyd->LinkSetting[k] == MISSING && s != net->Control[i].Status) {
                    change = 1;
                }
            }

            /* If a change occurs, update status & setting */
            if (change) {
                hyd->LinkStatus[k] = net->Control[i].Status;
                if (link->Type > EN_PIPE) {
                    hyd->LinkSetting[k] = net->Control[i].Setting;
                }
                if (rep->Statflag == FULL) {
                    writestatchange(pr, k, s, hyd->LinkStatus[k]);
                }

                /* Re-set flow if status has changed */
                //            if (S[k] != s) initlinkflow(k, S[k], K[k]);
                anychange = 1;
            }
        }
    }
    return(anychange);
}                        /* End of pswitch */


double newflows(EN_Project *pr, Hydbalance *hbal)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  returns solution convergence error
**  Purpose: updates link flows after new nodal heads computed
**----------------------------------------------------------------
*/
{
    double  dh,                    /* Link head loss       */
            dq;                    /* Link flow change     */
    double  dqsum,                 /* Network flow change  */
            qsum;                  /* Network total flow   */
    int     k, n, n1, n2;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link;

    /* Initialize net inflows (i.e., demands) at tanks */
    for (n = net->Njuncs + 1; n <= net->Nnodes; n++) {
        hyd->NodeDemand[n] = 0.0;
    }

    /* Initialize sum of flows & corrections */
    qsum = 0.0;
    dqsum = 0.0;

    hbal->maxflowchange = 0.0;
    hbal->maxflowlink = 1;

    /* Update flows in all links */
    for (k = 1; k <= net->Nlinks; k++)
    {
        link = &net->Link[k];
        /*
        ** Apply flow update formula:
        **   dq = Y - P*(new head loss)
        **    P = 1/(dh/dq)
        **    Y = P*(head loss based on current flow)
        ** where P & Y were computed in newcoeffs().
        */

        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
        dq = sol->Y[k] - sol->P[k] * dh;

        /* Adjust flow change by the relaxation factor */
        dq *= hyd->RelaxFactor;

        /* Prevent flow in constant HP pumps from going negative */
        if (link->Type == EN_PUMP) {
            n = findpump(net, k);
            if (net->Pump[n].Ptype == CONST_HP && dq > hyd->LinkFlows[k]) {
                dq = hyd->LinkFlows[k] / 2.0;
            }
        }
        hyd->LinkFlows[k] -= dq;

        /* Update sum of absolute flows & flow corrections */
        qsum += ABS(hyd->LinkFlows[k]);
        dqsum += ABS(dq);

        /* Update identity of link with max. flow change */
        if (ABS(dq) > hbal->maxflowchange) {
            hbal->maxflowchange = ABS(dq);
            hbal->maxflowlink = k;
         }

        /* Update net flows to tanks */
        if (hyd->LinkStatus[k] > CLOSED)
        {
            if (n1 > net->Njuncs) {
                hyd->NodeDemand[n1] -= hyd->LinkFlows[k];
            }
            if (n2 > net->Njuncs) {
                hyd->NodeDemand[n2] += hyd->LinkFlows[k];
            }
        }
    }

    /* Update emitter flows */
    for (k = 1; k <= net->Njuncs; k++)
    {
        if (net->Node[k].Ke == 0.0) {
            continue;
        }
        dq = emitflowchange(pr, k);
        hyd->EmitterFlows[k] -= dq;
        qsum += ABS(hyd->EmitterFlows[k]);
        dqsum += ABS(dq);
    }

    /* Return ratio of total flow corrections to total flow */
    if (qsum > hyd->Hacc) {
        return(dqsum / qsum);
    }
    else {
        return(dqsum);
    }
}                        /* End of newflows */


double  emitflowchange(EN_Project *pr, int i)
/*
**--------------------------------------------------------------
**   Input:   i = node index
**   Output:  returns change in flow at an emitter node
**   Purpose: computes flow change at an emitter node
**--------------------------------------------------------------
*/
{
    double ke, p;
    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network *n = &pr->network;
    Snode *node = &n->Node[i];

    ke = MAX(CSMALL, node->Ke);
    p = hyd->Qexp * ke * pow(ABS(hyd->EmitterFlows[i]), (hyd->Qexp - 1.0));
    if (p < hyd->RQtol) {
        p = 1 / hyd->RQtol;
    }
    else {
        p = 1.0 / p;
    }
    return(hyd->EmitterFlows[i] / hyd->Qexp - p * (hyd->NodeHead[i] - node->El));
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
    for (k = 1; k <= net->Nlinks; k++) {
        if (hyd->LinkStatus[k] <= CLOSED) continue;
        hlosscoeff(pr, k);
        if (sol->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
        headloss = sol->Y[k] / sol->P[k];
        headerror = ABS(dh - headloss);
        if (headerror > hbal->maxheaderror) {
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
    if (pr->report.Statflag == FULL) {
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
    int    hlink = hbal->maxheadlink;
    if (qlink >= 1) {
        sprintf(pr->Msg, FMT66, qchange, pr->network.Link[qlink].ID);
        writeline(pr, pr->Msg);
    }
    if (hlink >= 1) {
        sprintf(pr->Msg, FMT67, herror, pr->network.Link[hlink].ID);
        writeline(pr, pr->Msg);
    }
}
