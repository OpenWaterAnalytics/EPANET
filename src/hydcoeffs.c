/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       hydcoeffs.c
 Description:  computes coefficients for a hydraulic solution matrix
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 10/04/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

// Definitions of very small and very big coefficients
const double CSMALL = 1.e-6;
const double CBIG   = 1.e8;

// Exported functions
//void   resistcoeff(Project *, int );
//void   headlosscoeffs(Project *);
//void   matrixcoeffs(Project *);
//void   emitterheadloss(Project *, int, double *, double *);
//void   demandheadloss(Project *, int, double, double, double *, double *);

// Local functions
static void    linkcoeffs(Project *pr);
static void    nodecoeffs(Project *pr);
static void    valvecoeffs(Project *pr);
static void    emittercoeffs(Project *pr);
static void    demandcoeffs(Project *pr);

static void    pipecoeff(Project *pr, int k);
static void    DWpipecoeff(Project *pr, int k);
static double  frictionFactor(double q, double e, double s, double *dfdq);

static void    pumpcoeff(Project *pr, int k);
static void    curvecoeff(Project *pr, int i, double q, double *h0, double *r);

static void    valvecoeff(Project *pr, int k);
static void    gpvcoeff(Project *pr, int k);
static void    pbvcoeff(Project *pr, int k);
static void    tcvcoeff(Project *pr, int k);
static void    prvcoeff(Project *pr, int k, int n1, int n2);
static void    psvcoeff(Project *pr, int k, int n1, int n2);
static void    fcvcoeff(Project *pr, int k, int n1, int n2);


