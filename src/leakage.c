/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       leakage.c
 Description:  models additional nodal demands due to pipe leaks.
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 09/10/2025
 ******************************************************************************
*/
/*
This module uses the FAVAD (Fixed and Variable Discharge) equation to model
leaky pipes:
  
  Q = Co * L * (Ao + m * H) * sqrt(H)
  
where Q = pipe leak flow rate, Co = an orifice coefficient (= 0.6*sqrt(2g)),
L = pipe length, Ao = initial area of leak per unit of pipe length,
m = change in leak area per unit of pressure head, and H = pressure head.

The inverted form of this equation is used to model the leakage demand from
a pipe's end node using a pair of equivalent emitters as follows:

  H = Cfa * Qfa^2
  H = Cva * Qva^(2/3)
  
where Qfa = fixed area node leakage rate, Qva = variable area node leakage rate,
Cfa = 1 / SUM(Co*(L/2)*Ao)^2, Cva = 1 / SUM(Co*(L/2)*m)^2/3, and
SUM(x) is the summation of x over all pipes connected to the node.

In implementing this model, the pipe property "LeakArea" represents Ao in
sq. mm per 100 units of pipe length and "LeakExpan" represents m in sq. mm
per unit of pressure head.

*/
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "funcs.h"

// Exported functions (declared in funcs.h)
//int     openleakage(Project *);
//void    closeleakage(Project *);
//double  findlinkleakage(Project *, int);
//void    leakagecoeffs(Project *);
//double  leakageflowchange(Project *, int);
//int     leakagehasconverged(Project *);

// Local functions
static int   check_for_leakage(Project *pr);
static int   create_leakage_objects(Project *pr);
static void  convert_pipe_to_node_leakage(Project *pr);
static void  init_node_leakage(Project *pr);
static int   leakage_headloss(Project* pr, int i, double *hfa,
             double *gfa, double *hva, double *gva);
static void  eval_leak_headloss(double q, double c,
             double n, double *hloss, double *hgrad);
static void  add_lower_barrier(double q, double *hloss, double *hgrad);


