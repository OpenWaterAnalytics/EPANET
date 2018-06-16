/*
*********************************************************************

HYDCOEFFS.C --  hydraulic coefficients for the EPANET Program

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
#include "types.h"
#include "funcs.h"

// Constants used for computing Darcy-Weisbach friction factor
const double A1 = 0.314159265359e04;   // 1000*PI
const double A2 = 0.157079632679e04;   // 500*PI
const double A3 = 0.502654824574e02;   // 16*PI
const double A4 = 6.283185307;         // 2*PI
const double A8 = 4.61841319859;       // 5.74*(PI/4)^.9
const double A9 = -8.685889638e-01;    // -2/ln(10)
const double AA = -1.5634601348;       // -2*.9*2/ln(10)
const double AB = 3.28895476345e-03;   // 5.74/(4000^.9)
const double AC = -5.14214965799e-03;  // AA*AB

// External functions
//void  resistcoeff(EN_Project *pr, int k);
//void  hlosscoeff(EN_Project *pr, int k);
//void  matrixcoeffs(EN_Project *pr);

// Local functions
static void    linkcoeffs(EN_Project *pr);
static void    nodecoeffs(EN_Project *pr);
static void    valvecoeffs(EN_Project *pr);
static void    emittercoeffs(EN_Project *pr);

static void    pipecoeff(EN_Project *pr, int k);
static void    DWpipecoeff(EN_Project *pr, int k);
static double  frictionFactor(double q, double e, double s, double *dfdq);

static void    pumpcoeff(EN_Project *pr, int k);
static void    curvecoeff(EN_Project *pr, int i, double q, double *h0, double *r);

static void    valvecoeff(EN_Project *pr, int k);
static void    gpvcoeff(EN_Project *pr, int k);
static void    pbvcoeff(EN_Project *pr, int k);
static void    tcvcoeff(EN_Project *pr, int k);
static void    prvcoeff(EN_Project *pr, int k, int n1, int n2);
static void    psvcoeff(EN_Project *pr, int k, int n1, int n2);
static void    fcvcoeff(EN_Project *pr, int k, int n1, int n2);



void  resistcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------------
**  Input:   k = link index
**  Output:  none
**  Purpose: computes link flow resistance coefficient
**--------------------------------------------------------------------
*/
{
    double e, d, L;
    EN_Network *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    Slink *link = &net->Link[k];
    
    switch (link->Type) {

    // ... Link is a pipe. Compute resistance based on headloss formula.
    //     Friction factor for D-W formula gets included during head loss 
    //     calculation.
    case EN_CVPIPE:
    case EN_PIPE:
        e = link->Kc;                 // Roughness coeff.
        d = link->Diam;               // Diameter
        L = link->Len;                // Length
        switch (hyd->Formflag)
        {
        case HW:
            link->R = 4.727*L / pow(e, hyd->Hexp) / pow(d, 4.871);
            break;
        case DW:
            link->R = L / 2.0 / 32.2 / d / SQR(PI*SQR(d) / 4.0);
            break;
        case CM:
            link->R = SQR(4.0*e / (1.49*PI*d*d)) * pow((d / 4.0), -1.333)*L;
        }
        break;

    // ... Link is a pump. Use huge resistance.
    case EN_PUMP:
        link->R = CBIG;
        break;

    // ... For all other links (e.g. valves) use a small resistance
    default:
        link->R = CSMALL;
        break;
    }
}


void  hlosscoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P and Y coefficients for a link
**--------------------------------------------------------------
*/
{
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink        *link = &net->Link[k];

    switch (link->Type)
    {
    case EN_CVPIPE:
    case EN_PIPE:
        pipecoeff(pr, k);
        break;
    case EN_PUMP:
        pumpcoeff(pr, k);
        break;
    case EN_PBV:
        pbvcoeff(pr, k);
        break;
    case EN_TCV:
        tcvcoeff(pr, k);
        break;
    case EN_GPV:
        gpvcoeff(pr, k);
        break;
    case EN_FCV:
    case EN_PRV:
    case EN_PSV:
        if (hyd->LinkSetting[k] == MISSING) {
            valvecoeff(pr, k);
        }
        else sol->P[k] = 0.0;
    }
}


