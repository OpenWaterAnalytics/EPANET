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
const double A1 = 3.14159265358979323850e+03;   // 1000*PI
const double A2 = 1.57079632679489661930e+03;   // 500*PI
const double A3 = 5.02654824574366918160e+01;   // 16*PI
const double A4 = 6.28318530717958647700e+00;   // 2*PI
const double A8 = 4.61841319859066668690e+00;   // 5.74*(PI/4)^.9
const double A9 = -8.68588963806503655300e-01;  // -2/ln(10)
const double AA = -1.5634601348517065795e+00;   // -2*.9*2/ln(10)
const double AB = 3.28895476345399058690e-03;   // 5.74/(4000^.9)
const double AC = -5.14214965799093883760e-03;  // AA*AB

// External functions
//void   resistcoeff(EN_Project *pr, int k);
//void   headlosscoeffs(EN_Project *pr);
//void   matrixcoeffs(EN_Project *pr);
//void   emitheadloss(EN_Project *pr, int i, double *hloss, double *dhdq);
//double demandflowchange(EN_Project *pr, int i, double dp, double n);
//void   demandparams(EN_Project *pr, double *dp, double *n);

// Local functions
static void    linkcoeffs(EN_Project *pr);
static void    nodecoeffs(EN_Project *pr);
static void    valvecoeffs(EN_Project *pr);
static void    emittercoeffs(EN_Project *pr);
static void    demandcoeffs(EN_Project *pr);
static void    demandheadloss(double d, double dfull, double dp,
               double n, double *hloss, double *hgrad);

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

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    Slink *link = &net->Link[k];
    
    link->Qa = 0.0;
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
            // ... resistance coeff.
            link->R = 4.727 * L / pow(e, hyd->Hexp) / pow(d, 4.871);
            // ... flow below which linear head loss applies
            link->Qa = pow(hyd->RQtol / hyd->Hexp / link->R, 1.17371);
            break;
        case DW:
            link->R = L / 2.0 / 32.2 / d / SQR(PI * SQR(d) / 4.0);
            break;
        case CM:
            // ... resistance coeff.
            link->R = SQR(4.0 * e / (1.49 * PI * SQR(d))) *
                      pow((d / 4.0), -1.333) * L;
            // ... flow below which linear head loss applies
            link->Qa = hyd->RQtol / 2.0 / link->R;
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


void headlosscoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coefficients P (1 / head loss gradient)
**            and Y (head loss / gradient) for all links.
**--------------------------------------------------------------
*/
{
    int k;
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;

    for (k = 1; k <= net->Nlinks; k++)
    {
        switch (net->Link[k].Type)
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
            if (hyd->LinkSetting[k] == MISSING) valvecoeff(pr, k);
            else hyd->solver.P[k] = 0.0;
        }
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

    // Reset values of all diagonal coeffs. (Aii), off-diagonal
    // coeffs. (Aij), r.h.s. coeffs. (F) and node flow balance (X_tmp)
    memset(sol->Aii, 0, (net->Nnodes + 1) * sizeof(double));
    memset(sol->Aij, 0, (hyd->Ncoeffs + 1) * sizeof(double));
    memset(sol->F, 0, (net->Nnodes + 1) * sizeof(double));
    memset(hyd->X_tmp, 0, (net->Nnodes + 1) * sizeof(double));

    // Compute matrix coeffs. from links, emitters, and nodal demands
    linkcoeffs(pr);
    emittercoeffs(pr);
    demandcoeffs(pr);

    // Update nodal flow balances with demands and add onto r.h.s. coeffs.
    nodecoeffs(pr);

    // Finally, find coeffs. for PRV/PSV/FCV control valves whose
    // status is not fixed to OPEN/CLOSED
    valvecoeffs(pr);
}


