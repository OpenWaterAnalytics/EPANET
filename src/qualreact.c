/*
*********************************************************************

QUALREACT.C -- water quality reaction module for the EPANET program

*********************************************************************
*/

#include <stdio.h>
#include <math.h>
#include "types.h"

// Exported Functions
char    setreactflag(EN_Project *pr);
double  getucf(double);
void    ratecoeffs(EN_Project *pr);
void    reactpipes(EN_Project *pr, long);
void    reacttanks(EN_Project *pr, long);
double  mixtank(EN_Project *pr, int, double, double ,double);

// Imported Functions
extern  void addseg(EN_Project *pr, int, double, double);

// Local Functions
static double  piperate(EN_Project *pr, int);
static double  pipereact(EN_Project *pr, int, double, double, long);
static double  tankreact(EN_Project *pr, double, double, double, long);
static double  bulkrate(EN_Project *pr, double, double, double);
static double  wallrate(EN_Project *pr, double, double, double, double);

static void    tankmix1(EN_Project *pr, int, double, double, double);
static void    tankmix2(EN_Project *pr, int, double, double, double);
static void    tankmix3(EN_Project *pr, int, double, double, double);
static void    tankmix4(EN_Project *pr, int, double, double, double);


char setreactflag(EN_Project *pr)
/*
**-----------------------------------------------------------
**   Input:   none
**   Output:  returns 1 for reactive WQ constituent, 0 otherwise
**   Purpose: checks if reactive chemical being simulated
**-----------------------------------------------------------
*/
{
    int i;
    EN_Network *net = &pr->network;

    if (pr->quality.Qualflag == TRACE) return 0;
    else if (pr->quality.Qualflag == AGE)   return 1;
    else
    {
        for (i = 1; i <= net->Nlinks; i++)
        {
            if (net->Link[i].Type <= EN_PIPE)
            {
                if (net->Link[i].Kb != 0.0 || net->Link[i].Kw != 0.0) return 1;
            }
        }
        for (i = 1; i <= net->Ntanks; i++)
        {
            if (net->Tank[i].Kb != 0.0) return 1;
        }
    }
    return 0;
}


double getucf(double order)
/*
**--------------------------------------------------------------
**   Input:   order = bulk reaction order
**   Output:  returns a unit conversion factor
**   Purpose: converts bulk reaction rates from per Liter to
**            per FT3 basis
**--------------------------------------------------------------
*/
{
    if (order < 0.0)  order = 0.0;
    if (order == 1.0) return (1.0);
    else return (1. / pow(LperFT3, (order - 1.0)));
}


void ratecoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: determines wall reaction coeff. for each pipe
**--------------------------------------------------------------
*/
{
    int k;
    double kw;

    EN_Network *net = &pr->network;
    quality_t  *qual = &pr->quality;

    for (k = 1; k <= net->Nlinks; k++)
    {
        kw = net->Link[k].Kw;
        if (kw != 0.0)  kw = piperate(pr, k);
        net->Link[k].Rc = kw;
        qual->PipeRateCoeff[k] = 0.0;
    }
}


void reactpipes(EN_Project *pr, long dt)
/*
**--------------------------------------------------------------
**   Input:   dt = time step
**   Output:  none
**   Purpose: reacts water within each pipe over a time step.
**--------------------------------------------------------------
*/
{
    int k;
    Pseg seg;
    double cseg, rsum, vsum;

    EN_Network *net = &pr->network;
    quality_t  *qual = &pr->quality;

    // Examine each link in network
    for (k = 1; k <= net->Nlinks; k++)
    {
        // Skip non-pipe links (pumps & valves)
        if (net->Link[k].Type != EN_PIPE) continue;
        rsum = 0.0;
        vsum = 0.0;

        // Examine each segment of the pipe
        seg = qual->FirstSeg[k];
        while (seg != NULL)
        {
            // React segment over time dt
            cseg = seg->c;
            seg->c = pipereact(pr, k, seg->c, seg->v, dt);

            // Update reaction component of mass balance
            qual->massbalance.reacted += (cseg - seg->c) * seg->v;

            // Accumulate volume-weighted reaction rate
            if (qual->Qualflag == CHEM)
            {
                rsum += fabs(seg->c - cseg) * seg->v;
                vsum += seg->v;
            }
            seg = seg->prev;
        }

        // Normalize volume-weighted reaction rate
        if (vsum > 0.0) qual->PipeRateCoeff[k] = rsum / vsum / dt * SECperDAY;
        else qual->PipeRateCoeff[k] = 0.0;
    }
}