void   matrixcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: computes coefficients of linearized network eqns.
**--------------------------------------------------------------
*/
{
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;

    memset(sol->Aii, 0, (net->Nnodes + 1) * sizeof(double));
    memset(sol->Aij, 0, (hyd->Ncoeffs + 1) * sizeof(double));
    memset(sol->F, 0, (net->Nnodes + 1) * sizeof(double));
    memset(hyd->X_tmp, 0, (net->Nnodes + 1) * sizeof(double));
    linkcoeffs(pr);
    emittercoeffs(pr);
    nodecoeffs(pr);
    valvecoeffs(pr);
}


void  linkcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes matrix coefficients for links
**--------------------------------------------------------------
*/
{
    int   k, n1, n2;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link;

    // Examine each link of network */
    for (k = 1; k <= net->Nlinks; k++)
    {
        if (sol->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;           // Start node of link
        n2 = link->N2;           // End node of link


        // Update net nodal inflows (X), solution matrix (A) and RHS array (F)
        // (Use covention that flow out of node is (-), flow into node is (+))
        hyd->X_tmp[n1] -= hyd->LinkFlows[k];
        hyd->X_tmp[n2] += hyd->LinkFlows[k];

        // Off-diagonal coeff.
        sol->Aij[sol->Ndx[k]] -= sol->P[k];

        // Node n1 is junction
        if (n1 <= net->Njuncs)                     
        {
            sol->Aii[sol->Row[n1]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n1]] += sol->Y[k];     // RHS coeff.
        }

        // Node n1 is a tank
        else {
            sol->F[sol->Row[n2]] += (sol->P[k] * hyd->NodeHead[n1]); 
        }

        // Node n2 is junction
        if (n2 <= net->Njuncs) {
            sol->Aii[sol->Row[n2]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n2]] -= sol->Y[k];     // RHS coeff.
        }

        // Node n2 is a tank
        else {
            sol->F[sol->Row[n1]] += (sol->P[k] * hyd->NodeHead[n2]); 
        }
    }
}


void  nodecoeffs(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: completes calculation of nodal flow imbalance (X)
**           & flow correction (F) arrays
**----------------------------------------------------------------
*/
{
    int   i;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;

    // For junction nodes, subtract demand flow from net
    // flow imbalance & add imbalance to RHS array F.
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->X_tmp[i] -= hyd->NodeDemand[i];
        sol->F[sol->Row[i]] += hyd->X_tmp[i];
    }
}


void  valvecoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes matrix coeffs. for PRVs, PSVs & FCVs
**            whose status is not fixed to OPEN/CLOSED
**--------------------------------------------------------------
*/
{
    int i, k, n1, n2;

    hydraulics_t *hyd = &pr->hydraulics;
    EN_Network   *net = &pr->network;
    Slink *link;
    Svalve *valve;

    // Examine each valve
    for (i = 1; i <= net->Nvalves; i++)
    {
        valve = &net->Valve[i];
        k = valve->Link;                         // Link index of valve
        link = &net->Link[k];
        if (hyd->LinkSetting[k] == MISSING) {
            continue;                            // Valve status fixed
        }
        n1 = link->N1;                           // Start & end nodes
        n2 = link->N2;
        switch (link->Type)                      // Call valve-specific function 
        {
        case EN_PRV:
            prvcoeff(pr, k, n1, n2);
            break;
        case EN_PSV:
            psvcoeff(pr, k, n1, n2);
            break;
        case EN_FCV:
            fcvcoeff(pr, k, n1, n2);
            break;
        default:   continue;
        }
    }
}


void  emittercoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes matrix coeffs. for emitters
**
**   Note: Emitters consist of a fictitious pipe connected to
**         a fictitious reservoir whose elevation equals that
**         of the junction. The headloss through this pipe is
**         Ke*(Flow)^hyd->Qexp, where Ke = emitter headloss coeff.
**--------------------------------------------------------------
*/
{
    int   i;
    double  ke;
    double  p;
    double  q;
    double  y;
    double  z;

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;
    Snode *node;

    for (i = 1; i <= net->Njuncs; i++)
    {
        node = &net->Node[i];
        if (node->Ke == 0.0) {
            continue;
        }
        ke = MAX(CSMALL, node->Ke);              // emitter coeff.
        q = hyd->EmitterFlows[i];                // emitter flow
        z = ke * pow(ABS(q), hyd->Qexp);         // emitter head loss
        p = hyd->Qexp * z / ABS(q);              // head loss gradient
        if (p < hyd->RQtol) {
            p = 1.0 / hyd->RQtol;
        }
        else {
            p = 1.0 / p;                         // inverse head loss gradient
        }
        y = SGN(q)*z*p;                          // head loss / gradient
        sol->Aii[sol->Row[i]] += p;              // addition to main diagonal
        sol->F[sol->Row[i]] += y + p * node->El; // addition to r.h.s.
        hyd->X_tmp[i] -= q;                      // addition to net node inflow
    }
}


