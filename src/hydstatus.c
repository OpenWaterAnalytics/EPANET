/*
******************************************************************************
Project:      OWA EPANET
Version:      2.3
Module:       hydstatus.c
Description:  updates hydraulic status of network elements
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 02/03/2023
******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "funcs.h"

// Exported functions
int  valvestatus(Project *);
int  linkstatus(Project *);

// Local functions
static StatusType cvstatus(Project *, StatusType, double, double);
static StatusType pumpstatus(Project *, int, double);
static StatusType prvstatus(Project *, int, StatusType, double, double, double);
static StatusType psvstatus(Project *, int, StatusType, double, double, double);
static StatusType fcvstatus(Project *, int, StatusType, double, double);
static void       tankstatus(Project *, int, int, double);


int  valvestatus(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;

    int    change = FALSE,   // Status change flag
           i, k,             // Valve & link indexes
           n1, n2;           // Start & end nodes
    double hset;             // Valve head setting
    StatusType status;       // Valve status settings
    Slink *link;

    // Examine each valve
    for (i = 1; i <= net->Nvalves; i++)
    {
        // Get valve's link and its index
        k = net->Valve[i].Link;
        link = &net->Link[k];

        // Ignore valve if its status is fixed to OPEN/CLOSED
        if (hyd->LinkSetting[k] == MISSING) continue;

        // Get start/end node indexes & save current status
        n1 = link->N1;
        n2 = link->N2;
        status = hyd->LinkStatus[k];

        // Evaluate valve's new status
        switch (link->Type)
        {
        case PRV:
            hset = hydheadtosolver(pr,
                net->Node[n2].El + hyd->LinkSetting[k]);
            hyd->LinkStatus[k] = prvstatus(pr, k, status, hset,
                                     hyd->SolverState.NodeHead[n1],
                                     hyd->SolverState.NodeHead[n2]);
            break;
        case PSV:
            hset = hydheadtosolver(pr,
                net->Node[n1].El + hyd->LinkSetting[k]);
            hyd->LinkStatus[k] = psvstatus(pr, k, status, hset,
                                     hyd->SolverState.NodeHead[n1],
                                     hyd->SolverState.NodeHead[n2]);
            break;
        default:
            continue;
        }

        // Check for a status change
        if (status != hyd->LinkStatus[k])
        {
            if (rpt->Statflag == FULL)
            {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }
            change = TRUE;
        }
    }
    return change;
}


int  linkstatus(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if any link changes status, 0 otherwise
**  Purpose: determines new status for pumps, CVs, FCVs & pipes
**           to tanks.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;

    int change = FALSE,             // Status change flag
        k,                          // Link index
        n1,                         // Start node index
        n2;                         // End node index
    double dh;                      // Head difference across link
    StatusType  status;             // Current status
    Slink *link;

    // Examine each link
    for (k = 1; k <= net->Nlinks; k++)
    {
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->SolverState.NodeHead[n1] - hyd->SolverState.NodeHead[n2];

        // Re-open temporarily closed links (status = XHEAD or TEMPCLOSED)
        status = hyd->LinkStatus[k];
        if (status == XHEAD || status == TEMPCLOSED)
        {
            hyd->LinkStatus[k] = OPEN;
        }

        // Check for status changes in CVs and pumps
        if (link->Type == CVPIPE)
        {
            hyd->LinkStatus[k] = cvstatus(pr, hyd->LinkStatus[k], dh,
                                          hyd->SolverState.LinkFlow[k]);
        }
        if (link->Type == PUMP && hyd->LinkStatus[k] >= OPEN &&
            hyd->LinkSetting[k] > 0.0)
        {
            hyd->LinkStatus[k] = pumpstatus(pr, k, -dh);
        }

        // Check for status changes in non-fixed FCVs
        if (link->Type == FCV && hyd->LinkSetting[k] != MISSING)
        {
            hyd->LinkStatus[k] = fcvstatus(pr, k, status,
                                           hyd->SolverState.NodeHead[n1],
                                           hyd->SolverState.NodeHead[n2]);
        }

        // Check for flow into (out of) full (empty) tanks
        if (n1 > net->Njuncs)
            tankstatus(pr, k, n1, hyd->SolverState.LinkFlow[k]);
        if (n2 > net->Njuncs)
            tankstatus(pr, k, n2, -hyd->SolverState.LinkFlow[k]);

        // Note any change in link status; do not revise link flow
        if (status != hyd->LinkStatus[k])
        {
            change = TRUE;
            if (rpt->Statflag == FULL)
            {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }
        }
    }
    return change;
}


StatusType  cvstatus(Project *pr, StatusType s, double dh, double q)
/*
**--------------------------------------------------
**  Input:   s  = current link status
**           dh = head loss across link
**           q  = link flow
**  Output:  returns new link status
**  Purpose: updates status of a check valve link.
**--------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    double htol = hydheadtosolver(pr, hyd->Htol);
    double qtol = hydflowtosolver(pr, hyd->Qtol);

    // dh and q are solver quantities, so compare them against scaled
    // versions of EPANET's dimensional status tolerances.
    if (ABS(dh) > htol)
    {
        if (dh < -htol)     return CLOSED;
        else if (q < -qtol) return CLOSED;
        else                return OPEN;
    }
    else
    {
        if (q < -qtol) return CLOSED;
        else           return s;
    }
}


StatusType  pumpstatus(Project *pr, int k, double dh)
/*
**--------------------------------------------------
**  Input:   k  = link index
**           dh = head gain across link
**  Output:  returns new pump status
**  Purpose: updates status of an open pump.
**--------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Network *net = &pr->network;

    int   p;
    double hmax, htol;

    // Find maximum head (hmax) pump can deliver. Pump limits and Htol are
    // dimensional model values, while dh is already in solver head units.
    p = findpump(net, k);
    htol = hydheadtosolver(pr, hyd->Htol);
    if (net->Pump[p].Ptype == CONST_HP)
    {
        // Use huge value for constant HP pump
        hmax = hydheadtosolver(pr, BIG);
        if (hyd->SolverState.LinkFlow[k] < hydflowtosolver(pr, TINY))
            return TEMPCLOSED;
    }
    else
    {
        // Use speed-adjusted shut-off head for other pumps
        hmax = hydheadtosolver(pr,
            SQR(hyd->LinkSetting[k]) * net->Pump[p].Hmax);
    }

    // Check if current head gain exceeds pump's max. head
    if (dh > hmax + htol) return XHEAD;

    // No check is made to see if flow exceeds pump's max. flow
    return OPEN;
}


StatusType  prvstatus(Project *pr, int k, StatusType s, double hset,
                    double h1, double h2)
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
    Hydraul *hyd = &pr->hydraul;

    StatusType status;             // Valve's new status
    double  hml;                   // Head loss when fully opened
    double  htol, qtol;
    Slink   *link;

    htol = hydheadtosolver(pr, hyd->Htol);
    qtol = hydflowtosolver(pr, hyd->Qtol);
    link = &pr->network.Link[k];

    // Head loss when fully open. Km is stored in dimensional model units.
    hml = hydminorlosstosolver(pr, link->Km) *
          SQR(hyd->SolverState.LinkFlow[k]);

    // Rules for updating valve's status from current value s
    status = s;
    switch (s)
    {
    case ACTIVE:
        if (hyd->SolverState.LinkFlow[k] < -qtol)  status = CLOSED;
        else if (h1 - hml < hset - htol)     status = OPEN;
        else                                 status = ACTIVE;
        break;

    case OPEN:
        if (hyd->SolverState.LinkFlow[k] < -qtol)  status = CLOSED;
        else if (h2 >= hset + htol)          status = ACTIVE;
        else                                 status = OPEN;
        break;

    case CLOSED:
        if (h1 >= hset + htol && h2 < hset - htol)   status = ACTIVE;
        else if (h1 < hset - htol && h1 > h2 + htol) status = OPEN;
        else                                         status = CLOSED;
        break;

    case XPRESSURE:
        if (hyd->SolverState.LinkFlow[k] < -qtol) status = CLOSED;
        break;

    default:
        break;
    }
    return status;
}


StatusType  psvstatus(Project *pr, int k, StatusType s, double hset,
                    double h1, double h2)
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
    Hydraul *hyd = &pr->hydraul;

    StatusType status;             // Valve's new status
    double  hml;                   // Head loss when fully opened
    double  htol, qtol;
    Slink   *link;

    htol = hydheadtosolver(pr, hyd->Htol);
    qtol = hydflowtosolver(pr, hyd->Qtol);
    link = &pr->network.Link[k];

    // Head loss when fully open. Km is stored in dimensional model units.
    hml = hydminorlosstosolver(pr, link->Km) *
          SQR(hyd->SolverState.LinkFlow[k]);

    // Rules for updating valve's status from current value s
    status = s;
    switch (s)
    {
    case ACTIVE:
        if (hyd->SolverState.LinkFlow[k] < -qtol) status = CLOSED;
        else if (h2 + hml > hset + htol)    status = OPEN;
        else                                status = ACTIVE;
        break;

    case OPEN:
        if (hyd->SolverState.LinkFlow[k] < -qtol) status = CLOSED;
        else if (h1 < hset - htol)          status = ACTIVE;
        else                                status = OPEN;
        break;

    case CLOSED:
        if (h2 > hset + htol && h1 > h2 + htol)       status = OPEN;
        else if (h1 >= hset + htol && h1 > h2 + htol) status = ACTIVE;
        else                                          status = CLOSED;
        break;

    case XPRESSURE:
        if (hyd->SolverState.LinkFlow[k] < -qtol) status = CLOSED;
        break;

    default:
        break;
    }
    return status;
}


StatusType  fcvstatus(Project *pr, int k, StatusType s, double h1, double h2)
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
    Hydraul *hyd = &pr->hydraul;
    StatusType status;            // New valve status
    double htol = hydheadtosolver(pr, hyd->Htol);
    double qtol = hydflowtosolver(pr, hyd->Qtol);
    double qset = hydflowtosolver(pr, hyd->LinkSetting[k]);
    double km = hydminorlosstosolver(pr, pr->network.Link[k].Km);

    status = s;
    if (h1 - h2 < -htol)
    {
        status = XFCV;
    }
    else if (hyd->SolverState.LinkFlow[k] < -qtol)
    {
        status = XFCV;
    }
    else if (s == XFCV && hyd->SolverState.LinkFlow[k] >= qset)
    {
        status = ACTIVE;
    }

    // Active valve's loss coeff. can't be < fully open loss coeff.
    else if (status == ACTIVE)
    {
        if ((h1 - h2) / SQR(hyd->SolverState.LinkFlow[k]) < km)
        {
            status = XFCV;
        }
    }        
    return status;
}


void  tankstatus(Project *pr, int k, int n, double q)
/*
**----------------------------------------------------------------
**  Input:   k = link index
**           n = tank node index
**           q = link flow rate out of (+) or into (-) tank
**  Output:  none
**  Purpose: closes link flowing into full or out of empty tank
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int   i;
    Stank *tank;

    // Return if link is closed
    if (hyd->LinkStatus[k] <= CLOSED) return;

    // Ignore reservoirs
    i = n - net->Njuncs;
    tank = &net->Tank[i];
    if (tank->A == 0.0) return;
    
    // Can't add flow to a full tank
    if (tank->V >= tank->Vmax && !tank->CanOverflow && q < 0.0)
        hyd->LinkStatus[k] = TEMPCLOSED;
 
    // Can't remove flow from an empty tank
    else if (tank->V <= tank->Vmin && q > 0.0)
        hyd->LinkStatus[k] = TEMPCLOSED;
}