void reacttanks(EN_Project *pr, long dt)
/*
**--------------------------------------------------------------
**   Input:   dt = time step
**   Output:  none
**   Purpose: reacts water within each tank over a time step.
**--------------------------------------------------------------
*/
{
    int i, k;
    double c;
    Pseg seg;

    EN_Network *net = &pr->network;
    quality_t  *qual = &pr->quality;
    Stank      *tank;

    // Examine each tank in network
    for (i = 1; i <= net->Ntanks; i++)
    {
        // Skip reservoirs
        tank = &net->Tank[i];
        if (tank->A == 0.0) continue;

        // k is segment chain belonging to tank i
        k = net->Nlinks + i;

        // React each volume segment in the chain
        seg = qual->FirstSeg[k];
        while (seg != NULL)
        {
            c = seg->c;
            seg->c = tankreact(pr, seg->c, seg->v, tank->Kb, dt);
            qual->massbalance.reacted += (c - seg->c) * seg->v;
            seg = seg->prev;
        }
    }
}


double piperate(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  returns reaction rate coeff. for 1st-order wall
**            reactions or mass transfer rate coeff. for 0-order
**            reactions
**   Purpose: finds wall reaction rate coeffs.
**--------------------------------------------------------------
*/
{
    double a, d, u, q, kf, kw, y, Re, Sh;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    d = net->Link[k].Diam;   // Pipe diameter, ft

    // Ignore mass transfer if Schmidt No. is 0 
    if (qual->Sc == 0.0)
    {
        if (qual->WallOrder == 0.0) return BIG;
        else return (net->Link[k].Kw * (4.0 / d) / pr->Ucf[ELEV]);
    }

    // Compute Reynolds No.
    // Flow rate made consistent with how its saved to hydraulics file
    q = (hyd->LinkStatus[k] <= CLOSED) ? 0.0 : hyd->LinkFlows[k];
    a = PI * d * d / 4.0;         // pipe area
    u = fabs(q) / a;              // flow velocity
    Re = u * d / hyd->Viscos;     // Reynolds number

    // Compute Sherwood No. for stagnant flow
    // (mass transfer coeff. = Diffus./radius)
    if (Re < 1.0) Sh = 2.0;

    // Compute Sherwood No. for turbulent flow using the Notter-Sleicher formula.
    else if (Re >= 2300.0) Sh = 0.0149 * pow(Re, 0.88) * pow(qual->Sc, 0.333);

    // Compute Sherwood No. for laminar flow using Graetz solution formula.
    else
    {
        y = d / net->Link[k].Len * Re * qual->Sc;
        Sh = 3.65 + 0.0668 * y / (1.0 + 0.04 * pow(y, 0.667));
    }

    // Compute mass transfer coeff. (in ft/sec)
    kf = Sh * qual->Diffus / d;

    // For zero-order reaction, return mass transfer coeff.
    if (qual->WallOrder == 0.0) return kf;

    // For first-order reaction, return apparent wall coeff.
    kw = net->Link[k].Kw / pr->Ucf[ELEV];        // Wall coeff, ft/sec
    kw = (4.0 / d) * kw * kf / (kf + fabs(kw));   // Wall coeff, 1/sec
    return kw;
}


double pipereact(EN_Project *pr, int k, double c, double v, long dt)
/*
**------------------------------------------------------------
**   Input:   k = link index
**            c = current quality in segment
**            v = segment volume
**            dt = time step
**   Output:  returns new WQ value
**   Purpose: computes new quality in a pipe segment after
**            reaction occurs
**------------------------------------------------------------
*/
{
    double cnew, dc, dcbulk, dcwall, rbulk, rwall;

    EN_Network  *net = &pr->network;
    quality_t   *qual = &pr->quality;

    // For water age (hrs), update concentration by timestep
    if (qual->Qualflag == AGE)
    {
        dc = (double)dt / 3600.0;
        cnew = c + dc;
        cnew = MAX(0.0, cnew);
        return cnew;
    }

    // Otherwise find bulk & wall reaction rates
    rbulk = bulkrate(pr, c, net->Link[k].Kb, qual->BulkOrder) * qual->Bucf;
    rwall = wallrate(pr, c, net->Link[k].Diam, net->Link[k].Kw, net->Link[k].Rc);

    // Find change in concentration over timestep
    dcbulk = rbulk * (double)dt;
    dcwall = rwall * (double)dt;

    // Update cumulative mass reacted
    if (pr->time_options.Htime >= pr->time_options.Rstart)
    {
        qual->Wbulk += fabs(dcbulk) * v;
        qual->Wwall += fabs(dcwall) * v;
    }

    // Update concentration
    dc = dcbulk + dcwall;
    cnew = c + dc;
    cnew = MAX(0.0, cnew);
    return cnew;
}


double tankreact(EN_Project *pr, double c, double v, double kb, long dt)
/*
**-------------------------------------------------------
**   Input:   c = current quality in tank
**            v = tank volume
**            kb = reaction coeff.
**            dt = time step
**   Output:  returns new WQ value
**   Purpose: computes new quality in a tank after
**            reaction occurs
**-------------------------------------------------------
*/
{
    double cnew, dc, rbulk;
    quality_t *qual = &pr->quality;

    // For water age, update concentration by timestep
    if (qual->Qualflag == AGE)
    {
        dc = (double)dt / 3600.0;
    }

    // For chemical analysis apply bulk reaction rate
    else
    {
        // Find bulk reaction rate
        rbulk = bulkrate(pr, c, kb, qual->TankOrder) * qual->Tucf;

        // Find concentration change & update quality
        dc = rbulk * (double)dt;
        if (pr->time_options.Htime >= pr->time_options.Rstart)
        {
            qual->Wtank += fabs(dc) * v;
        }
    }
    cnew = c + dc;
    cnew = MAX(0.0, cnew);
    return cnew;
}


double bulkrate(EN_Project *pr, double c, double kb, double order)
/*
**-----------------------------------------------------------
**   Input:   c = current WQ concentration
**            kb = bulk reaction coeff.
**            order = bulk reaction order
**   Output:  returns bulk reaction rate
**   Purpose: computes bulk reaction rate (mass/volume/time)
**-----------------------------------------------------------
*/
{
    double c1;
    quality_t *qual = &pr->quality;

    // Find bulk reaction potential taking into account
    // limiting potential & reaction order.

    // Zero-order kinetics:
    if (order == 0.0) c = 1.0;

    // Michaelis-Menton kinetics:
    else if (order < 0.0)
    {
        c1 = qual->Climit + SGN(kb) * c;
        if (fabs(c1) < TINY) c1 = SGN(c1) * TINY;
        c = c / c1;
    }

    // N-th order kinetics:
    else
    {
        // Account for limiting potential
        if (qual->Climit == 0.0) c1 = c;
        else c1 = MAX(0.0, SGN(kb) * (qual->Climit - c));

        // Compute concentration potential
        if (order == 1.0) c = c1;
        else if (order == 2.0) c = c1 * c;
        else c = c1 * pow(MAX(0.0, c), order - 1.0);
    }

    // Reaction rate = bulk coeff. * potential
    if (c < 0) c = 0;
    return kb * c;
}


double wallrate(EN_Project *pr, double c, double d, double kw, double kf)
/*
**------------------------------------------------------------
**   Input:   c = current WQ concentration
**            d = pipe diameter
**            kw = intrinsic wall reaction coeff.
**            kf = mass transfer coeff. for 0-order reaction
**                 (ft/sec) or apparent wall reaction coeff.
**                 for 1-st order reaction (1/sec)
**   Output:  returns wall reaction rate in mass/ft3/sec
**   Purpose: computes wall reaction rate
**------------------------------------------------------------
*/
{
    quality_t *qual = &pr->quality;
    if (kw == 0.0 || d == 0.0) return (0.0);

    if (qual->WallOrder == 0.0)             // 0-order reaction */
    {
        kf = SGN(kw) * c * kf;              //* Mass transfer rate (mass/ft2/sec)
        kw = kw * SQR(pr->Ucf[ELEV]);       // Reaction rate (mass/ft2/sec)
        if (fabs(kf) < fabs(kw)) kw = kf;   // Reaction mass transfer limited
        return (kw * 4.0 / d);              // Reaction rate (mass/ft3/sec)
    }
    else return (c * kf);                   // 1st-order reaction
}


double mixtank(EN_Project *pr, int n, double volin, double massin, double volout)
/*
**------------------------------------------------------------
**   Input:   n      = node index
**            volin  = inflow volume to tank over time step
**            massin = mass inflow to tank over time step
**            volout = outflow volume from tank over time step
**   Output:  returns new quality for tank
**   Purpose: mixes inflow with tank's contents to update its quality.
**------------------------------------------------------------
*/
{
    int i;
    double vnet;
    EN_Network   *net = &pr->network;
    quality_t    *qual = &pr->quality;

    i = n - net->Njuncs;
    vnet = volin - volout;
    switch (net->Tank[i].MixModel)
    {
        case MIX1: tankmix1(pr, i, volin, massin, vnet); break;
        case MIX2: tankmix2(pr, i, volin, massin, vnet); break;
        case FIFO: tankmix3(pr, i, volin, massin, vnet); break;
        case LIFO: tankmix4(pr, i, volin, massin, vnet); break;
    }
    return net->Tank[i].C;
}


void tankmix1(EN_Project *pr, int i, double vin, double win, double vnet)
/*
**---------------------------------------------
**   Input:   i = tank index
**            vin = inflow volume
**            win = mass inflow
**            vnet = inflow - outflow
**   Output:  none
**   Purpose: updates quality in a complete mix tank model
**---------------------------------------------
*/
{
    int k;
    double vnew;
    Pseg seg;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;
    Stank        *tank = &net->Tank[i];

    k = net->Nlinks + i;
    seg = qual->FirstSeg[k];
    if (seg)
    {
       vnew = seg->v + vin;
       if (vnew > 0.0) seg->c = (seg->c * seg->v + win) / vnew;
       seg->v += vnet;
       seg->v = MAX(0.0, seg->v);
       tank->C = seg->c;
    }
}


void tankmix2(EN_Project *pr, int i, double vin, double win, double vnet)
/*
**------------------------------------------------
**   Input:   i = tank index
**            vin = inflow volume
**            win = mass inflow
**            vnet = inflow - outflow
**   Output:  none
**   Purpose: updates quality in a 2-compartment tank model
**------------------------------------------------
*/
{
    int k;
    double vt,          // Transferred volume
           vmz;         // Full mixing zone volume
    Pseg   mixzone,     // Mixing zone segment
           stagzone;    // Stagnant zone segment

    EN_Network   *net = &pr->network;
    quality_t    *qual = &pr->quality;
    Stank        *tank = &pr->network.Tank[i];

    // Identify segments for each compartment
    k = net->Nlinks + i;
    mixzone = qual->LastSeg[k];
    stagzone = qual->FirstSeg[k];
    if (mixzone == NULL || stagzone == NULL) return;

    // Full mixing zone volume
    vmz = tank->V1max;

    // Tank is filling
    vt = 0.0;
    if (vnet > 0.0)
    {
        vt = MAX(0.0, (mixzone->v + vnet - vmz));
        if (vin > 0.0)
        {
            mixzone->c = ((stagzone->c) * (stagzone->v) + win) /
                         (mixzone->v + vin);
        }
        if (vt > 0.0)
        {
            stagzone->c = ((stagzone->c) * (stagzone->v) +
                           (mixzone->c) * vt) / (stagzone->v + vt);
        }
    }

    // Tank is emptying 
    else if (vnet < 0.0)
    {
        if (stagzone->v > 0.0) vt = MIN(stagzone->v, (-vnet));
        if (vin + vt > 0.0)
        {
            mixzone->c = ((mixzone->c) * (mixzone->v) + win +
                          (stagzone->c) * vt) / (mixzone->v + vin + vt);
        }
    }

    // Update segment volumes 
    if (vt > 0.0)
    {
        mixzone->v = vmz;
        if (vnet > 0.0) stagzone->v += vt;
        else            stagzone->v = MAX(0.0, ((stagzone->v) - vt));
    }
    else
    {
        mixzone->v += vnet;
        mixzone->v = MIN(mixzone->v, vmz);
        mixzone->v = MAX(0.0, mixzone->v);
        stagzone->v = 0.0;
    }

    // Use quality of mixing zone to represent quality of
    // tank since this is where outflow begins to flow from
    tank->C = mixzone->c;
}


void tankmix3(EN_Project *pr, int i, double vin, double win, double vnet)
/*
**----------------------------------------------------------
**   Input:   i = tank index
**            vin = inflow volume
**            win = mass inflow
**            vnet = inflow - outflow
**   Output:  none
**   Purpose: Updates quality in a First-In-First-Out (FIFO) tank model.
**----------------------------------------------------------
*/
{
    int k;
    double vout, vseg;
    double cin, vsum, wsum;
    Pseg seg;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;
    Stank        *tank = &pr->network.Tank[i];

    k = net->Nlinks + i;
    if (qual->LastSeg[k] == NULL || qual->FirstSeg[k] == NULL) return;

    // Add new last segment for flow entering the tank
    if (vin > 0.0)
    {
        // ... increase segment volume if inflow has same quality as segment
        cin = win / vin;
        seg = qual->LastSeg[k];
        if (fabs(seg->c - cin) < qual->Ctol) seg->v += vin;

        // ... otherwise add a new last segment to the tank
        else addseg(pr, k, vin, cin);
    }

    // Withdraw flow from first segment
    vsum = 0.0;
    wsum = 0.0;
    vout = vin - vnet;
    while (vout > 0.0)
    {
        seg = qual->FirstSeg[k];
        if (seg == NULL)  break;
        vseg = seg->v;            // Flow volume from leading seg
        vseg = MIN(vseg, vout);
        if (seg == qual->LastSeg[k]) vseg = vout;
        vsum += vseg;
        wsum += (seg->c) * vseg;
        vout -= vseg;                       // Remaining flow volume
        if (vout >= 0.0 && vseg >= seg->v)  // Seg used up
        {
            if (seg->prev)
            {
                qual->FirstSeg[k] = seg->prev;
                seg->prev = qual->FreeSeg;
                qual->FreeSeg = seg;
            }
        }
        else seg->v -= vseg;      // Remaining volume in segment
    }

    // Use quality withdrawn from 1st segment
    // to represent overall quality of tank 
    if      (vsum > 0.0)                tank->C = wsum / vsum;
    else if (qual->FirstSeg[k] == NULL) tank->C = 0.0;
    else                                tank->C = qual->FirstSeg[k]->c;
}


void tankmix4(EN_Project *pr, int i, double vin, double win, double vnet)
/*
**----------------------------------------------------------
**   Input:   i = tank index
**            vin = inflow volume
**            win = mass inflow
**            vnet = inflow - outflow
**   Output:  none
**   Purpose: Updates quality in a Last In-First Out (LIFO) tank model.
**----------------------------------------------------------
*/
{
    int k, n;
    double cin, vsum, wsum, vseg;
    Pseg seg;

    EN_Network   *net  = &pr->network;
    quality_t    *qual = &pr->quality;
    Stank        *tank = &pr->network.Tank[i];

    k = net->Nlinks + i;
    if (qual->LastSeg[k] == NULL || qual->FirstSeg[k] == NULL) return;

    // Find inflows & outflows
    n = tank->Node;
    if (vin > 0.0) cin = win / vin;
    else           cin = 0.0;

    // If tank filling, then create new last seg
    tank->C = qual->LastSeg[k]->c;
    seg = qual->LastSeg[k];
    if (vnet > 0.0)
    {
        // ... quality is the same, so just add flow volume to last seg
        if (fabs(seg->c - cin) < qual->Ctol) seg->v += vnet;

        // ... otherwise add a new last seg to tank which points to old last seg
        else
        {
            qual->LastSeg[k] = NULL;
            addseg(pr, k, vnet, cin);
            qual->LastSeg[k]->prev = seg;
        }

        // ... update reported tank quality 
        tank->C = qual->LastSeg[k]->c;
    }

    // If tank emptying then remove last segments until vnet consumed
    else if (vnet < 0.0)
    {
        vsum = 0.0;
        wsum = 0.0;
        vnet = -vnet;
        while (vnet > 0.0)
        {
            seg = qual->LastSeg[k];
            if (seg == NULL) break;
            vseg = seg->v;
            vseg = MIN(vseg, vnet);
            if (seg == qual->FirstSeg[k]) vseg = vnet;
            vsum += vseg;
            wsum += (seg->c) * vseg;
            vnet -= vseg;
            if (vnet >= 0.0 && vseg >= seg->v)   // Seg used up
            {
                if (seg->prev)
                {
                    qual->LastSeg[k] = seg->prev;
                    seg->prev = qual->FreeSeg;
                    qual->FreeSeg = seg;
                }
            }
            else seg->v -= vseg;       // Remaining volume in segment
        }

        // Reported tank quality is mixture of flow released and any inflow
        tank->C = (wsum + win) / (vsum + vin);
    }
}