void  pipecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**  Purpose:  computes P & Y coefficients for pipe k
**
**    P = inverse head loss gradient = 1/(dh/dQ)
**    Y = flow correction term = h*P
**--------------------------------------------------------------
*/
{
    double  hpipe,     // Normal head loss
            hml,       // Minor head loss
            ml,        // Minor loss coeff.
            p,         // q*(dh/dq)
            q,         // Abs. value of flow
            r;         // Resistance coeff.

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link = &pr->network.Link[k];

    // For closed pipe use headloss formula: h = CBIG*q
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        sol->P[k] = 1.0 / CBIG;
        sol->Y[k] = hyd->LinkFlows[k];
        return;
    }

    // ... head loss formula is Darcy-Weisbach
    if (hyd->Formflag == DW) {
        DWpipecoeff(pr, k);
        return;
    }

    // Evaluate headloss coefficients
    q = ABS(hyd->LinkFlows[k]);                  // Absolute flow
    ml = link->Km;                               // Minor loss coeff.
    r = link->R;                                 // Resistance coeff.

    // Use large P coefficient for small flow resistance product
    if ( (r+ml)*q < hyd->RQtol)
    {
        sol->P[k] = 1.0 / hyd->RQtol;
        sol->Y[k] = hyd->LinkFlows[k] / hyd->Hexp;
        return;
    }

    // Compute P and Y coefficients
    hpipe = r*pow(q, hyd->Hexp);             // Friction head loss
    p = hyd->Hexp*hpipe;                     // Q*dh(friction)/dQ 
    if (ml > 0.0)
    {
        hml = ml*q*q;                        // Minor head loss
        p += 2.0*hml;                        // Q*dh(Total)/dQ
    }
    else  hml = 0.0;
    p = hyd->LinkFlows[k] / p;               // 1 / (dh/dQ)
    sol->P[k] = ABS(p);
    sol->Y[k] = p*(hpipe + hml);
}


void DWpipecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes pipe head loss coeffs. for Darcy-Weisbach
**            formula.
**--------------------------------------------------------------
*/
{
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link = &pr->network.Link[k];

    double s = hyd->Viscos * link->Diam;
    double q = ABS(hyd->LinkFlows[k]);
    double dfdq = 0.0;
    double e;      // relative roughness height
    double r;      // total resistance coeff.
    double f;      // friction factor
    double hloss;  // head loss
    double hgrad;  // head loss gradient

    // ... use Hagen-Poiseuille formula for laminar flow (Re <= 2000)
    if (q <= A2 * s)
    {
        r = 16.0 * PI * s * link->R;
        hloss = q * (r + link->Km * q);
        hgrad = r + 2.0 * link->Km * q;
    }

    // ... use Colebrook formula for turbulent flow
    else
    {
        e = link->Kc / link->Diam;
        f = frictionFactor(q, e, s, &dfdq);
        r = f * link->R + link->Km;
        hloss = r * q * hyd->LinkFlows[k];
        hgrad = (2.0 * r * q) + (dfdq * link->R * q * q);
    }

    // ... head loss has same sign as flow
    hloss *= SGN(hyd->LinkFlows[k]);

    // ... compute P and Y coeffs.
    sol->P[k] = 1.0 / hgrad;
    sol->Y[k] = hloss / hgrad;
}


