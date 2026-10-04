/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       hydscale.c
 Description:  manages numerical scaling used by the hydraulic solver
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 10/04/2026
 ******************************************************************************
*/

#include <math.h>

#include "types.h"
#include "funcs.h"


void inithydraulicscaling(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes the hydraulic solver's numerical scales
**----------------------------------------------------------------
**  The current scales intentionally reproduce EPANET's legacy internal
**  hydraulic representation exactly: one solver head unit is one internal
**  foot of head and one solver flow unit is one internal cfs. Later stages
**  of the unit-independence migration will change these scale values without
**  changing the public input/output unit contract.
*/
{
    ShydScale *scale = &pr->hydraul.SolverScale;

    scale->Head = 1.0;
    scale->Flow = 1.0;
}


void loadhydraulicsolverstate(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: copies dimensional hydraulic state into solver state
**----------------------------------------------------------------
**  This is the dimensional -> numerical boundary for hydraulic state.
**  Keep unit conversion here rather than scattering it through GGA code.
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    int i;

    for (i = 1; i <= net->Nnodes; i++)
    {
        hyd->SolverState.NodeHead[i] =
            hydheadtosolver(pr, hyd->NodeHead[i]);
        hyd->SolverState.NodeDemand[i] =
            hydflowtosolver(pr, hyd->NodeDemand[i]);
        hyd->SolverState.FullDemand[i] =
            hydflowtosolver(pr, hyd->FullDemand[i]);
        hyd->SolverState.DemandFlow[i] =
            hydflowtosolver(pr, hyd->DemandFlow[i]);
        hyd->SolverState.EmitterFlow[i] =
            hydflowtosolver(pr, hyd->EmitterFlow[i]);
        hyd->SolverState.LeakageFlow[i] =
            hydflowtosolver(pr, hyd->LeakageFlow[i]);
    }
    for (i = 1; i <= net->Nlinks; i++)
    {
        hyd->SolverState.LinkFlow[i] =
            hydflowtosolver(pr, hyd->LinkFlow[i]);
    }
}


void savehydraulicsolverstate(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: copies solver hydraulic state back to dimensional state
**----------------------------------------------------------------
**  This is the numerical -> dimensional boundary after a hydraulic solve.
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    int i;

    for (i = 1; i <= net->Nnodes; i++)
    {
        hyd->NodeHead[i] =
            hydheadfromsolver(pr, hyd->SolverState.NodeHead[i]);
        hyd->NodeDemand[i] =
            hydflowfromsolver(pr, hyd->SolverState.NodeDemand[i]);
        hyd->DemandFlow[i] =
            hydflowfromsolver(pr, hyd->SolverState.DemandFlow[i]);
        hyd->EmitterFlow[i] =
            hydflowfromsolver(pr, hyd->SolverState.EmitterFlow[i]);
        hyd->LeakageFlow[i] =
            hydflowfromsolver(pr, hyd->SolverState.LeakageFlow[i]);
    }
    for (i = 1; i <= net->Nlinks; i++)
    {
        hyd->LinkFlow[i] =
            hydflowfromsolver(pr, hyd->SolverState.LinkFlow[i]);
    }
}


double hydheadtosolver(Project *pr, double head)
/*
**----------------------------------------------------------------
**  Purpose: converts dimensional internal head to solver head
**----------------------------------------------------------------
*/
{
    return head / pr->hydraul.SolverScale.Head;
}


double hydheadfromsolver(Project *pr, double head)
/*
**----------------------------------------------------------------
**  Purpose: converts solver head to dimensional internal head
**----------------------------------------------------------------
*/
{
    return head * pr->hydraul.SolverScale.Head;
}


double hydflowtosolver(Project *pr, double flow)
/*
**----------------------------------------------------------------
**  Purpose: converts dimensional internal flow to solver flow
**----------------------------------------------------------------
*/
{
    return flow / pr->hydraul.SolverScale.Flow;
}


double hydflowfromsolver(Project *pr, double flow)
/*
**----------------------------------------------------------------
**  Purpose: converts solver flow to dimensional internal flow
**----------------------------------------------------------------
*/
{
    return flow * pr->hydraul.SolverScale.Flow;
}


double hydresistancetosolver(Project *pr, double resistance, double exponent)
/*
**----------------------------------------------------------------
**  Purpose: converts a dimensional headloss resistance coefficient
**           for H = R * Q^exponent into solver units
**----------------------------------------------------------------
**  If H = Hs*H' and Q = Qs*Q', then:
**
**      H' = [R * Qs^exponent / Hs] * Q'^exponent
**
**  where Hs and Qs are the configured head and flow scales.
*/
{
    ShydScale *scale = &pr->hydraul.SolverScale;

    return resistance * pow(scale->Flow, exponent) / scale->Head;
}


double hydconductancetosolver(Project *pr, double conductance)
/*
**----------------------------------------------------------------
**  Purpose: converts a dimensional flow/head conductance into
**           solver units
**----------------------------------------------------------------
**  Conductance is the reciprocal of a linear headloss gradient.
**  If Q = C * H, then Q' = [C * Hs / Qs] * H'.
*/
{
    ShydScale *scale = &pr->hydraul.SolverScale;

    return conductance * scale->Head / scale->Flow;
}


double hydminorlosstosolver(Project *pr, double resistance)
/*
**----------------------------------------------------------------
**  Purpose: converts a quadratic minor-loss coefficient into
**           solver units
**----------------------------------------------------------------
*/
{
    return hydresistancetosolver(pr, resistance, 2.0);
}