void  resistcoeff(Project *pr, int k)
/*
**--------------------------------------------------------------------
**  Input:   k = link index
**  Output:  none
**  Purpose: computes link flow resistance coefficient
**--------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    double e, d, L;
    Slink *link = &net->Link[k];

    switch (link->Type) {

    // ... Link is a pipe. Compute resistance based on headloss formula.
    //     Friction factor for D-W formula gets included during head loss
    //     calculation.
    case CVPIPE:
    case PIPE:
        e = link->Kc;                 // Roughness coeff.
        d = link->Diam;               // Diameter
        L = link->Len;                // Length
        switch (hyd->Formflag)
        {
        case HW:
            link->R = 4.727 * L / pow(e, hyd->Hexp) / pow(d, 4.871);
            break;
        case DW:
            link->R = L / 2.0 / 32.2 / d / SQR(PI * SQR(d) / 4.0);
            break;
        case CM:
            link->R = SQR(4.0 * e / (1.49 * PI * SQR(d))) *
                      pow((d / 4.0), -1.333) * L;
        }
        break;

    // ... Link is a pump. Use huge resistance.
    case PUMP:
        link->R = CBIG;
        break;

    // ... For all other links (e.g. valves) use a small resistance
    default:
        link->R = CSMALL;
        break;
    }
}


void headlosscoeffs(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coefficients P (1 / head loss gradient)
**            and Y (head loss / gradient) for all links.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int k;

    for (k = 1; k <= net->Nlinks; k++)
    {
        switch (net->Link[k].Type)
        {
        case CVPIPE:
        case PIPE:
            pipecoeff(pr, k);
            break;
        case PUMP:
            pumpcoeff(pr, k);
            break;
        case PBV:
            pbvcoeff(pr, k);
            break;
        case TCV:
            tcvcoeff(pr, k);
            break;
        case GPV:
            gpvcoeff(pr, k);
            break;
        case FCV:
        case PRV:
        case PSV:
            if (hyd->LinkSetting[k] == MISSING) valvecoeff(pr, k);
            else hyd->P[k] = 0.0;
        }
    }
}


void   matrixcoeffs(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: computes coefficients of linearized network eqns.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    // Reset values of all diagonal coeffs. (Aii), off-diagonal
    // coeffs. (Aij), r.h.s. coeffs. (F) and node excess flow (Xflow)
    memset(sm->Aii, 0, (net->Nnodes + 1) * sizeof(double));
    memset(sm->Aij, 0, (sm->Ncoeffs + 1) * sizeof(double));
    memset(sm->F, 0, (net->Nnodes + 1) * sizeof(double));
    memset(hyd->Xflow, 0, (net->Nnodes + 1) * sizeof(double));

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


void  linkcoeffs(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coefficients contributed by links to the
**            linearized system of hydraulic equations.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int   k, n1, n2;
    Slink *link;

    // Examine each link of network
    for (k = 1; k <= net->Nlinks; k++)
    {
        if (hyd->P[k] == 0.0) continue;
        link = &net->Link[k];
        n1 = link->N1;           // Start node of link
        n2 = link->N2;           // End node of link

        // Update nodal flow excess (Xflow)
        // (Flow out of node is (-), flow into node is (+))
        hyd->Xflow[n1] -= hyd->LinkFlow[k];
        hyd->Xflow[n2] += hyd->LinkFlow[k];

        // Add to off-diagonal coeff. of linear system matrix
        sm->Aij[sm->Ndx[k]] -= hyd->P[k];

        // Update linear system coeffs. associated with start node n1
        // ... node n1 is junction
        if (n1 <= net->Njuncs)
        {
            sm->Aii[sm->Row[n1]] += hyd->P[k];   // Diagonal coeff.
            sm->F[sm->Row[n1]] += hyd->Y[k];     // RHS coeff.
        }

        // ... node n1 is a tank/reservoir
        else sm->F[sm->Row[n2]] += (hyd->P[k] * hyd->NodeHead[n1]);

        // Update linear system coeffs. associated with end node n2
        // ... node n2 is junction
        if (n2 <= net->Njuncs)
        {
            sm->Aii[sm->Row[n2]] += hyd->P[k];   // Diagonal coeff.
            sm->F[sm->Row[n2]] -= hyd->Y[k];     // RHS coeff.
        }

        // ... node n2 is a tank/reservoir
        else sm->F[sm->Row[n1]] += (hyd->P[k] * hyd->NodeHead[n2]);
    }
}


void  nodecoeffs(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: completes calculation of nodal flow balance array
**           (Xflow) & r.h.s. (F) of linearized hydraulic eqns.
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int   i;

    // For junction nodes, subtract demand flow from net
    // flow excess & add flow excess to RHS array F
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->Xflow[i] -= hyd->DemandFlow[i];
        sm->F[sm->Row[i]] += hyd->Xflow[i];
    }
}


void  valvecoeffs(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int i, k, n1, n2;
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
        case PRV:
            prvcoeff(pr, k, n1, n2);
            break;
        case PSV:
            psvcoeff(pr, k, n1, n2);
            break;
        case FCV:
            fcvcoeff(pr, k, n1, n2);
            break;
        default:   continue;
        }
    }
}


void  emittercoeffs(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int    i, row;
    double hloss, hgrad;
    Snode  *node;

    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions without emitters
        node = &net->Node[i];
        if (node->Ke == 0.0) continue;

        // Find emitter head loss and gradient
        emitterheadloss(pr, i, &hloss, &hgrad);

        // Row of solution matrix
        row = sm->Row[i];

        // Addition to matrix diagonal & r.h.s
        sm->Aii[row] += 1.0 / hgrad;
        sm->F[row] += (hloss + node->El) / hgrad;

        // Update to node flow excess
        hyd->Xflow[i] -= hyd->EmitterFlow[i];
    }
}


void emitterheadloss(Project *pr, int i, double *hloss, double *hgrad)
/*
**-------------------------------------------------------------
**   Input:   i = node index
**   Output:  hloss = head loss across node's emitter
**            hgrad = head loss gradient
**   Purpose: computes an emitters's head loss and gradient.
**-------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    double  ke;
    double  q;

    // Set adjusted emitter coeff.
    ke = MAX(CSMALL, pr->network.Node[i].Ke);

    // Compute gradient of head loss through emitter
    q = hyd->EmitterFlow[i];
    *hgrad = hyd->Qexp * ke * pow(fabs(q), hyd->Qexp - 1.0);
    
    // Use linear head loss function for small gradient
    if (*hgrad < hyd->RQtol)
    {
        *hgrad = hyd->RQtol;
        *hloss = (*hgrad) * q;
    }            

    // Otherwise use normal emitter head loss function
    else *hloss = (*hgrad) * q / hyd->Qexp;
}


void  demandcoeffs(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int i, row;
    double  dp,         // pressure range over which demand can vary (ft)
            n,          // exponent in head loss v. demand function
            hloss,      // head loss in supplying demand (ft)
            hgrad;      // gradient of demand head loss (ft/cfs)
            
    // Get demand function parameters
    if (hyd->DemandModel == DDA) return;
    dp = hyd->Preq - hyd->Pmin;
    n = 1.0 / hyd->Pexp;

    // Examine each junction node
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions with non-positive demands
        if (hyd->NodeDemand[i] <= 0.0) continue;
        
        // Find head loss for demand outflow at node's elevation
        demandheadloss(pr, i, dp, n, &hloss, &hgrad);
                    
        // Update row of solution matrix A & its r.h.s. F
        if (hgrad > 0.0)
        {
            row = sm->Row[i];
            sm->Aii[row] += 1.0 / hgrad;
            sm->F[row] += (hloss + net->Node[i].El + hyd->Pmin) / hgrad;
        }
    }
}

void demandheadloss(Project *pr, int i, double dp, double n,
                    double *hloss, double *hgrad)
/*
**--------------------------------------------------------------
**   Input:   i  = junction index
**            dp = pressure range for demand function (ft)
**            n  = exponent in head v. demand function
**   Output:  hloss = pressure dependent demand head loss (ft)
**            hgrad = gradient of head loss (ft/cfs)
**  Purpose:  computes head loss and its gradient for delivering
**            a pressure dependent demand flow.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
   
    double d = hyd->DemandFlow[i];
    double dfull = hyd->NodeDemand[i];
    double r = d / dfull;
    
    // Use lower barrier function for negative demand
    if (r <= 0)
    {
        *hgrad = CBIG;
        *hloss = CBIG * d;
    }

    // Use power head loss function for demand less than full
    else if (r < 1.0)
    {
        *hgrad = n * dp * pow(r, n - 1.0) / dfull;
        // ... use linear function for very small gradient
        if (*hgrad < hyd->RQtol)
        {
            *hgrad = hyd->RQtol;
            *hloss = (*hgrad) * d;
        }
        else *hloss = (*hgrad) * d / n;
    }
    
    // Use upper barrier function for demand above full value
    else
    {
        *hgrad = CBIG;
        *hloss = dp + CBIG * (d - dfull);
    }
}


void  pipecoeff(Project *pr, int k)
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
    Hydraul *hyd = &pr->hydraul;

    double  hloss,     // Head loss
            hgrad,     // Head loss gradient
            ml,        // Minor loss coeff.
            q,         // Abs. value of flow
            r;         // Resistance coeff.

    // For closed pipe use headloss formula: hloss = CBIG*q
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        hyd->P[k] = 1.0 / CBIG;
        hyd->Y[k] = hyd->LinkFlow[k];
        return;
    }

    // Use custom function for Darcy-Weisbach formula
    if (hyd->Formflag == DW)
    {
        DWpipecoeff(pr, k);
        return;
    }

    q = ABS(hyd->LinkFlow[k]);
    ml = pr->network.Link[k].Km;
    r = pr->network.Link[k].R;

    // Friction head loss gradient
    hgrad = hyd->Hexp * r * pow(q, hyd->Hexp - 1.0);
    
    // Friction head loss:
    // ... use linear function for very small gradient
    if (hgrad < hyd->RQtol)
    {
        hgrad = hyd->RQtol;
        hloss = hgrad * q;
    }
    // ... otherwise use original formula
    else hloss = hgrad * q / hyd->Hexp;
    
    // Contribution of minor head loss
    if (ml > 0.0)
    {
        hloss += ml * q * q;
        hgrad += 2.0 * ml * q;
    }

    // Adjust head loss sign for flow direction
    hloss *= SGN(hyd->LinkFlow[k]);

    // P and Y coeffs.
    hyd->P[k] = 1.0 / hgrad;
    hyd->Y[k] = hloss / hgrad;
}


void DWpipecoeff(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes pipe head loss coeffs. for Darcy-Weisbach
**            formula.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Slink   *link = &pr->network.Link[k];

    double q = ABS(hyd->LinkFlow[k]);
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
        hloss = hyd->LinkFlow[k] * (r + ml * q);
        hgrad  = r + 2.0 * ml * q;
    }

    // ... otherwise use Darcy-Weisbach formula with friction factor
    else
    {
        dfdq = 0.0;
        f = frictionFactor(q, e, s, &dfdq);
        r1 = f * r + ml;
        hloss = r1 * q * hyd->LinkFlow[k];
        hgrad = (2.0 * r1 * q) + (dfdq * r * q * q);
    }

    // Compute P and Y coefficients
    hyd->P[k] = 1.0 / hgrad;
    hyd->Y[k] = hloss / hgrad;
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


void  pumpcoeff(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for pump in link k
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    int    p;                // Pump index
    double h0,               // Shutoff head
           q,                // Abs. value of flow
           r,                // Flow resistance coeff.
           n,                // Flow exponent coeff.
           setting,          // Pump speed setting
           hloss,            // Head loss across pump
           hgrad;            // Head loss gradient
    Spump  *pump;

    // Use high resistance pipe if pump closed or cannot deliver head
    setting = hyd->LinkSetting[k];
    if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0)
    {
        hyd->P[k] = 1.0 / CBIG;
        hyd->Y[k] = hyd->LinkFlow[k];
        return;
    }

    // Obtain reference to pump object
    q = ABS(hyd->LinkFlow[k]);
    p = findpump(&pr->network, k);
    pump = &pr->network.Pump[p];

    // If no pump curve treat pump as an open valve
    if (pump->Ptype == NOCURVE)
    {
        hyd->P[k] = 1.0 / CSMALL;
        hyd->Y[k] = hyd->LinkFlow[k];
        return;
    }

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

        // Compute head loss and its gradient (with speed adjustment)
        hgrad = pump->R * setting ;
        hloss = pump->H0 * SQR(setting) + hgrad * hyd->LinkFlow[k];
    }
    else
    {
        // Adjust head loss coefficients for pump speed
        h0 = SQR(setting) * pump->H0;
        n = pump->N;
        if (ABS(n - 1.0) < TINY) n = 1.0;
        r = pump->R * pow(setting, 2.0 - n);
        
        // Constant HP pump
        if (pump->Ptype == CONST_HP)
        {
            // ... compute pump curve's gradient
            hgrad = -r / q / q;
            // ... use linear curve if gradient too large or too small
            if (hgrad > CBIG)
            {
                hgrad = CBIG;
                hloss = -hgrad * hyd->LinkFlow[k];
            }
            else if (hgrad < hyd->RQtol)
            {
                hgrad = hyd->RQtol;
                hloss = -hgrad * hyd->LinkFlow[k];
            }
            // ... otherwise compute head loss from pump curve
            else
            {
                hloss = r / hyd->LinkFlow[k];
            }
        }            

        // Compute head loss and its gradient
        // ... pump curve is nonlinear
        else if (n != 1.0)
        {
            // ... compute pump curve's gradient
            hgrad = n * r * pow(q, n - 1.0);
            // ... use linear pump curve if gradient too small
            if (hgrad < hyd->RQtol)
            {
                hgrad = hyd->RQtol;
                hloss = h0 + hgrad * hyd->LinkFlow[k];
            }
            // ... otherwise compute head loss from pump curve
            else hloss = h0 + hgrad * hyd->LinkFlow[k] / n;
        }
        // ... pump curve is linear
        else
        {
            hgrad = r;
            hloss = h0 + hgrad * hyd->LinkFlow[k];
        }
    }

    // P and Y coeffs.
    hyd->P[k] = 1.0 / hgrad;
    hyd->Y[k] = hloss / hgrad;
}


void  curvecoeff(Project *pr, int i, double q, double *h0, double *r)
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


void  gpvcoeff(Project *pr, int k)
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

    Hydraul *hyd = &pr->hydraul;

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
        q = ABS(hyd->LinkFlow[k]);
        q = MAX(q, TINY);

        // Intercept and slope of curve segment containing q
        curvecoeff(pr, i, q, &h0, &r);
        r = MAX(r, TINY);

        // Resulting P and Y coeffs.
        hyd->P[k] = 1.0 / r;
        hyd->Y[k] = (h0 / r + q) * SGN(hyd->LinkFlow[k]);
    }
}


void  pbvcoeff(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for pressure breaker valve
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
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
        if (link->Km * SQR(hyd->LinkFlow[k]) > hyd->LinkSetting[k])
        {
            valvecoeff(pr, k);
        }
        // Otherwise force headloss across valve to be equal to setting
        else
        {
            hyd->P[k] = CBIG;
            hyd->Y[k] = hyd->LinkSetting[k] * CBIG;
        }
    }
}


void  tcvcoeff(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: computes P & Y coeffs. for throttle control valve
**--------------------------------------------------------------
*/
{
    double km;
    Hydraul *hyd = &pr->hydraul;
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


void  prvcoeff(Project *pr, int k, int n1, int n2)
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
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int   i, j;                        // Rows of solution matrix
    double hset;                       // Valve head setting

    i = sm->Row[n1];                  // Matrix rows of nodes
    j = sm->Row[n2];
    hset = pr->network.Node[n2].El +
           hyd->LinkSetting[k];        // Valve setting

    if (hyd->LinkStatus[k] == ACTIVE)
    {

        // Set coeffs. to force head at downstream
        // node equal to valve setting & force flow
        // to equal to flow excess at downstream node.

        hyd->P[k] = 0.0;
        hyd->Y[k] = hyd->LinkFlow[k] + hyd->Xflow[n2];   // Force flow balance
        sm->F[j] += (hset * CBIG);                        // Force head = hset
        sm->Aii[j] += CBIG;                               // at downstream node
        if (hyd->Xflow[n2] < 0.0)
        {
            sm->F[i] += hyd->Xflow[n2];
        }
        return;
    }

    // For OPEN, CLOSED, or XPRESSURE valve
    // compute matrix coeffs. using the valvecoeff() function.

    valvecoeff(pr, k);
    sm->Aij[sm->Ndx[k]] -= hyd->P[k];
    sm->Aii[i] += hyd->P[k];
    sm->Aii[j] += hyd->P[k];
    sm->F[i] += (hyd->Y[k] - hyd->LinkFlow[k]);
    sm->F[j] -= (hyd->Y[k] - hyd->LinkFlow[k]);
}


