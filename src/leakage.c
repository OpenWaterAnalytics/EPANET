/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       leakage.c
 Description:  models additional nodal demands due to pipe leaks.
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 06/14/2024
 ******************************************************************************
*/
/*
This module uses the FAVAD (Fixed and Variable Discharge) equation to model
leaky pipes:
  
  Q = Co * L * (Ao + m * H) * sqrt(H)
  
where Q = leak flow rate, Co = an orifice coefficient (= 0.6*sqrt(2g)),
L = pipe length, Ao = initial area of leak per unit of pipe length,
m = change in leak area per unit of pressure head, and H = pressure head.

The inverted form of this equation is used to model the leakage demand from
a pipe's end node using a pair of equivalent emitters as follows:

  H = Cfa * Qfa^2
  H = Cva * Qva^(2/3)
  
where Qfa = fixed area leakage rate, Qva = variable area leakage rate,
Cfa = 1 / SUM(Co*(L/2)*Ao)^2, Cva = 1 / SUM(Co*(L/2)*m)^2/3, and
SUM(x) is the summation of x over all pipes connected to the node.

In implementing this model, the pipe property "LeakArea" represents Ao in
sq. mm per 100 units of pipe length and "LeakExpan" represents m in sq. mm
per unit of pressure head.

*/
#include <stdlib.h>
#include <math.h>

#include "types.h"

// Exported functions (declared in funcs.h)
//int     leakage_open(Project *);
//void    leakage_init(Project *);
//void    leakage_close(Project *);
//double  leakage_getlinkleakage(Project *, int);
//void    leakage_solvercoeffs(Project *);
//double  leakage_getflowchange(Project *, int);
//int     leakage_hasconverged(Project *);

// Local functions
static int   findnodecoeffs(Project* pr, int i, double *hfa,
             double *gfa, double *hva, double *gva);
static void  evalnodeleakage(double RQtol, double q, double c,
             double n, double *h, double *g);
static void  addlowerbarrier(double q, double* h, double* g);

int leakage_open(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  returns an error code
**   Purpose: allocates memory for nodal leakage objects
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    hyd->Leakage = (Sleakage *)calloc(net->Njuncs + 1, sizeof(Sleakage));
    if (hyd->Leakage == NULL) return 101;
    else return 0;
}

void leakage_init(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: initializes the coefficients used by each junction
**            node's Leakage object. 
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int i;
    double c_area, c_expan, c_orif, len;
    Slink *link;
    Snode *node1;
    Snode *node2;

    // Initialize contents of junction Leakage objects
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->Leakage[i].cfa = 0.0;
        hyd->Leakage[i].cva = 0.0;
        hyd->Leakage[i].qfa = 0.0;
        hyd->Leakage[i].qva = 0.0;
    }
 
    // Accumulate pipe leak coeffs. to their respective end nodes
    hyd->HasLeakage = FALSE;
    c_orif = 4.8149866 * 1.e-6;
    for (i = 1; i <= net->Nlinks; i++)
    {
        // Only pipes have leakage
        link = &net->Link[i];
        if (link->Type > PIPE) continue;
        
        // Ignore leakage in a pipe connecting two tanks or
        // reservoirs (since those nodes don't have demands)
        node1 = &net->Node[link->N1];
        node2 = &net->Node[link->N2];
        if (node1->Type != JUNCTION && node2->Type != JUNCTION) continue;

        // Fixed and variable area leak coeffs.
        if (link->LeakArea == 0.0 && link->LeakExpan == 0.0) continue;
        c_area = c_orif * link->LeakArea / SQR(MperFT);
        c_expan = c_orif * link->LeakExpan;
        hyd->HasLeakage = TRUE;

        // Adjustment for number of 100-ft pipe sections
        len = link->Len * pr->Ucf[LENGTH] / 100.;
        if (node1->Type == JUNCTION && node2->Type == JUNCTION)
        {
            len *= 0.5;
        }
        c_area *= len;
        c_expan *= len;
        
        // Accumulate coeffs. at pipe's end nodes
        if (node1->Type == JUNCTION)
        {
            hyd->Leakage[link->N1].cfa += c_area;
            hyd->Leakage[link->N1].cva += c_expan;
        }
        if (node2->Type == JUNCTION)
        {
            hyd->Leakage[link->N2].cfa += c_area;
            hyd->Leakage[link->N2].cva += c_expan;
        }
    }
    
    // Compute coeffs. for inverted form of the leakage formula
    // at each junction node
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Coeff. for fixed area leakage
        c_area = hyd->Leakage[i].cfa;
        if (c_area > 0.0)
            hyd->Leakage[i].cfa = 1.0 / (c_area * c_area);
        else
            hyd->Leakage[i].cfa = 0.0;
        
        // Coeff. for variable area leakage
        c_expan = hyd->Leakage[i].cva;
        if (c_expan > 0.0)
            hyd->Leakage[i].cva = 1.0 / pow(c_expan, 2./3.);
        else
            hyd->Leakage[i].cva = 0.0;
        
        // Initialize leakage flow to a non-zero value (as required by
        // the hydraulic solver)
        if (hyd->Leakage[i].cfa > 0.0)
            hyd->Leakage[i].qfa = 0.001;
        if (hyd->Leakage[i].cva > 0.0)
            hyd->Leakage[i].qva = 0.001;
    }
}