void  linkcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coefficients contributed by links to the
**            linearized system of hydraulic equations.
**--------------------------------------------------------------
*/
{
    int   k, n1, n2;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link;

    // Examine each link of network
    for (k = 1; k <= net->Nlinks; k++)
    {
        if (sol->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;           // Start node of link
        n2 = link->N2;           // End node of link

        // Update nodal flow balance (X_tmp)
        // (Flow out of node is (-), flow into node is (+))
        hyd->X_tmp[n1] -= hyd->LinkFlows[k];
        hyd->X_tmp[n2] += hyd->LinkFlows[k];

        // Add to off-diagonal coeff. of linear system matrix
        sol->Aij[sol->Ndx[k]] -= sol->P[k];

        // Update linear system coeffs. associated with start node n1
        // ... node n1 is junction
        if (n1 <= net->Njuncs)                     
        {
            sol->Aii[sol->Row[n1]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n1]] += sol->Y[k];     // RHS coeff.
        }

        // ... node n1 is a tank/reservoir
        else sol->F[sol->Row[n2]] += (sol->P[k] * hyd->NodeHead[n1]); 

        // Update linear system coeffs. associated with end node n2
        // ... node n2 is junction
        if (n2 <= net->Njuncs)
        {
            sol->Aii[sol->Row[n2]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n2]] -= sol->Y[k];     // RHS coeff.
        }

        // ... node n2 is a tank/reservoir
        else sol->F[sol->Row[n1]] += (sol->P[k] * hyd->NodeHead[n2]); 
    }
}


void  nodecoeffs(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: completes calculation of nodal flow balance array
**           (X_tmp) & r.h.s. (F) of linearized hydraulic eqns.
**----------------------------------------------------------------
*/
{
    int   i;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;

    // For junction nodes, subtract demand flow from net
    // flow balance & add flow balance to RHS array F
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->X_tmp[i] -= hyd->DemandFlows[i];
        sol->F[sol->Row[i]] += hyd->X_tmp[i];
    }
}


void  valvecoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coeffs. of the linearized hydraulic eqns.
**            contributed by PRVs, PSVs & FCVs whose status is
**            not fixed to OPEN/CLOSED
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
        // Find valve's link index
        valve = &net->Valve[i];
        k = valve->Link;

        // Coeffs. for fixed status valves have already been computed
        if (hyd->LinkSetting[k] == MISSING) continue;

        // Start & end nodes of valve's link        
        link = &net->Link[k];
        n1 = link->N1;
        n2 = link->N2;

        // Call valve-specific function
        switch (link->Type) 
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
**   Purpose: computes coeffs. of the linearized hydraulic eqns.
**            contributed by emitters.
**
**   Note: Emitters consist of a fictitious pipe connected to
**         a fictitious reservoir whose elevation equals that
**         of the junction. The headloss through this pipe is
**         Ke*(Flow)^hyd->Qexp, where Ke = emitter headloss coeff.
**--------------------------------------------------------------
*/
{
    int     i, row;
    double  hloss, hgrad;

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;
    Snode *node;

    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions without emitters
        node = &net->Node[i];
        if (node->Ke == 0.0) continue;

        // Find emitter head loss and gradient
        emitheadloss(pr, i, &hloss, &hgrad);

        // Row of solution matrix
        row = sol->Row[i];

        // Addition to matrix diagonal & r.h.s
        sol->Aii[row] += 1.0 / hgrad;
        sol->F[row] += (hloss + node->El) / hgrad;

        // Update to node flow balance
        hyd->X_tmp[i] -= hyd->EmitterFlows[i];
    }
}


void emitheadloss(EN_Project *pr, int i, double *hloss, double *hgrad)
/*
**-------------------------------------------------------------
**   Input:   i = node index
**   Output:  hloss = head loss across node's emitter
**            hgrad = head loss gradient
**   Purpose: computes an emitters's head loss and gradient.
**-------------------------------------------------------------
*/
{
    double  ke;
    double  q;
    double  qa;
    hydraulics_t *hyd = &pr->hydraulics;

    // Set adjusted emitter coeff.
    ke = MAX(CSMALL, pr->network.Node[i].Ke);

    // Find flow below which head loss is linear
    qa = pow(hyd->RQtol / ke / hyd->Qexp, 1.0 / (hyd->Qexp - 1.0));

    // Use linear head loss relation for small flow
    q = hyd->EmitterFlows[i];
    if (fabs(q) <= qa)
    {
        *hgrad = hyd->RQtol;
        *hloss = (*hgrad) * q;
    }

    // Otherwise use normal emitter function
    else
    {
        *hgrad = hyd->Qexp * ke * pow(fabs(q), hyd->Qexp - 1.0);
        *hloss = (*hgrad) * q / hyd->Qexp;
    }
}