void  psvcoeff(Project *pr, int k, int n1, int n2)
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
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int   i, j;                        // Rows of solution matrix
    double hset;                       // Valve head setting

    i = sm->Row[n1];                   // Matrix rows of nodes
    j = sm->Row[n2];
    hset = pr->network.Node[n1].El +
           hyd->LinkSetting[k];        // Valve setting

    if (hyd->LinkStatus[k] == ACTIVE)
    {
        // Set coeffs. to force head at upstream
        // node equal to valve setting & force flow
        // equal to flow excess at upstream node.

        hyd->P[k] = 0.0;
        hyd->Y[k] = hyd->LinkFlow[k] - hyd->Xflow[n1];   // Force flow balance
        sm->F[i] += (hset * CBIG);                        // Force head = hset
        sm->Aii[i] += CBIG;                               // at upstream node
        if (hyd->Xflow[n1] > 0.0)
        {
            sm->F[j] += hyd->Xflow[n1];
        }
        return;
    }

    // For OPEN, CLOSED, or XPRESSURE valve
    // compute matrix coeffs. using the valvecoeff() function.

    valvecoeff(pr, k);
    sm->Aij[sm->Ndx[k]] -= hyd->P[k];
    sm->Aii[i] += hyd->P[k];
    sm->Aii[j] += hyd->P[k];
    sm->F[i] += (hyd->Y[k] - hyd->LinkFlow[k]);
    sm->F[j] -= (hyd->Y[k] - hyd->LinkFlow[k]);
}