double frictionFactor(double q, double e, double s, double *dfdq)
/*
**--------------------------------------------------------------
**   Input:   q = flow rate (cfs)
**            e = pipe roughness / diameter
**            s = viscosity * diameter
**   Output:  f = friction factor
**            dfdq = derivative of f w.r.t. flow
**   Purpose: computes Darcy-Weisbach friction factor and its
**            derivative as a function of Reynolds Number (Re).
**--------------------------------------------------------------
*/
{
    double f;
    double x1, x2, x3, x4,
        y1, y2, y3,
        fa, fb, r;
    double w = q / s;        // Re*Pi/4

                             // ... For Re >= 4000 use Colebrook Formula
    if (w >= A1)
    {
        y1 = A8 / pow(w, 0.9);
        y2 = e / 3.7 + y1;
        y3 = A9 * log(y2);
        f = 1.0 / (y3*y3);
        *dfdq = 1.8 * f * y1 * A9 / y2 / y3 / q;
    }

    // ... Use interpolating polynomials developed by
    //     E. Dunlop for transition flow from 2000 < Re < 4000.
    else
    {
        y2 = e / 3.7 + AB;
        y3 = A9 * log(y2);
        fa = 1.0 / (y3*y3);
        fb = (2.0 + AC / (y2*y3)) * fa;
        r = w / A2;
        x1 = 7.0 * fa - fb;
        x2 = 0.128 - 17.0 * fa + 2.5 * fb;
        x3 = -0.128 + 13.0 * fa - (fb + fb);
        x4 = r * (0.032 - 3.0 * fa + 0.5 *fb);
        f = x1 + r * (x2 + r * (x3 + x4));
        *dfdq = (x2 + 2.0 * r * (x3 + x4)) / s / A2;
    }
    return f;
}


void  pumpcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for pump in link k
**--------------------------------------------------------------
*/
{
    int    p;                // Pump index
    double h0,               // Shutoff head
           q,                // Abs. value of flow 
           r,                // Flow resistance coeff.
           n;                // Flow exponent coeff.
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    double setting = hyd->LinkSetting[k];
    Spump *pump;

    // Use high resistance pipe if pump closed or cannot deliver head
    if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0) {
        sol->P[k] = 1.0 / CBIG;
        sol->Y[k] = hyd->LinkFlows[k];
        return;
    }

    q = ABS(hyd->LinkFlows[k]);
    q = MAX(q, TINY);

    // Obtain reference to pump object
    p = findpump(&pr->network, k);
    pump = &pr->network.Pump[p];

    // Get pump curve coefficients for custom pump curve.
    if (pump->Ptype == CUSTOM)
    {
        // Find intercept (h0) & slope (r) of pump curve
        // line segment which contains speed-adjusted flow.
        curvecoeff(pr, pump->Hcurve, q / setting, &h0, &r);

        // Determine head loss coefficients.
        pump->H0 = -h0;
        pump->R = -r;
        pump->N = 1.0;
    }

    // Adjust head loss coefficients for pump speed.
    h0 = SQR(setting) * pump->H0;
    n = pump->N;
    r = pump->R * pow(setting, 2.0 - n);
    if (n != 1.0) {
        r = n * r * pow(q, n - 1.0);
    }

    // Compute inverse headloss gradient (P) and flow correction factor (Y)
    sol->P[k] = 1.0 / MAX(r, hyd->RQtol);
    sol->Y[k] = hyd->LinkFlows[k] / n + sol->P[k] * h0;
}


void  curvecoeff(EN_Project *pr, int i, double q, double *h0, double *r)
/*
**-------------------------------------------------------------------
**   Input:   i   = curve index
**            q   = flow rate
**   Output:  *h0  = head at zero flow (y-intercept)
**            *r  = dHead/dFlow (slope)
**   Purpose: computes intercept and slope of head v. flow curve
**            at current flow.
**-------------------------------------------------------------------
*/
{
    int   k1, k2, npts;
    double *x, *y;
    Scurve *curve;

    // Remember that curve is stored in untransformed units
    q *= pr->Ucf[FLOW];
    curve = &pr->network.Curve[i];
    x = curve->X;                      // x = flow
    y = curve->Y;                      // y = head
    npts = curve->Npts;

    // Find linear segment of curve that brackets flow q
    k2 = 0;
    while (k2 < npts && x[k2] < q)
        k2++;

    if (k2 == 0)
        k2++;

    else if (k2 == npts)
        k2--;

    k1 = k2 - 1;

    // Compute slope and intercept of this segment
    *r = (y[k2] - y[k1]) / (x[k2] - x[k1]);
    *h0 = y[k1] - (*r)*x[k1];

    // Convert units
    *h0 = (*h0) / pr->Ucf[HEAD];
    *r = (*r) * pr->Ucf[FLOW] / pr->Ucf[HEAD];
}