void leakage_close(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: frees memory for nodal leakage objects
**-------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    if (hyd->Leakage) free(hyd->Leakage);
    hyd->Leakage = NULL;
    hyd->HasLeakage = FALSE;
}
   
double leakage_getlinkleakage(Project *pr, int i)
/*-------------------------------------------------------------
**   Input:   i = link index
**   Output:  returns link leakage flow (cfs)
**   Purpose: computes leakage flow from link i at current
**            hydraulic solution
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;
    Slink *link = &net->Link[i];
    
    int    n1, n2;
    double h1, h2, hsqrt, a, m, c, len, q1, q2;

    // Only pipes can leak
    link = &net->Link[i];
    if (link->Type > PIPE) return 0.0;
    
    // No leakage if area & expansion are 0
    if (link->LeakArea == 0.0 && link->LeakExpan == 0.0) return 0.0;
   
    // No leakage if link's end nodes are both fixed grade
    n1 = link->N1;
    n2 = link->N2;
    if (n1 > net->Njuncs && n2 > net->Njuncs) return 0.0;
    
    // Pressure head of end nodes
    h1 = hyd->NodeHead[n1] - net->Node[n1].El;
    h1 = MAX(h1, 0.0);
    h2 = hyd->NodeHead[n2] - net->Node[n2].El;
    h2 = MAX(h2, 0.0);
    
    // Pipe leak parameters converted to feet
    a = link->LeakArea / SQR(MperFT);
    m = link->LeakExpan;
    len = link->Len * pr->Ucf[LENGTH] / 100.; // # 100 ft pipe lengths
    c = 4.8149866 * len / 2.0 * 1.e-6;
    
    // Leakage from 1st half of pipe connected to node n1
    q1 = 0.0;
    if (n1 <= net->Njuncs)
    {
        hsqrt = sqrt(h1);    
        q1 = c * (a + m * h1) * hsqrt;
    }
    
    // Leakage from 2nd half of pipe connected to node n2
    q2 = 0.0;
    if (n2 <= net->Njuncs)
    {
        hsqrt = sqrt(h2);    
        q2 = c * (a + m * h2) * hsqrt;
    }
    
    // Adjust leakage flows to account for one node being fixed grade
    if (q2 == 0.0) q1 *= 2.0;
    if (q1 == 0.0) q2 *= 2.0;
    return q1 + q2;
}    

void leakage_solvercoeffs(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coeffs. of the linearized hydraulic eqns.
**            contributed by pipe leakage.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int    i, row;
    double hfa,    // fixed area head producing current fixed area leakage
           gfa,    // gradient of fixed area head w.r.t. leakage flow
           hva,    // variable area head producing current variable area leakage
           gva;    // gradient of variable area head w.r.t. leakage flow
    
    Snode* node;
    
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions that don't leak
        node = &net->Node[i];
        if (!findnodecoeffs(pr, i, &hfa, &gfa, &hva, &gva)) continue;

        // Addition to matrix diagonal & r.h.s
        row = sm->Row[i];
        if (gfa > 0.0)
        {
            sm->Aii[row] += 1.0 / gfa;
            sm->F[row] += (hfa + node->El) / gfa;
        }
        if (gva > 0.0)
        {
            sm->Aii[row] += 1.0 / gva;
            sm->F[row] += (hva + node->El) / gva;
        }

        // Update node's flow excess (inflow - outflow)
        hyd->Xflow[i] -= (hyd->Leakage[i].qfa + hyd->Leakage[i].qva);
    }
}

double leakage_getflowchange(Project *pr, int i)
/*
**--------------------------------------------------------------
**   Input:   i = node index
**   Output:  returns change in leakage flow rate
**   Purpose: finds new leakage flow rate at a node after new
**            heads are computed by the hydraulic solver.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    double  hfa, gfa, hva, gva,  // same as defined in leakage_solvercoeffs()
            dh, dqfa, dqva;
    
    // Find the head loss and gradient of the inverted leakage
    // equation for both fixed and variable area leakage at the
    // current leakage flow rates
    if (!findnodecoeffs(pr, i, &hfa, &gfa, &hva, &gva)) return 0.0;
    
    // Pressure head using latest head solution
    dh = hyd->NodeHead[i] - net->Node[i].El;

    // GGA flow update formula for fixed area leakage
    dqfa = 0.0;
    if (gfa > 0.0)
    {
        dqfa = (hfa - dh) / gfa * hyd->RelaxFactor;
        hyd->Leakage[i].qfa -= dqfa;
    }

    // GGA flow update formula for variable area leakage
    dqva = 0.0;
    if (gva > 0.0)
    {
        dqva = (hva - dh) / gva * hyd->RelaxFactor;
        hyd->Leakage[i].qva -= dqva;
    }

    // New leakage flow at the node 
    hyd->LeakageFlow[i] = hyd->Leakage[i].qfa + hyd->Leakage[i].qva;
    return dqfa + dqva;
}

int leakage_hasconverged(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns 1 if leakage calculations converged,
**            0 if not
**   Purpose: checks if leakage calculations have converged.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    int i;
    double h, qref, qtest;
    const double ABSTOL = 0.0001;  // 0.0001 cfs ~= 0.005 gpm ~= 0.2 lpm)
    const double RELTOL = 0.001;

    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions that don't leak
        if (hyd->Leakage[i].cfa == 0 && hyd->Leakage[i].cva == 0) continue;
        
        // Evaluate node's pressure head
        h = hyd->NodeHead[i] - net->Node[i].El;
        
        // Directly compute a reference leakage at this pressure head
        qref = 0.0;
        // Contribution from pipes with fixed area leaks
        if (hyd->Leakage[i].cfa > 0.0)
            qref = sqrt(h / hyd->Leakage[i].cfa);
        // Contribution from pipes with variable area leaks
        if (hyd->Leakage[i].cva > 0.0)
            qref += pow((h / hyd->Leakage[i].cva), 1.5);
        
        // Compare reference leakage to solution leakage
        qtest = hyd->Leakage[i].qfa + hyd->Leakage[i].qva;
        if (fabs(qref - qtest) > ABSTOL + RELTOL * qref) return 0;
    }
    return 1;
}

int findnodecoeffs(Project* pr, int i, double *hfa, double *gfa,
    double *hva, double *gva)
/*
**--------------------------------------------------------------
**   Input:   i = node index
**   Output:  hfa = fixed area leak head (ft)
**            gfa = gradient of fixed area head (ft/cfs)
**            hva = variable area leak head (ft)
**            gva = gradient of variable area head (ft/cfs)
**            returns TRUE if node has leakage, FALSE otherwise
**   Purpose: finds head and its gradient for a node's leakage
**            as a function of leakage flow.
**--------------------------------------------------------------
*/
{     
    Hydraul *hyd = &pr->hydraul;
    if (hyd->Leakage[i].cfa == 0.0 && hyd->Leakage[i].cva == 0.0) return FALSE;
    if (hyd->Leakage[i].cfa == 0.0)
    {
        *hfa = 0.0;
        *gfa = 0.0;
    }
    else
        evalnodeleakage(hyd->RQtol, hyd->Leakage[i].qfa, hyd->Leakage[i].cfa,
                        0.5, hfa, gfa);
    if (hyd->Leakage[i].cva == 0.0)
    {
        *hva = 0.0;
        *gva = 0.0;
    }
    else
        evalnodeleakage(hyd->RQtol, hyd->Leakage[i].qva, hyd->Leakage[i].cva,
                        1.5, hva, gva);
    return TRUE;
}

