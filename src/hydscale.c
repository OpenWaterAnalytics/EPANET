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
