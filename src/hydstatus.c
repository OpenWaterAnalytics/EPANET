/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       hydstatus.c
Description:  updates hydraulic status of network elements
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 11/10/2018
******************************************************************************
*/

#include <stdio.h>
#include "types.h"
#include "funcs.h"

// External functions
int  valvestatus(EN_Project pr);
int  linkstatus(EN_Project pr);

// Local functions
static StatType cvstatus(EN_Project pr, StatType, double, double);
static StatType pumpstatus(EN_Project pr, int, double);
static StatType prvstatus(EN_Project pr, int, StatType, double, double, double);
static StatType psvstatus(EN_Project pr, int, StatType, double, double, double);
static StatType fcvstatus(EN_Project pr, int, StatType, double, double);
static void     tankstatus(EN_Project pr, int, int, int);


int  valvestatus(EN_Project pr)
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
    int   change = FALSE,            // Status change flag
          i, k,                      // Valve & link indexes
          n1, n2;                    // Start & end nodes
    StatType  status;                // Valve status settings
    double hset;                     // Valve head setting
    Slink *link;

    network_t        *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;

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
            hset = net->Node[n2].El + hyd->LinkSetting[k];
            hyd->LinkStatus[k] = prvstatus(pr, k, status, hset,
                                     hyd->NodeHead[n1], hyd->NodeHead[n2]);
            break;
        case PSV:
            hset = net->Node[n1].El + hyd->LinkSetting[k];
            hyd->LinkStatus[k] = psvstatus(pr, k, status, hset,
                                     hyd->NodeHead[n1], hyd->NodeHead[n2]);
            break;
        default:
            continue;
        }

        // Check for a status change
        if (status != hyd->LinkStatus[k])
        {
            if (rep->Statflag == FULL)
            {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }
            change = TRUE;
        }
    }
    return change;
}


int  linkstatus(EN_Project pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if any link changes status, 0 otherwise
**  Purpose: determines new status for pumps, CVs, FCVs & pipes
**           to tanks.
**--------------------------------------------------------------
*/
{
    int change = FALSE,             // Status change flag
        k,                          // Link index
        n1,                         // Start node index
        n2;                         // End node index
    double dh;                      // Head difference across link
    StatType  status;               // Current status

    network_t        *net = &pr->network;
    hydraulics_t     *hyd = &pr->hydraulics;
    report_options_t *rep = &pr->report;
    Slink *link;

    // Examine each link
    for (k = 1; k <= net->Nlinks; k++)
    {
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;
        dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];

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
                                          hyd->LinkFlows[k]);
        }
        if (link->Type == PUMP && hyd->LinkStatus[k] >= OPEN &&
            hyd->LinkSetting[k] > 0.0)
        {
            hyd->LinkStatus[k] = pumpstatus(pr, k, -dh);
        }

        // Check for status changes in non-fixed FCVs
        if (link->Type == FCV && hyd->LinkSetting[k] != MISSING)
        {
            hyd->LinkStatus[k] = fcvstatus(pr, k, status, hyd->NodeHead[n1],
                                           hyd->NodeHead[n2]);
        }

        // Check for flow into (out of) full (empty) tanks
        if (n1 > net->Njuncs || n2 > net->Njuncs)
        {
            tankstatus(pr, k, n1, n2);
        }

        // Note any change in link status; do not revise link flow
        if (status != hyd->LinkStatus[k])
        {
            change = TRUE;
            if (rep->Statflag == FULL)
            {
                writestatchange(pr, k, status, hyd->LinkStatus[k]);
            }
        }
    }
    return change;
}


StatType  cvstatus(EN_Project pr, StatType s, double dh, double q)
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
    hydraulics_t *hyd = &pr->hydraulics;

    // Prevent reverse flow through CVs
    if (ABS(dh) > hyd->Htol)
    {
        if (dh < -hyd->Htol)     return CLOSED;
        else if (q < -hyd->Qtol) return CLOSED;
        else                     return OPEN;
    }
    else
    {
        if (q < -hyd->Qtol) return CLOSED;
        else                return s;
    }
}


StatType  pumpstatus(EN_Project pr, int k, double dh)
/*
**--------------------------------------------------
**  Input:   k  = link index
**           dh = head gain across link
**  Output:  returns new pump status
**  Purpose: updates status of an open pump.
**--------------------------------------------------
*/
{
    int   p;
    double hmax;
    hydraulics_t *hyd = &pr->hydraulics;
    network_t    *net = &pr->network;

    // Find maximum head (hmax) pump can deliver
    p = findpump(net, k);
    if (net->Pump[p].Ptype == CONST_HP)
    {
        // Use huge value for constant HP pump
        hmax = BIG;
    }
    else
    {
        // Use speed-adjusted shut-off head for other pumps
        hmax = SQR(hyd->LinkSetting[k]) * net->Pump[p].Hmax;
    }

    // Check if currrent head gain exceeds pump's max. head
    if (dh > hmax + hyd->Htol) return XHEAD;

    // No check is made to see if flow exceeds pump's max. flow
    return OPEN;
}