void demandparams(EN_Project *pr, double *dp, double *n)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  dp = pressure range over which demands can vary
**            n = exponent in head loss v. demand function
**   Purpose: retrieves parameters that define a pressure
**            dependent demand function.
**--------------------------------------------------------------
*/
{
    hydraulics_t *hyd = &pr->hydraulics;

    // If required pressure equals minimum pressure, use a linear demand
    // curve with a 0.01 PSI pressure range to approximate an all or
    // nothing demand solution
    if (hyd->Preq == hyd->Pmin)
    {
        *dp = 0.01 / PSIperFT;
        *n = 1.0;
    }

    // Otherwise use the user-supplied demand curve parameters
    else
    {
        *dp = hyd->Preq - hyd->Pmin;
        *n = 1.0 / hyd->Pexp;
    }
}


void  demandcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coeffs. of the linearized hydraulic eqns.
**            contributed by pressure dependent demands.
**
**   Note: Pressure dependent demands are modelled like emitters
**         with Hloss = Preq * (D / Dfull)^(1/Pexp)
**         where D (actual demand) is zero for negative pressure
**         and is Dfull above pressure Preq.
**--------------------------------------------------------------
*/
{
    int i, row;
    double  dp,         // pressure range over which demand can vary (ft)
            n,          // exponent in head loss v. demand function
            hloss,      // head loss in supplying demand (ft)
            hgrad;      // gradient of demand head loss (ft/cfs)    

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;

    // Get demand function parameters
    if (hyd->DemandModel == DDA) return;
    demandparams(pr, &dp, &n);

    // Examine each junction node
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions with non-positive demands
        if (hyd->NodeDemand[i] <= 0.0) continue;

        // Find head loss for demand outflow at node's elevation
        demandheadloss(hyd->DemandFlows[i], hyd->NodeDemand[i], dp, n,
                    &hloss, &hgrad);

        // Update row of solution matrix A & its r.h.s. F
        row = sol->Row[i];
        sol->Aii[row] += 1.0 / hgrad;
        sol->F[row] += (hloss + net->Node[i].El + hyd->Pmin) / hgrad;
    }
}


double demandflowchange(EN_Project *pr, int i, double dp, double n)
/*
**--------------------------------------------------------------
**   Input:   i  = node index
**            dp = pressure range fro demand funtion (ft)
**            n  = exponent in head v. demand function
**   Output:  returns change in pressure dependent demand flow
**   Purpose: computes change in outflow at at a node subject to
**            pressure dependent demands
**--------------------------------------------------------------
*/
{
    double hloss, hgrad;
    hydraulics_t *hyd = &pr->hydraulics;

    demandheadloss(hyd->DemandFlows[i], hyd->NodeDemand[i], dp, n, &hloss, &hgrad);
    return (hloss - hyd->NodeHead[i] + pr->network.Node[i].El + hyd->Pmin) / hgrad;
}


void demandheadloss(double d, double dfull, double dp, double n,
    double *hloss, double *hgrad)
    /*
    **--------------------------------------------------------------
    **   Input:   d     = actual junction demand (cfs)
    **            dfull = full junction demand required (cfs)
    **            dp    = pressure range for demand function (ft)
    **            n     = exponent in head v. demand function
    **   Output:  hloss = head loss delivering demand d (ft)
    **            hgrad = gradient of head loss (ft/cfs)
    **  Purpose:  computes head loss and its gradient for delivering
    **            a pressure dependent demand flow.
    **--------------------------------------------------------------
    */
{
    const double RB = 1.0e9;
    const double EPS = 0.001;
    double r = d / dfull;

    // Use upper barrier function for excess demand above full value
    if (r > 1.0)
    {
        *hgrad = RB;
        *hloss = dp + RB * (d - dfull);
    }

    // Use lower barrier function for negative demand
    else if (r < 0)
    {
        *hgrad = RB;
        *hloss = RB * d;
    }

    // Use linear head loss function for near zero demand
    else if (r < EPS)
    {
        *hgrad = dp * pow(EPS, n) / dfull / EPS;
        *hloss = (*hgrad) * d;
    }

    // Otherwise use power head loss function
    else
    {
        *hgrad = n * dp * pow(r, n - 1.0) / dfull;
        *hloss = (*hgrad) * d / n;
    }
}


