/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       flowbalance.c
 Description:  computes components of network's flow balance
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 06/26/2024
 ******************************************************************************
*/

#include "types.h"

// Exported functions (declared in funcs.h)
//void startflowbalance(Project *);
//void updateflowbalance(Project *, long);
//void endflowbalance(Project *);

void startflowbalance(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes components of the network's flow balance.
**-------------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    hyd->FlowBalance.totalInflow = 0.0;
    hyd->FlowBalance.totalOutflow = 0.0;
    hyd->FlowBalance.consumerDemand = 0.0;
    hyd->FlowBalance.emitterDemand = 0.0;
    hyd->FlowBalance.leakageDemand = 0.0;
    hyd->FlowBalance.deficitDemand = 0.0;
    hyd->FlowBalance.storageDemand = 0.0;
    hyd->FlowBalance.ratio = 0.0;
}    

void updateflowbalance(Project *pr, long hstep)
/*
**-------------------------------------------------------------------
**  Input:   hstep = time step (sec)
**  Output:  none
**  Purpose: updates components of the system flow balance.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;

    int i, j;
    double v, dt, deficit, fullDemand;
    SflowBalance flowBalance;
    
    // Determine current time interval in seconds
    if (time->Dur == 0) dt = 1.0;
    else if (time->Htime < time->Dur)
    {
        dt = (double) hstep;
    }
    else return;
    
    // Initialize local flow balance
    flowBalance.totalInflow = 0.0;
    flowBalance.totalOutflow = 0.0;
    flowBalance.consumerDemand = 0.0;
    flowBalance.emitterDemand = 0.0;
    flowBalance.leakageDemand = 0.0;
    flowBalance.deficitDemand = 0.0;
    flowBalance.storageDemand = 0.0;
    fullDemand = 0.0;
    
    // Initialize leakage loss
    hyd->LeakageLoss = 0.0;

    // Examine each junction node
    for (i = 1; i <= net->Njuncs; i++) 
    {
        // Accumulate consumer demand flow
        v = hyd->DemandFlow[i];
        if (v < 0.0)
            flowBalance.totalInflow += (-v);
        else
        {
            fullDemand += hyd->FullDemand[i];
            flowBalance.consumerDemand += v;
            flowBalance.totalOutflow += v;
        }
        
        // Accumulate emitter and leakage flow
        v = hyd->EmitterFlow[i];
        flowBalance.emitterDemand += v;
        flowBalance.totalOutflow += v;
        v = hyd->LeakageFlow[i];
        flowBalance.leakageDemand += v;
        flowBalance.totalOutflow += v;

        // Accumulate demand deficit flow
        if (hyd->DemandModel == PDA && hyd->FullDemand[i] > 0.0)
        {
            deficit = hyd->FullDemand[i] - hyd->DemandFlow[i];
            if (deficit > 0.0)
                flowBalance.deficitDemand += deficit;
        }
    }
    
    // Examine each tank/reservoir node
    for (j = 1; j <= net->Ntanks; j++)
    {
        i = net->Tank[j].Node;
        v = hyd->NodeDemand[i];

        // For a reservoir node
        if (net->Tank[j].A == 0.0)
        {
            if (v >= 0.0)
                flowBalance.totalOutflow += v;
            else
                flowBalance.totalInflow += (-v);
        }
        
        // For tank
        else
            flowBalance.storageDemand += v;
    }
    
    // Find % leakage for current period
    v = flowBalance.totalInflow;
    if (flowBalance.storageDemand < 0.0) v += (-flowBalance.storageDemand);
    if (v > 0.0)
        hyd->LeakageLoss = flowBalance.leakageDemand / v * 100.0;

    // Update flow balance for entire run
    hyd->FlowBalance.totalInflow += flowBalance.totalInflow * dt;
    hyd->FlowBalance.totalOutflow += flowBalance.totalOutflow * dt;
    hyd->FlowBalance.consumerDemand += flowBalance.consumerDemand * dt;
    hyd->FlowBalance.emitterDemand += flowBalance.emitterDemand * dt;
    hyd->FlowBalance.leakageDemand += flowBalance.leakageDemand * dt;
    hyd->FlowBalance.deficitDemand += flowBalance.deficitDemand * dt;
    hyd->FlowBalance.storageDemand += flowBalance.storageDemand * dt;
}

void endflowbalance(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: finalizes components of the system flow balance.
**-------------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    Times   *time = &pr->times;
    
    double seconds, qin, qout, qstor, r;

    if (time->Htime > 0)
        seconds = time->Htime;
    else
        seconds = 1.0;        
    hyd->FlowBalance.totalInflow /= seconds;
    hyd->FlowBalance.totalOutflow /= seconds;
    hyd->FlowBalance.consumerDemand /= seconds;
    hyd->FlowBalance.emitterDemand /= seconds;
    hyd->FlowBalance.leakageDemand /= seconds;
    hyd->FlowBalance.deficitDemand /= seconds;
    hyd->FlowBalance.storageDemand /= seconds;
    
    qin = hyd->FlowBalance.totalInflow;
    qout = hyd->FlowBalance.totalOutflow;
    qstor = hyd->FlowBalance.storageDemand;
    if (qstor > 0.0)
        qout += qstor;
    else
        qin -= qstor;
    if (qin == qout)
        r = 1.0;
    else if (qin > 0.0)
        r = qout / qin;
    else
        r = 0.0;
    hyd->FlowBalance.ratio = r;
}