StatType  prvstatus(EN_Project pr, int k, StatType s, double hset,
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
    StatType  status;             // Valve's new status
    double hml;                   // Head loss when fully opened
    hydraulics_t *hyd = &pr->hydraulics;

    double htol = hyd->Htol;
    Slink *link = &pr->network.Link[k];

    // Head loss when fully open
    hml = link->Km * SQR(hyd->LinkFlows[k]);

    // Rules for updating valve's status from current value s
    status = s;
    switch (s)
    {
    case ACTIVE:
        if (hyd->LinkFlows[k] < -hyd->Qtol)  status = CLOSED;
        else if (h1 - hml < hset - htol)     status = OPEN;
        else                                 status = ACTIVE;
        break;

    case OPEN:
        if (hyd->LinkFlows[k] < -hyd->Qtol)  status = CLOSED;
        else if (h2 >= hset + htol)          status = ACTIVE;
        else                                 status = OPEN;
        break;

    case CLOSED:
        if (h1 >= hset + htol && h2 < hset - htol)   status = ACTIVE;
        else if (h1 < hset - htol && h1 > h2 + htol) status = OPEN;
        else                                         status = CLOSED;
        break;

    case XPRESSURE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) status = CLOSED;
        break;

    default:
        break;
    }
    return status;
}


StatType  psvstatus(EN_Project pr, int k, StatType s, double hset,
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
    StatType  status;             // Valve's new status
    double hml;                   // Head loss when fully opened
    hydraulics_t *hyd = &pr->hydraulics;

    double htol = hyd->Htol;
    Slink *link = &pr->network.Link[k];

    // Head loss when fully open
    hml = link->Km * SQR(hyd->LinkFlows[k]);

    // Rules for updating valve's status from current value s
    status = s;
    switch (s)
    {
    case ACTIVE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) status = CLOSED;
        else if (h2 + hml > hset + htol)    status = OPEN;
        else                                status = ACTIVE;
        break;

    case OPEN:
        if (hyd->LinkFlows[k] < -hyd->Qtol) status = CLOSED;
        else if (h1 < hset - htol)          status = ACTIVE;
        else                                status = OPEN;
        break;

    case CLOSED:
        if (h2 > hset + htol && h1 > h2 + htol)       status = OPEN;
        else if (h1 >= hset + htol && h1 > h2 + htol) status = ACTIVE;
        else                                          status = CLOSED;
        break;

    case XPRESSURE:
        if (hyd->LinkFlows[k] < -hyd->Qtol) status = CLOSED;
        break;

    default:
        break;
    }
    return status;
}


StatType  fcvstatus(EN_Project pr, int k, StatType s, double h1, double h2)
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
    StatType  status;             // New valve status
    hydraulics_t *hyd = &pr->hydraulics;

    status = s;
    if (h1 - h2 < -hyd->Htol)
    {
        status = XFCV;
    }
    else if (hyd->LinkFlows[k] < -hyd->Qtol)
    {
        status = XFCV;
    }
    else if (s == XFCV && hyd->LinkFlows[k] >= hyd->LinkSetting[k])
    {
        status = ACTIVE;
    }
    return status;
}


void  tankstatus(EN_Project pr, int k, int n1, int n2)
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
    network_t    *net = &pr->network;
    Slink *link = &net->Link[k];

    // Return if link is closed
    if (hyd->LinkStatus[k] <= CLOSED) return;

    // Make node n1 be the tank, reversing flow (q) if need be
    q = hyd->LinkFlows[k];
    i = n1 - net->Njuncs;
    if (i <= 0)
    {
        i = n2 - net->Njuncs;
        if (i <= 0) return;
        n = n1;
        n1 = n2;
        n2 = n;
        q = -q;
    }

    // Ignore reservoirs
    tank = &net->Tank[i];
    if (tank->A == 0.0) return;

    // Find head difference across link
    h = hyd->NodeHead[n1] - hyd->NodeHead[n2];

    // If tank is full, then prevent flow into it
    if (hyd->NodeHead[n1] >= tank->Hmax - hyd->Htol)
    {
        // Case 1: Link is a pump discharging into tank
        if (link->Type == PUMP)
        {
            if (link->N2 == n1) hyd->LinkStatus[k] = TEMPCLOSED;
        }

        // Case 2: Downstream head > tank head
        // (e.g., an open outflow check valve would close)
        else if (cvstatus(pr, OPEN, h, q) == CLOSED) 
        {
            hyd->LinkStatus[k] = TEMPCLOSED;
        }
    }

    // If tank is empty, then prevent flow out of it
    if (hyd->NodeHead[n1] <= tank->Hmin + hyd->Htol)
    {
        // Case 1: Link is a pump discharging from tank
        if (link->Type == PUMP)
        {
            if (link->N1 == n1) hyd->LinkStatus[k] = TEMPCLOSED;
        }

        // Case 2: Tank head > downstream head
        // (e.g., a closed outflow check valve would open)
        else if (cvstatus(pr, CLOSED, h, q) == OPEN)
        {
            hyd->LinkStatus[k] = TEMPCLOSED;
        }
    }
}