void  pipecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose:  computes P & Y coefficients for pipe k.
**
**    P = inverse head loss gradient = 1/hgrad
**    Y = flow correction term = hloss / hgrad
**--------------------------------------------------------------
*/
{
    double  hloss,     // Head loss
            hgrad,     // Head loss gradient
            ml,        // Minor loss coeff.
            q,         // Abs. value of flow
            r;         // Resistance coeff.

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    // For closed pipe use headloss formula: hloss = CBIG*q
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        sol->P[k] = 1.0 / CBIG;
        sol->Y[k] = hyd->LinkFlows[k];
        return;
    }

    // Use custom function for Darcy-Weisbach formula
    if (hyd->Formflag == DW)
    {
        DWpipecoeff(pr, k);
        return;
    }

    q = ABS(hyd->LinkFlows[k]);
    ml = pr->network.Link[k].Km;
    r = pr->network.Link[k].R;

    // Friction head loss
    // ... use linear relation for small flows
    if (q <= pr->network.Link[k].Qa)
    {
        hgrad = hyd->RQtol;
        hloss = hgrad * q;
    }
    // ... use original formula for other flows
    else
    {
        hgrad = hyd->Hexp * r * pow(q, hyd->Hexp - 1.0);
        hloss = hgrad * q / hyd->Hexp;
    }

    // Contribution of minor head loss
    if (ml > 0.0)
    {
        hloss += ml * q * q;
        hgrad += 2.0 * ml * q;
    }

    // Adjust head loss sign for flow direction
    hloss *= SGN(hyd->LinkFlows[k]);

    // P and Y coeffs.
    sol->P[k] = 1.0 / hgrad;
    sol->Y[k] = hloss / hgrad;
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

    double q = ABS(hyd->LinkFlows[k]);
    double r = link->R;                         // Resistance coeff. 
    double ml = link->Km;                       // Minor loss coeff. 
    double e = link->Kc / link->Diam;           // Relative roughness
    double s = hyd->Viscos * link->Diam;        // Viscosity / diameter
    
    double hloss, hgrad, f, dfdq, r1;
    
    // Compute head loss and its derivative
    // ... use Hagen-Poiseuille formula for laminar flow (Re <= 2000)
    if (q <= A2 * s)
    {
        r = 16.0 * PI * s * r;
        hloss = hyd->LinkFlows[k] * (r + ml * q);
        hgrad  = r + 2.0 * ml * q;
    }
    
    // ... otherwise use Darcy-Weisbach formula with friction factor
    else
    {
        dfdq = 0.0;
        f = frictionFactor(q, e, s, &dfdq);
        r1 = f * r + ml;
        hloss = r1 * q * hyd->LinkFlows[k];
        hgrad = (2.0 * r1 * q) + (dfdq * r * q * q);
    }
    
    // Compute P and Y coefficients
    sol->P[k] = 1.0 / hgrad;
    sol->Y[k] = hloss / hgrad;
}