void  gpvcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for general purpose valve
**--------------------------------------------------------------
*/
{
    double h0,        // Headloss curve intercept
           q,         // Abs. value of flow
           r;         // Flow resistance coeff.

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    // Treat as a pipe if valve closed
    if (hyd->LinkStatus[k] == CLOSED) {
        valvecoeff(pr, k);                          
    }

    // Otherwise utilize headloss curve
    // whose index is stored in K
    else {
        // Find slope & intercept of headloss curve.
        q = ABS(hyd->LinkFlows[k]);
        q = MAX(q, TINY);
        curvecoeff(pr, (int)ROUND(hyd->LinkSetting[k]), q, &h0, &r);

        // Compute inverse headloss gradient (P)
        // and flow correction factor (Y).
        sol->P[k] = 1.0 / MAX(r, hyd->RQtol);
        sol->Y[k] = sol->P[k] * (h0 + r*q) * SGN(hyd->LinkFlows[k]);
    }
}


void  pbvcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for pressure breaker valve
**--------------------------------------------------------------
*/
{
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link = &pr->network.Link[k];

    // If valve fixed OPEN or CLOSED then treat as a pipe
    if (hyd->LinkSetting[k] == MISSING || hyd->LinkSetting[k] == 0.0) {
        valvecoeff(pr, k);         
    }

    // If valve is active
    else {
        // Treat as a pipe if minor loss > valve setting
        if (link->Km * SQR(hyd->LinkFlows[k]) > hyd->LinkSetting[k]) {
            valvecoeff(pr, k);        
        }
        // Otherwise force headloss across valve to be equal to setting
        else {
            sol->P[k] = CBIG;
            sol->Y[k] = hyd->LinkSetting[k] * CBIG;
        }
    }
}


void  tcvcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for throttle control valve
**--------------------------------------------------------------
*/
{
    double km;
    hydraulics_t *hyd = &pr->hydraulics;
    Slink *link = &pr->network.Link[k];

    // Save original loss coeff. for open valve
    km = link->Km;

    // If valve not fixed OPEN or CLOSED, compute its loss coeff.
    if (hyd->LinkSetting[k] != MISSING) {
        link->Km = 0.02517 * hyd->LinkSetting[k] / (SQR(link->Diam)*SQR(link->Diam));
    }

    // Then apply usual valve formula
    valvecoeff(pr, k);                                             

    // Restore original loss coeff.
    link->Km = km;
}


