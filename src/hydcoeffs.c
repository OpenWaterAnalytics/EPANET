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
//void   resistcoeff(EN_Project *pr, int k);
//void   headlosscoeffs(EN_Project *pr);
//void   matrixcoeffs(EN_Project *pr);
//double emitflowchange(EN_Project *pr, int i);
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
static double  frictionFactor(EN_Project *pr, int k, double *dfdq);

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
**   Purpose: computes matrix coefficients for links
**--------------------------------------------------------------
*/
{
    int   k, n1, n2;
    int   fg1, fg2;

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

        // Determine if each end node is fixed grade or not
        fg1 = (n1 > net->Njuncs && hyd->Xtank[n1 - net->Njuncs].FixedGrade);
        fg2 = (n2 > net->Njuncs && hyd->Xtank[n2 - net->Njuncs].FixedGrade);

        // Update net nodal inflows (X_tmp), solution matrix (A) and RHS array (F)
        // (Use covention that flow out of node is (-), flow into node is (+))
        hyd->X_tmp[n1] -= hyd->LinkFlows[k];
        hyd->X_tmp[n2] += hyd->LinkFlows[k];

        // Off-diagonal coeff.
        if (!fg1 && !fg2) sol->Aij[sol->Ndx[k]] -= sol->P[k];

        // Node n1 not fixed grade
        if (!fg1)                     
        {
            sol->Aii[sol->Row[n1]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n1]] += sol->Y[k];     // RHS coeff.
        }

        // Node n1 is fixed grade
        else sol->F[sol->Row[n2]] += (sol->P[k] * hyd->NodeHead[n1]); 

        // Node n2 is not fixed grade
        if (!fg2)
        {
            sol->Aii[sol->Row[n2]] += sol->P[k];   // Diagonal coeff.
            sol->F[sol->Row[n2]] -= sol->Y[k];     // RHS coeff.
        }

        // Node n2 is fixed grade
        else sol->F[sol->Row[n1]] += (sol->P[k] * hyd->NodeHead[n2]); 
    }
}


void  nodecoeffs(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: completes calculation of nodal flow imbalance (X_tmp)
**           & flow correction (F) arrays
**----------------------------------------------------------------
*/
{
    int   i, j, k;
    long   tstep;
    double a;

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;
    EN_Network   *net = &pr->network;

    // For junction nodes, subtract demand flow from net
    // flow imbalance & add imbalance to RHS array F.
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->X_tmp[i] -= hyd->DemandFlows[i];
        sol->F[sol->Row[i]] += hyd->X_tmp[i];
    }

    // For implicit tank dynamics find matrix coeffs. for tanks
    if (hyd->TankDynamics == IMPLICIT)
    {
        tstep = pr->time_options.Hydstep;
        for (j = 1; j <= net->Ntanks; j++)
        {
            // Matrix row corresponding to tank j
            i = net->Tank[j].Node;
            k = sol->Row[i];

            // Tank is fixed grade - force solution to produce it
            if (hyd->Xtank[j].FixedGrade)
            {
                sol->Aii[k] = 1.0;
                sol->F[k] = hyd->NodeHead[i];
            }

            // Tank is dynamic - add area terms into row coeffs.
            else
            {
                a = hyd->Xtank[j].PastArea / tstep;
                sol->Aii[k] += a;
                sol->F[k] += a * hyd->Xtank[j].PastHead + hyd->X_tmp[i];
            }
        }
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
        if (node->Ke == 0.0) continue;
        ke = MAX(CSMALL, node->Ke);              // emitter coeff.
        q = hyd->EmitterFlows[i];                // emitter flow
        z = ke * pow(ABS(q), hyd->Qexp);         // emitter head loss
        p = hyd->Qexp * z / ABS(q);              // head loss gradient
        if (p < hyd->RQtol)
        {
            p = 1.0 / hyd->RQtol;
        }
        else
        {
            p = 1.0 / p;                         // inverse head loss gradient
        }
        y = SGN(q)*z*p;                          // head loss / gradient
        sol->Aii[sol->Row[i]] += p;              // addition to main diagonal
        sol->F[sol->Row[i]] += y + p * node->El; // addition to r.h.s.
        hyd->X_tmp[i] -= q;                      // addition to net node inflow
    }
}


double emitflowchange(EN_Project *pr, int i)
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
    Snode *node = &pr->network.Node[i];

    ke = MAX(CSMALL, node->Ke);
    p = hyd->Qexp * ke * pow(ABS(hyd->EmitterFlows[i]), (hyd->Qexp - 1.0));
    if (p < hyd->RQtol) p = 1 / hyd->RQtol;
    else                p = 1.0 / p;
    return(hyd->EmitterFlows[i] / hyd->Qexp - p * (hyd->NodeHead[i] - node->El));
}