void  fcvcoeff(Project *pr, int k, int n1, int n2)
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
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int   i, j;                   // Rows in solution matrix
    double q;                     // Valve flow setting

    q = hyd->LinkSetting[k];
    i = sm->Row[n1];
    j = sm->Row[n2];

    // If valve active, break network at valve and treat
    // flow setting as external demand at upstream node
    // and external supply at downstream node.

    if (hyd->LinkStatus[k] == ACTIVE)
    {
        hyd->Xflow[n1] -= q;
        hyd->Xflow[n2] += q;
        hyd->Y[k] = hyd->LinkFlow[k] - q;
        sm->F[i] -= q;
        sm->F[j] += q;
        hyd->P[k] = 1.0 / CBIG;
        sm->Aij[sm->Ndx[k]] -= hyd->P[k];
        sm->Aii[i] += hyd->P[k];
        sm->Aii[j] += hyd->P[k];
    }

    // Otherwise treat valve as an open pipe

    else
    {
        valvecoeff(pr, k);
        sm->Aij[sm->Ndx[k]] -= hyd->P[k];
        sm->Aii[i] += hyd->P[k];
        sm->Aii[j] += hyd->P[k];
        sm->F[i] += (hyd->Y[k] - hyd->LinkFlow[k]);
        sm->F[j] -= (hyd->Y[k] - hyd->LinkFlow[k]);
    }
}


void valvecoeff(Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k    = link index
**   Output:  none
**   Purpose: computes solution matrix coeffs. for a completely
**            open, closed, or throttled control valve.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Slink *link = &pr->network.Link[k];

    double flow, q, hloss, hgrad;

    flow = hyd->LinkFlow[k];

    // Valve is closed. Use a very small matrix coeff.
    if (hyd->LinkStatus[k] <= CLOSED)
    {
        hyd->P[k] = 1.0 / CBIG;
        hyd->Y[k] = flow;
        return;
    }

    // Account for any minor headloss through the valve
    if (link->Km > 0.0)
    {
        q = fabs(flow);
        hgrad = 2.0 * link->Km * q;
        
        // Guard against too small a head loss gradient
        if (hgrad < hyd->RQtol)
        {
            hgrad = hyd->RQtol;
            hloss = flow * hgrad;
        }
        else hloss = flow * hgrad / 2.0;        
        
        // P and Y coeffs.
        hyd->P[k] = 1.0 / hgrad;
        hyd->Y[k] = hloss / hgrad;
    }

    // If no minor loss coeff. specified use a
    // low resistance linear head loss relation
    else
    {
        hyd->P[k] = 1.0 / CSMALL;
        hyd->Y[k] = flow;
    }
}