void  prvcoeff(EN_Project *pr, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index
**            n1   = upstream node of valve
**            n2   = downstream node of valve
**   Output:  none
**   Purpose: computes solution matrix coeffs. for pressure
**            reducing valves
**--------------------------------------------------------------
*/
{
    int   i, j;                        // Rows of solution matrix
    double hset;                       // Valve head setting
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    i = sol->Row[n1];                  // Matrix rows of nodes
    j = sol->Row[n2];
    hset = pr->network.Node[n2].El +
           hyd->LinkSetting[k];        // Valve setting

    if (hyd->LinkStatus[k] == ACTIVE)
    {

        // Set coeffs. to force head at downstream
        // node equal to valve setting & force flow 
        // to equal to flow imbalance at downstream node.

        sol->P[k] = 0.0;
        sol->Y[k] = hyd->LinkFlows[k] + hyd->X_tmp[n2];    // Force flow balance
        sol->F[j] += (hset * CBIG);                        // Force head = hset
        sol->Aii[j] += CBIG;                               // at downstream node
        if (hyd->X_tmp[n2] < 0.0) {
            sol->F[i] += hyd->X_tmp[n2];
        }
        return;
    }

    // For OPEN, CLOSED, or XPRESSURE valve
    // compute matrix coeffs. using the valvecoeff() function.

    valvecoeff(pr, k);
    sol->Aij[sol->Ndx[k]] -= sol->P[k];
    sol->Aii[i] += sol->P[k];
    sol->Aii[j] += sol->P[k];
    sol->F[i] += (sol->Y[k] - hyd->LinkFlows[k]);
    sol->F[j] -= (sol->Y[k] - hyd->LinkFlows[k]);
}


void  psvcoeff(EN_Project *pr, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index
**            n1   = upstream node of valve
**            n2   = downstream node of valve
**   Output:  none
**   Purpose: computes solution matrix coeffs. for pressure
**            sustaining valve
**--------------------------------------------------------------
*/
{
    int   i, j;                        // Rows of solution matrix
    double hset;                       // Valve head setting
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    i = sol->Row[n1];                  // Matrix rows of nodes
    j = sol->Row[n2];
    hset = pr->network.Node[n1].El +
           hyd->LinkSetting[k];        // Valve setting

    if (hyd->LinkStatus[k] == ACTIVE)
    {
        // Set coeffs. to force head at upstream
        // node equal to valve setting & force flow 
        // equal to flow imbalance at upstream node.

        sol->P[k] = 0.0;
        sol->Y[k] = hyd->LinkFlows[k] - hyd->X_tmp[n1];    // Force flow balance
        sol->F[i] += (hset * CBIG);                        // Force head = hset
        sol->Aii[i] += CBIG;                               // at upstream node
        if (hyd->X_tmp[n1] > 0.0) {
            sol->F[j] += hyd->X_tmp[n1];
        }
        return;
    }

    // For OPEN, CLOSED, or XPRESSURE valve
    // compute matrix coeffs. using the valvecoeff() function.

    valvecoeff(pr, k);
    sol->Aij[sol->Ndx[k]] -= sol->P[k];
    sol->Aii[i] += sol->P[k];
    sol->Aii[j] += sol->P[k];
    sol->F[i] += (sol->Y[k] - hyd->LinkFlows[k]);
    sol->F[j] -= (sol->Y[k] - hyd->LinkFlows[k]);
}


void  fcvcoeff(EN_Project *pr, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index
**            n1   = upstream node of valve
**            n2   = downstream node of valve
**   Output:  none
**   Purpose: computes solution matrix coeffs. for flow control
**            valve
**--------------------------------------------------------------
*/
{
    int   i, j;                   // Rows in solution matrix
    double q;                     // Valve flow setting
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    q = hyd->LinkSetting[k];
    i = hyd->solver.Row[n1];
    j = hyd->solver.Row[n2];

    // If valve active, break network at valve and treat
    // flow setting as external demand at upstream node
    // and external supply at downstream node.

    if (hyd->LinkStatus[k] == ACTIVE)
    {
        hyd->X_tmp[n1] -= q;
        sol->F[i] -= q;
        hyd->X_tmp[n2] += q;
        sol->F[j] += q;
        sol->P[k] = 1.0 / CBIG;
        sol->Aij[sol->Ndx[k]] -= sol->P[k];
        sol->Aii[i] += sol->P[k];
        sol->Aii[j] += sol->P[k];
        sol->Y[k] = hyd->LinkFlows[k] - q;
    }

    // Otherwise treat valve as an open pipe

    else
    {
        valvecoeff(pr, k);                                          
        sol->Aij[sol->Ndx[k]] -= sol->P[k];
        sol->Aii[i] += sol->P[k];
        sol->Aii[j] += sol->P[k];
        sol->F[i] += (sol->Y[k] - hyd->LinkFlows[k]);
        sol->F[j] -= (sol->Y[k] - hyd->LinkFlows[k]);
    }
}


void valvecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k    = link index
**   Output:  none
**   Purpose: computes solution matrix coeffs. for a completely
**            open, closed, or throttled control valve.
**--------------------------------------------------------------
*/
{
    double p;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link = &net->Link[k];

    double flow = hyd->LinkFlows[k];

    // Valve is closed. Use a very small matrix coeff.
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        sol->P[k] = 1.0 / CBIG;
        sol->Y[k] = flow;
        return;
    }

    // Account for any minor headloss through the valve
    if (link->Km > 0.0)
    {
        p = 2.0 * link->Km * fabs(flow);
        if (p < hyd->RQtol) {
            p = hyd->RQtol;
        }
        sol->P[k] = 1.0 / p;
        sol->Y[k] = flow / 2.0;
    }
    else
    {
        sol->P[k] = 1.0 / hyd->RQtol;
        sol->Y[k] = flow;
    }
}