void demandparams(EN_Project *pr, double *dp, double *n)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  dp = pressure range over which demands can vary
**            n = exponent in head loss v. demand function
**   Purpose: retrieves parameters that define a pressure dependent
**            demand function.
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
**   Purpose: computes matrix coeffs. for pressure dependent demands
**
**   Note: Pressure dependent demands are modelled like emitters
**         with Hloss = Pserv * (D / Dfull)^(1/Pexp)
**         where D (actual demand) is zero for negative pressure
**         and is Dfull above pressure Pserv.
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
        *hgrad = dp * pow(EPS, n - 1.0) / dfull;
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

    // Use custom function for Darcy-Weisbach formula
    if (hyd->Formflag == DW)
    {
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

    double q = ABS(hyd->LinkFlows[k]);
    double dfdq = 0.0;
    double r, r1, f, ml, p, hloss;

    ml = link->Km;                       // Minor loss coeff. 
    r = link->R;                         // Resistance coeff. 
    f =  frictionFactor(pr,k,&dfdq);     // D-W friction factor
    r1 = f*r+ml;
 
    // Use large P coefficient for small flow resistance product
    if (r1*q < hyd->RQtol)
    {
        sol->P[k] = 1.0/hyd->RQtol;
        sol->Y[k] = hyd->LinkFlows[k]/hyd->Hexp;
        return;
    }

    // Compute P and Y coefficients
    hloss = r1*SQR(q);                   // Total head loss
    p = 2.0*r1*q;                        // |dHloss/dQ|
    // + dfdq*r*q*q;                     // Ignore df/dQ term 
    p = 1.0 / p;
    sol->P[k] = p;
    sol->Y[k] = SGN(hyd->LinkFlows[k]) * hloss * p;
}


double frictionFactor(EN_Project *pr, int k, double *dfdq)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  returns friction factor and
**            replaces dfdq (derivative of f w.r.t. flow)
**   Purpose: computes Darcy-Weisbach friction factor and its
**            derivative as a function of Reynolds Number (Re).
**
**   Note:    Current formulas for dfdq need to be corrected
**            so dfdq returned as 0.
**--------------------------------------------------------------
*/
{
   double q,             // Abs. value of flow
          f;             // Friction factor 
   double x1,x2,x3,x4,
          y1,y2,y3,
          fa,fb,r;
   double s,w;
  
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *link = &pr->network.Link[k];

   *dfdq = 0.0;
   if (link->Type > EN_PIPE)
     return(1.0);                 // Only apply to pipes
   q = ABS(hyd->LinkFlows[k]);
   s = hyd->Viscos * link->Diam;
   w = q/s;                       // w = Re(Pi/4)
   
   // For Re >= 4000 use Colebrook Formula
   if (w >= A1)                   
   {
      y1 = A8/pow(w,0.9);
      y2 = link->Kc/(3.7*link->Diam) + y1;
      y3 = A9*log(y2);
      f = 1.0/SQR(y3);
      /*  *dfdq = (2.0+AA*y1/(y2*y3))*f; */   /* df/dq */
   }
   
   // For Re > 2000 use Interpolation Formula
   else if (w > A2)              
   {
      y2 = link->Kc/(3.7*link->Diam) + AB;
      y3 = A9*log(y2);
      fa = 1.0/SQR(y3);
      fb = (2.0+AC/(y2*y3))*fa;
      r = w/A2;
      x1 = 7.0*fa - fb;
      x2 = 0.128 - 17.0*fa + 2.5*fb;
      x3 = -0.128 + 13.0*fa - (fb+fb);
      x4 = r*(0.032 - 3.0*fa + 0.5*fb);
      f = x1 + r*(x2 + r*(x3+x4));
      /*  *dfdq = (x1 + x1 + r*(3.0*x2 + r*(4.0*x3 + 5.0*x4)));  */
   }
   
   // For Re > 8 (Laminar Flow) use Hagen-Poiseuille Formula
   else if (w > A4)
   {
      f = A3*s/q;  // 16 * PI * Viscos * Diam / Flow
      /*  *dfdq = A3*s; */
   }
   else
   {
      f = 8.0;
      *dfdq = 0.0;
   }
   return(f);

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
    if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0)
    {
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
    if (n != 1.0) r = n * r * pow(q, n - 1.0);

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
    double h0,        // Headloss curve intercept
           q,         // Abs. value of flow
           r;         // Flow resistance coeff.

    hydraulics_t *hyd = &pr->hydraulics;
    solver_t     *sol = &hyd->solver;

    // Treat as a pipe if valve closed
    if (hyd->LinkStatus[k] == CLOSED) valvecoeff(pr, k);                          

    // Otherwise utilize headloss curve
    // whose index is stored in K
    else
    {
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
        if (p < hyd->RQtol) p = hyd->RQtol;
        sol->P[k] = 1.0 / p;
        sol->Y[k] = flow / 2.0;
    }
    else
    {
        sol->P[k] = 1.0 / hyd->RQtol;
        sol->Y[k] = flow;
    }
}