double frictionFactor(double q, double e, double s, double *dfdq)
/*
**--------------------------------------------------------------
**   Input:   q = |pipe flow|
**            e = pipe roughness  / diameter
**            s = viscosity * pipe diameter
**   Output:  dfdq = derivative of friction factor w.r.t. flow
**   Returns: pipe's friction factor
**   Purpose: computes Darcy-Weisbach friction factor and its
**            derivative as a function of Reynolds Number (Re).
**--------------------------------------------------------------
*/
{
    double f;                // friction factor
    double x1, x2, x3, x4,
           y1, y2, y3,
           fa, fb, r;
    double w = q / s;        // Re*Pi/4

    //   For Re >= 4000 use Swamee & Jain approximation
    //   of the Colebrook-White Formula
    if ( w >= A1 )
    {
        y1 = A8 / pow(w, 0.9);
        y2 = e / 3.7 + y1;
        y3 = A9 * log(y2);
        f = 1.0 / (y3*y3);
        *dfdq = 1.8 * f * y1 * A9 / y2 / y3 / q;
    }

    //   Use interpolating polynomials developed by
    //   E. Dunlop for transition flow from 2000 < Re < 4000.
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
        x4 = 0.032 - 3.0 * fa + 0.5 *fb;
        f = x1 + r * (x2 + r * (x3 + r * x4));
        *dfdq = (x2 + r * (2.0 * x3 + r * 3.0 * x4)) / s / A2;
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
           n,                // Flow exponent coeff.
           setting,          // Pump speed setting
           qa,               // Flow limit for linear head loss
           hloss,            // Head loss across pump
           hgrad;            // Head loss gradient

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Spump        *pump;

    // Use high resistance pipe if pump closed or cannot deliver head
    setting = hyd->LinkSetting[k];
    if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0)
    {
        sol->P[k] = 1.0 / CBIG;
        sol->Y[k] = hyd->LinkFlows[k];
        return;
    }

    // Obtain reference to pump object & its speed setting
    q = ABS(hyd->LinkFlows[k]);
    p = findpump(&pr->network, k);
    pump = &pr->network.Pump[p];

    // Get pump curve coefficients for custom pump curve
    // (Other pump types have pre-determined coeffs.)
    if (pump->Ptype == CUSTOM)
    {
        // Find intercept (h0) & slope (r) of pump curve
        // line segment which contains speed-adjusted flow.
        curvecoeff(pr, pump->Hcurve, q / setting, &h0, &r);

        // Determine head loss coefficients (negative sign
        // converts from pump curve's head gain to head loss)
        pump->H0 = -h0;
        pump->R = -r;
        pump->N = 1.0;

        // Compute head loss and its gradient
        hgrad = pump->R * setting ;
        hloss = pump->H0 * SQR(setting) + hgrad * hyd->LinkFlows[k];
    }
    else
    {
        // Adjust head loss coefficients for pump speed
        h0 = SQR(setting) * pump->H0;
        n = pump->N;
        r = pump->R * pow(setting, 2.0 - n);

        // Compute head loss and its gradient
        // ... use linear approx. to pump curve for small flows
        qa = pow(hyd->RQtol / n / r, 1.0 / (n - 1.0));
        if (q <= qa)
        {
            hgrad = hyd->RQtol;
            hloss = h0 + hgrad * hyd->LinkFlows[k];
        }
        // ... use original pump curve for normal flows
        else
        {
            hgrad = n * r * pow(q, n - 1.0);
            hloss = h0 + hgrad * hyd->LinkFlows[k] / n;
        }
    }

    // P and Y coeffs.
    sol->P[k] = 1.0 / hgrad;
    sol->Y[k] = hloss / hgrad;
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
    while (k2 < npts && x[k2] < q) k2++;
    if (k2 == 0) k2++;
    else if (k2 == npts)  k2--;
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
    int    i;
    double h0,        // Intercept of head loss curve segment
           r,         // Slope of head loss curve segment
           q;         // Abs. value of flow

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    // Treat as a pipe if valve closed
    if (hyd->LinkStatus[k] == CLOSED) valvecoeff(pr, k);                          

    // Otherwise utilize segment of head loss curve
    // bracketing current flow (curve index is stored
    // in valve's setting)
    else
    {
        // Index of valve's head loss curve
        i = (int)ROUND(hyd->LinkSetting[k]);

        // Adjusted flow rate
        q = ABS(hyd->LinkFlows[k]);
        q = MAX(q, TINY);

        // Intercept and slope of curve segment containing q
        curvecoeff(pr, i, q, &h0, &r);
        r = MAX(r, TINY);

        // Resulting P and Y coeffs.
        sol->P[k] = 1.0 / r;
        sol->Y[k] = (h0 / r + q) * SGN(hyd->LinkFlows[k]);
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
    if (hyd->LinkSetting[k] == MISSING || hyd->LinkSetting[k] == 0.0)
    {
        valvecoeff(pr, k);         
    }

    // If valve is active
    else
    {
        // Treat as a pipe if minor loss > valve setting
        if (link->Km * SQR(hyd->LinkFlows[k]) > hyd->LinkSetting[k])
        {
            valvecoeff(pr, k);        
        }
        // Otherwise force headloss across valve to be equal to setting
        else
        {
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
    if (hyd->LinkSetting[k] != MISSING)
    {
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
        if (hyd->X_tmp[n2] < 0.0)
        {
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
        if (hyd->X_tmp[n1] > 0.0)
        {
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
    double flow, q, y, qa, hgrad;
    
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    Slink *link = &net->Link[k];

    flow = hyd->LinkFlows[k];

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
        // Adjust for very small flow
        q = fabs(flow);
        qa = hyd->RQtol / 2.0 / link->Km;
        if (q <= qa)
        {
            hgrad = hyd->RQtol;
            y = flow;
        }
        else
        {
            hgrad = 2.0 * link->Km * q;
            y = flow / 2.0;
        }

        // P and Y coeffs.
        sol->P[k] = 1.0 / hgrad;
        sol->Y[k] = y;
    }

    // If no minor loss coeff. specified use a
    // low resistance linear head loss relation
    else
    {
        sol->P[k] = 1.0 / CSMALL;
        sol->Y[k] = flow;
    }
}