void evalnodeleakage(double RQtol, double q, double c, double n,
    double *h, double *g)
/*
**--------------------------------------------------------------
**   Input:   RQtol = low gradient tolerance (ft/cfs)
**            q = leakage flow rate (cfs)
**            c = leakage head loss coefficient
**            n = leakage head loss exponent
**   Output:  h = leakage head loss (ft)
**            g = gradient of leakage head loss (ft/cfs)
**   Purpose: evaluates inverted form of leakage equation to
**            compute head loss and its gradient as a function
**            flow.
**--------------------------------------------------------------
*/
{
    n = 1.0 / n;
    *g = n * c * pow(fabs(q), n - 1.0);
    
    // Use linear head loss function for small gradient
    if (*g < RQtol)
    {
        *g = RQtol / n;
        *h = (*g) * q;
    }            

    // Otherwise use normal leakage head loss function
    else *h = (*g) * q / n;
    
    // Prevent leakage from going negative
    addlowerbarrier(q, h, g);
}

void addlowerbarrier(double q, double* h, double* g)
/*
**--------------------------------------------------------------------
**  Input:   q = current flow rate
**  Output:  h = head loss value
**           g = head loss gradient value
**  Purpose: adds a head loss barrier to prevent flow from falling
**           below 0.
**--------------------------------------------------------------------
*/
{
    double a = 1.e9 * q;
    double b = sqrt(a*a + 1.e-6);
    *h += (a - b) / 2.;
    *g += (1.e9 / 2.) * ( 1.0 - a / b);
}
