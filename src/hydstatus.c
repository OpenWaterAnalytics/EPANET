/*
*********************************************************************

HYDSTATUS.C --  Hydraulic status updating for the EPANET Program

*******************************************************************
*/

#include <stdio.h>
#include "types.h"
#include "funcs.h"

// External functions
int  valvestatus(EN_Project *pr);
int  linkstatus(EN_Project *pr);
int  tankstatus(EN_Project *pr);

// Local functions
static StatType cvstatus(EN_Project *pr, StatType, double, double);
static StatType pumpstatus(EN_Project *pr, int, double);
static StatType prvstatus(EN_Project *pr, int, StatType, double, double, double);
static StatType psvstatus(EN_Project *pr, int, StatType, double, double, double);
static StatType fcvstatus(EN_Project *pr, int, StatType, double, double);
static void     tanklinkstatus(EN_Project *pr, int, int, int);
static void     tank2linkstatus(EN_Project *pr, int, int, int);


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
    int   change = FALSE,            // Status change flag
          i, k,                      // Valve & link indexes
          n1, n2;                    // Start & end nodes
    StatType  status;                // Valve status settings
    double hset;                     // Valve head setting
    Slink *link;

    EN_Network       *net = &pr->network;
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
        case EN_PRV:
            hset = net->Node[n2].El + hyd->LinkSetting[k];
            hyd->LinkStatus[k] = prvstatus(pr, k, status, hset,
                                     hyd->NodeHead[n1], hyd->NodeHead[n2]);
            break;
        case EN_PSV:
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
}                       /* End of valvestatus() */


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
    int change = FALSE,             // Status change flag
        k,                          // Link index
        n1,                         // Start node index
        n2;                         // End node index
    double dh;                      // Head difference across link
    StatType  status;               // Current status

    EN_Network       *net = &pr->network;
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
        if (link->Type == EN_CVPIPE)
        {
            hyd->LinkStatus[k] = cvstatus(pr, hyd->LinkStatus[k], dh,
                                          hyd->LinkFlows[k]);
        }
        if (link->Type == EN_PUMP && hyd->LinkStatus[k] >= OPEN &&
            hyd->LinkSetting[k] > 0.0)
        {
            hyd->LinkStatus[k] = pumpstatus(pr, k, -dh);
        }

        // Check for status changes in non-fixed FCVs
        if (link->Type == EN_FCV && hyd->LinkSetting[k] != MISSING)
        {
            hyd->LinkStatus[k] = fcvstatus(pr, k, status, hyd->NodeHead[n1],
                                           hyd->NodeHead[n2]);
        }

        // Check for flow into (out of) full (empty) tanks
        if (n1 > net->Njuncs || n2 > net->Njuncs)
        {
            tanklinkstatus(pr, k, n1, n2);
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


StatType  cvstatus(EN_Project *pr, StatType s, double dh, double q)
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


StatType  pumpstatus(EN_Project *pr, int k, double dh)
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
    EN_Network *net = &pr->network;

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


StatType  prvstatus(EN_Project *pr, int k, StatType s, double hset,
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


StatType  psvstatus(EN_Project *pr, int k, StatType s, double hset,
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


int tankstatus(EN_Project *pr)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns TRUE if a tank changes its fixed grade status
**  Purpose: updates the fixed grade status of all tanks.
**-----------------------------------------------------------
*/
{
    int i, j, emptying, filling, empty, full, fixedgrade;
    int change = FALSE;

    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network   *net = &pr->network;
    Stank *tank;

    // Only apply to implicit tank analysis after time 0
    if (hyd->TankDynamics != IMPLICIT) return FALSE;
    if (pr->time_options.Htime == 0) return FALSE;

    // Examine each tank
    for (j = 1; j <= net->Ntanks; j++)
    {
        // Skip reservoirs
        tank = &net->Tank[j];
        if (tank->A == 0.0) continue;

        // Determine current state of tank and its net inflow
        i = tank->Node;
        empty = (hyd->NodeHead[i] <= tank->Hmin + hyd->Htol);
        full = (hyd->NodeHead[i] >= tank->Hmax - hyd->Htol);
        emptying = (hyd->NodeDemand[i] < -hyd->Qtol);
        filling = (hyd->NodeDemand[i] > hyd->Qtol);
        fixedgrade = hyd->Xtank[j].FixedGrade;

        // See if a fixed grade tank becomes variable level
        if (fixedgrade)
        {
            if ((empty && filling) ||
                (full && emptying)) fixedgrade = FALSE;
        }

        // See if a variable level tank becomes fixed grade
        else
        {
            if (empty && emptying)
            {
                fixedgrade = TRUE;
                hyd->NodeHead[i] = tank->Hmin;
            }
            else if (full && filling)
            {
                fixedgrade = TRUE;
                hyd->NodeHead[i] = tank->Hmax;
            }
        }

        // Update tank's fixed grade status
        if (fixedgrade != hyd->Xtank[j].FixedGrade)
        {
            hyd->Xtank[j].FixedGrade = fixedgrade;
            change = TRUE;
        }
    }
    return change;
}


void  tanklinkstatus(EN_Project *pr, int k, int n1, int n2)
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
    EN_Network   *net = &pr->network;
    Slink        *link = &net->Link[k];

    // Return if link is closed
    if (hyd->LinkStatus[k] <= CLOSED) return;

    // Special case of link connecting two tanks
    if (n1 > net->Njuncs && n2 > net->Njuncs)
    {
        tank2linkstatus(pr, k, n1, n2);
        return;
    }

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
        if (link->Type == EN_PUMP)
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
        if (link->Type == EN_PUMP)
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


void tank2linkstatus(EN_Project *pr, int k, int n1, int n2)
/*
**----------------------------------------------------------------
**  Input:   k  = link index
**           n1 = start node of link
**           n2 = end node of link
**  Output:  none
**  Purpose: closes a link between two tanks if one is empty
**           and has outflow or one is full and has inflow.
**----------------------------------------------------------------
*/
{
    int    j1, j2, closetank = FALSE;
    double h1, h2;

    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network *net = &pr->network;
    Stank *tank1, *tank2;

    // Find current heads at each tank
    h1 = hyd->NodeHead[n1];
    h2 = hyd->NodeHead[n2];
    j1 = n1 - net->Njuncs;
    j2 = n2 - net->Njuncs;
    tank1 = &net->Tank[j1];
    tank2 = &net->Tank[j2];

    // If first tank not a reservoir see if it must be closed off
    if (tank1->A > 0.0)
    {
        // close tank if full and second tank sends flow to it
        if (h1 >= tank1->Hmax - hyd->Htol && h2 >= h1) closetank = TRUE;
        // close tank if empty and sends flow to second tank
        else if (h1 <= tank1->Hmin + hyd->Htol && h1 >= h2) closetank = TRUE;
    }

    // First tank not closed - see if second tank should be
    if (tank2->A > 0.0 && !closetank)
    {
        // close tank if full and first tank sends flow to it
        if (h2 >= tank2->Hmax - hyd->Htol && h1 >= h2) closetank = TRUE;
        // close tank if empty and sends flow to first tank
        else if (h2 <= tank2->Hmin + hyd->Htol && h2 >= h1) closetank = TRUE;
    }

    // Temporarily close the link connected to a closed tank
    if (closetank) hyd->LinkStatus[k] = TEMPCLOSED;
}