int openleakage(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  returns an error code
**   Purpose: opens the pipe leakage modeling system
**-------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;

    int err;
    
    // Check if project includes leakage
    closeleakage(pr);
    hyd->HasLeakage = check_for_leakage(pr);
    if (!hyd->HasLeakage) return 0;
    
    // Allocate memory for leakage data objects
    err = create_leakage_objects(pr);
    if (err > 0) return err;
    
    // Convert pipe leakage coeffs. to node coeffs.
    convert_pipe_to_node_leakage(pr);
    init_node_leakage(pr);
    return 0;
}

    
int check_for_leakage(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  returns TRUE if any pipes can leak, FALSE otherwise
**   Purpose: checks if any pipes can leak.
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int i;
    Slink *link;
    
    for (i = 1; i <= net->Nlinks; i++)
    {
        // Only pipes have leakage
        link = &net->Link[i];
        if (link->Type > PIPE) continue;
        if (link->LeakArea > 0.0 || link->LeakExpan > 0.0) return TRUE;
    }
    return FALSE;
}    

    
int create_leakage_objects(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  returns an error code
**   Purpose: allocates an array of node leakage objects.
**-------------------------------------------------------------
*/
{    
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    int i;
    
    hyd->Leakage = (Sleakage *)calloc(net->Njuncs + 1, sizeof(Sleakage));
    if (hyd->Leakage == NULL) return 101;
    for (i = 1; i <= net->Njuncs; i++)
    {
        hyd->Leakage[i].cfa = 0.0;
        hyd->Leakage[i].cva = 0.0;
        hyd->Leakage[i].qfa = 0.0;
        hyd->Leakage[i].qva = 0.0;
    }
    return 0;
}

void convert_pipe_to_node_leakage(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: converts pipe leakage parameters into node leakage
**            coefficients.
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

    // Orifice coeff. with conversion from sq. mm to sq. m
    c_orif = 4.8149866 * 1.e-6;
    
    // Examine each link
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

        // Get pipe's fixed and variable area leak coeffs.
        if (link->LeakArea == 0.0 && link->LeakExpan == 0.0) continue;
        c_area = c_orif * link->LeakArea / SQR(MperFT);
        c_expan = c_orif * link->LeakExpan;

        // Adjust for number of 100-ft pipe sections
        len = link->Len * pr->Ucf[LENGTH] / 100.;
        if (node1->Type == JUNCTION && node2->Type == JUNCTION)
        {
            len *= 0.5;
        }
        c_area *= len;
        c_expan *= len;
        
        // Add these coeffs. to pipe's end nodes
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
}    
    
void init_node_leakage(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: initializes node leakage coeffs. and flows.
**-------------------------------------------------------------
*/
{    
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int i;
    double c_area, c_expan;

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

void closeleakage(Project *pr)
/*-------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: frees memory for nodal leakage objects.
**-------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    if (hyd->Leakage) free(hyd->Leakage);
    hyd->Leakage = NULL;
    hyd->HasLeakage = FALSE;
}
   
double findlinkleakage(Project *pr, int i)
/*-------------------------------------------------------------
**   Input:   i = link index
**   Output:  returns link leakage flow (cfs)
**   Purpose: computes leakage flow from link i at current
**            hydraulic solution.
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
    if (n2 > net->Njuncs) q1 *= 2.0;
    if (n1 > net->Njuncs) q2 *= 2.0;
    return q1 + q2;
}    

void leakagecoeffs(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: computes coeffs. of the linearized hydraulic eqns.
**            contributed by node leakages.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Smatrix *sm = &hyd->smatrix;

    int    i, row;
    double hfa,    // head loss producing current fixed area leakage
           gfa,    // gradient of fixed area head loss
           hva,    // head loss producing current variable area leakage
           gva;    // gradient of variable area head loss
    
    Snode* node;
    
    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions that don't leak
        node = &net->Node[i];
        if (!leakage_headloss(pr, i, &hfa, &gfa, &hva, &gva)) continue;

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

double leakageflowchange(Project *pr, int i)
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

    double  hfa, gfa, hva, gva;  // same as defined in leakage_solvercoeffs()
    double  h, dqfa, dqva;       // pressure head, change in leakage flows
    
    // Find the head loss and gradient of the inverted leakage
    // equation for both fixed and variable area leakage at the
    // current leakage flow rates
    if (!leakage_headloss(pr, i, &hfa, &gfa, &hva, &gva)) return 0.0;
    
    // Pressure head using latest head solution
    h = hyd->NodeHead[i] - net->Node[i].El;

    // GGA flow update formula for fixed area leakage
    dqfa = 0.0;
    if (gfa > 0.0)
    {
        dqfa = (hfa - h) / gfa * hyd->RelaxFactor;
        hyd->Leakage[i].qfa -= dqfa;
    }

    // GGA flow update formula for variable area leakage
    dqva = 0.0;
    if (gva > 0.0)
    {
        dqva = (hva - h) / gva * hyd->RelaxFactor;
        hyd->Leakage[i].qva -= dqva;
    }

    // New leakage flow at the node 
    hyd->LeakageFlow[i] = hyd->Leakage[i].qfa + hyd->Leakage[i].qva;
    return dqfa + dqva;
}

int leakagehasconverged(Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns TRUE if leakage calculations converged,
**            FALSE if not
**   Purpose: checks if leakage calculations have converged.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    
    int i;
    double h, qref, qtest;
    const double QTOL = 0.0001;  // 0.0001 cfs ~= 0.005 gpm ~= 0.2 lpm)

    for (i = 1; i <= net->Njuncs; i++)
    {
        // Skip junctions that don't leak
        if (hyd->Leakage[i].cfa == 0 && hyd->Leakage[i].cva == 0) continue;
        
        // Evaluate node's pressure head
        h = hyd->NodeHead[i] - net->Node[i].El;
        
        // Directly compute a reference leakage at this pressure head
        qref = 0.0;
        if ( h > 0.0)
        {
            // Contribution from pipes with fixed area leaks
            if (hyd->Leakage[i].cfa > 0.0)
                qref = sqrt(h / hyd->Leakage[i].cfa);
            // Contribution from pipes with variable area leaks
            if (hyd->Leakage[i].cva > 0.0)
                qref += pow((h / hyd->Leakage[i].cva), 1.5);
        }
        
        // Compare reference leakage to solution leakage
        qtest = hyd->Leakage[i].qfa + hyd->Leakage[i].qva;        
        if (fabs(qref - qtest) > QTOL) return FALSE;
    }
    return TRUE;
}

int leakage_headloss(Project* pr, int i, double *hfa, double *gfa,
    double *hva, double *gva)
/*
**--------------------------------------------------------------
**   Input:   i = node index
**   Output:  hfa = fixed area leak head loss (ft)
**            gfa = gradient of fixed area head loss (ft/cfs)
**            hva = variable area leak head loss (ft)
**            gva = gradient of variable area head loss (ft/cfs)
**            returns TRUE if node has leakage, FALSE otherwise
**   Purpose: finds head loss and its gradient for a node's
**            leakage as a function of leakage flow.
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
        eval_leak_headloss(hyd->Leakage[i].qfa, hyd->Leakage[i].cfa,
                           0.5, hfa, gfa);
    if (hyd->Leakage[i].cva == 0.0)
    {
        *hva = 0.0;
        *gva = 0.0;
    }
    else
        eval_leak_headloss(hyd->Leakage[i].qva, hyd->Leakage[i].cva,
                           1.5, hva, gva);
    return TRUE;
}

void eval_leak_headloss(double q, double c, double n,
    double *hloss, double *hgrad)
/*
**--------------------------------------------------------------
**   Input:   q = leakage flow rate (cfs)
**            c = leakage head loss coefficient
**            n = leakage head loss exponent
**   Output:  hloss = leakage head loss (ft)
**            hgrad = gradient of leakage head loss (ft/cfs)
**   Purpose: evaluates inverted form of leakage equation to
**            compute head loss and its gradient as a function
**            flow.
**
**   Note:  Inverted leakage equation is:
**            hloss = c * q ^ (1/n)
**--------------------------------------------------------------
*/
{
    n = 1.0 / n;
    *hgrad = n * c * pow(fabs(q), n - 1.0);
    *hloss = (*hgrad) * q / n;
    
    // Prevent leakage from going negative
    add_lower_barrier(q, hloss, hgrad);
}

void add_lower_barrier(double q, double* hloss, double* hgrad)
/*
**--------------------------------------------------------------------
**  Input:   q = current flow rate
**  Output:  hloss = head loss value
**           hgrad = head loss gradient value
**  Purpose: adds a head loss barrier to prevent flow from falling
**           below 0.
**--------------------------------------------------------------------
*/
{
    double a = 1.e9 * q;
    double b = sqrt(a*a + 1.e-6);
    *hloss += (a - b) / 2.;
    *hgrad += (1.e9 / 2.) * ( 1.0 - a / b);
}
