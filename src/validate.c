/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       validate.c
 Description:  validates project data
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 09/28/2023
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> 

#include "types.h"
#include "funcs.h"
#include "text.h"

// Exported functions
int  validateproject(Project *pr);
void reindextanks(Project *pr);

int validatetanks(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if successful, 0 if not
**  Purpose: checks for valid tank levels.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int i, j, n, result = 1, levelerr;
    char errmsg[MAXMSG+1] = "";
    Stank *tank;
    Scurve *curve;
    
    for (j = 1; j <= net->Ntanks; j++)
    {
        tank = &net->Tank[j];
        if (tank->A == 0.0) continue;  // Skip reservoirs

        // Check for valid lower/upper tank levels
        levelerr = 0;
        if (tank->H0 > tank->Hmax ||
            tank->Hmin > tank->Hmax ||
            tank->H0 < tank->Hmin
        ) levelerr = 1;

        // Check that tank heights are within volume curve
        i = tank->Vcurve;
        if (i > 0)
        {
            curve = &net->Curve[i];
            n = curve->Npts - 1;
            if (tank->Hmin * pr->Ucf[ELEV] < curve->X[0] ||
                tank->Hmax * pr->Ucf[ELEV]> curve->X[n])
            {
                levelerr = 1;
            }
        }
        // Report error in levels if found
        if (levelerr)
        {
            sprintf(pr->Msg, "Error 225: %s node %s", geterrmsg(225, errmsg),
                    net->Node[tank->Node].ID);
            writeline(pr, pr->Msg);
            result = 0;
        }
    }
    return result;
}

int validatepatterns(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if successful, 0 if not
**  Purpose: checks if time patterns have data.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int j, result = 1;
    char errmsg[MAXMSG+1] = "";

    if (pr->network.Pattern != NULL)
    {
        for (j = 0; j <= pr->network.Npats; j++)
        {
            if (pr->network.Pattern[j].Length == 0)
            {
                sprintf(pr->Msg, "Error 232: %s %s", geterrmsg(232, errmsg),
                    pr->network.Pattern[j].ID);
                writeline(pr, pr->Msg);
                result = 0;
            }
        }
    }
    return result;            
}

int validatecurves(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if successful, 0 if not
**  Purpose: checks if data curves have data.
**-------------------------------------------------------------------
*/
{
    int i, j, npts, result = 1;
    char errmsg[MAXMSG+1] = "";
    Scurve *curve;

    if (pr->network.Curve != NULL)
    {
        for (j = 1; j <= pr->network.Ncurves; j++)
        {
            // Check that curve has data
            curve = &pr->network.Curve[j];
            npts = curve->Npts;
            if (npts == 0)
            {
                sprintf(pr->Msg, "Error 231: %s %s", geterrmsg(231, errmsg),
                    curve->ID);
                writeline(pr, pr->Msg);
                result = 0;
            }
            
            // Check that x values are increasing
            for (i = 1; i < npts; i++)
            {
                if (curve->X[i-1] >= curve->X[i])
                {
                    sprintf(pr->Msg, "Error 230: %s %s", geterrmsg(230, errmsg),
                        curve->ID);
                    writeline(pr, pr->Msg);
                    result = 0;
                    break;
                }
            }       
        }
    }
    return result;
}

int powercurve(double h0, double h1, double h2, double q1, double q2,
               double *a, double *b, double *c)
/*
**---------------------------------------------------------
**  Input:   h0 = shutoff head
**           h1 = design head
**           h2 = head at max. flow
**           q1 = design flow
**           q2 = max. flow
**  Output:  *a, *b, *c = pump curve coeffs. (H = a-bQ^c),
**           Returns 1 if sucessful, 0 otherwise.
**  Purpose: computes coeffs. for pump curve
**----------------------------------------------------------
*/
{
    double h4, h5;

    if (h0 < TINY || h0 - h1 < TINY || h1 - h2 < TINY ||
        q1 < TINY || q2 - q1 < TINY
       ) return 0;
    *a = h0;
    h4 = h0 - h1;
    h5 = h0 - h2;
    *c = log(h5 / h4) / log(q2 / q1);
    if (*c <= 0.0 || *c > 20.0) return 0;
    *b = -h4 / pow(q1, *c);
    if (*b >= 0.0) return 0;
    return 1;
}

int findpumpparams(Project *pr, int pumpindex)
/*
**-------------------------------------------------------------
**  Input:   pumpindex = index of a pump
**  Output:  returns error code
**  Purpose: computes & checks a pump's head curve coefficients
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Spump  *pump;
    Scurve *curve;

    int m;
    int curveindex;
    int npts = 0;
    int errcode = 0;
    double a, b, c, h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;

    pump = &net->Pump[pumpindex];
    if (pump->Ptype == CONST_HP)  // Constant Hp pump
    {
        pump->H0 = 0.0;
        pump->R = -8.814 * net->Link[pump->Link].Km;
        pump->N = -1.0;
        pump->Hmax = BIG; // No head limit
        pump->Qmax = BIG; // No flow limit
        pump->Q0 = 1.0;   // Init. flow = 1 cfs
        return 0;
    }

    else if (pump->Ptype == NOCURVE) // Pump curve specified
    {
        curveindex = pump->Hcurve;
        if (curveindex == 0) return 226;
        curve = &net->Curve[curveindex];
        curve->Type = PUMP_CURVE;
        npts = curve->Npts;

        // Generic power function curve
        if (npts == 1)
        {
            pump->Ptype = POWER_FUNC;
            q1 = curve->X[0];
            h1 = curve->Y[0];
            h0 = 1.33334 * h1;
            q2 = 2.0 * q1;
            h2 = 0.0;
        }

        // 3 point curve with shutoff head
        else if (npts == 3 && curve->X[0] == 0.0)
        {
            pump->Ptype = POWER_FUNC;
            h0 = curve->Y[0];
            q1 = curve->X[1];
            h1 = curve->Y[1];
            q2 = curve->X[2];
            h2 = curve->Y[2];
        }

        // Custom pump curve
        else
        {
            pump->Ptype = CUSTOM;
            for (m = 1; m < npts; m++)
            {
                if (curve->Y[m] >= curve->Y[m - 1])
                {
                    pump->Ptype = NOCURVE;
                    return 227;
                }
            }
            pump->Qmax = curve->X[npts - 1];
            pump->Q0 = (curve->X[0] + pump->Qmax) / 2.0;
            pump->Hmax = curve->Y[0];
        }

        // Compute shape factors & limits of power function curves
        if (pump->Ptype == POWER_FUNC)
        {
            if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c))
            {
                pump->Ptype = NOCURVE;
                return 227;
            }
            else
            {
                pump->H0 = -a;
                pump->R = -b;
                pump->N = c;
                pump->Q0 = q1;
                pump->Qmax = pow((-a / b), (1.0 / c));
                pump->Hmax = h0;
            }
        }

        // Convert units of pump coefficients        
        if (pump->Ptype == POWER_FUNC)
        {
            pump->H0 /= pr->Ucf[HEAD];
            pump->R *= (pow(pr->Ucf[FLOW], pump->N) / pr->Ucf[HEAD]);
        }
        pump->R /= pr->Ucf[POWER];
        pump->Q0 /= pr->Ucf[FLOW];
        pump->Qmax /= pr->Ucf[FLOW];
        pump->Hmax /= pr->Ucf[HEAD];
    }
    return 0;
}
 
int validatepumps(Project *pr)
/*
**-------------------------------------------------------------------
**  Input:   none
**  Output:  returns 1 if successful, 0 if not
**  Purpose: checks if pumps assigned pump curves.
**-------------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int i, k, errcode, result = 1;
    char errmsg[MAXMSG+1] = "";
    Spump  *pump;
    
    for (i = 1; i <= net->Npumps; i++)
    {
        // Check if pump has neither a head curve nor power setting
        pump = &net->Pump[i];
        k = pump->Link;
        if (net->Link[k].Km == 0.0 && pump->Hcurve <= 0)
        {
            sprintf(pr->Msg, "Error 226: %s %s",
                geterrmsg(226, errmsg), net->Link[k].ID);
            writeline(pr, pr->Msg);
            result = 0;
        }
        
        // Compute & check pump's head curve coefficients
        else
        {
            errcode = findpumpparams(pr, i);
            if (errcode)
            {
                sprintf(pr->Msg, "Error %d: %s %s",
                    errcode, geterrmsg(errcode, errmsg), net->Link[k].ID);
                writeline(pr, pr->Msg);
                result = 0;
            }
        }
    }
    return result;
}    

int validateproject(Project *pr)
/*
 *--------------------------------------------------------------
 *  Input:   none
 *  Output:  returns error code
 *  Purpose: checks for valid network data.
 *--------------------------------------------------------------
*/
{
    int errcode = 0;
    if (pr->network.Nnodes < 2) return 223;
    if (pr->network.Ntanks == 0) return 224;
    if (!validatetanks(pr)) errcode = 110;
    if (!validatepumps(pr)) errcode = 110;
    if (!validatepatterns(pr)) errcode = 110;
    if (!validatecurves(pr)) errcode = 110;
    return errcode;
}

void reindextanks(Project *pr)
/*
 *--------------------------------------------------------------
 *  Input:   none
 *  Output:  none
 *  Purpose: adjusts tank node indexes when the number of
 *           junctions created from an input file is less than
 *           the total number of junction lines in the file.
 *--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Quality *qual   = &pr->quality;
    Scontrol *control;
    int i, j, ndiff, n1, n2, size;
    
    // ndiff = # unused entries in Node array before first tank node
    ndiff = parser->MaxJuncs - net->Njuncs;
    if (ndiff > 0)
    {   
        for (i = 1; i <= net->Ntanks; ++i)
        {
            // n1 is current tank index in Node array, n2 is adjusted index
            n1 = net->Tank[i].Node;
            n2 = n1 - ndiff;
            
            // Update the tank node's hash table entry
            hashtable_update(net->NodeHashTable, net->Node[n1].ID, n2);
            
            // Update the tank's node index
            net->Tank[i].Node = n2;
            
            // Re-position tank node in Node array
            net->Node[n2] = net->Node[n1];
            
            // Replace all references to old tank node index with new one
            for (j = 1; j <= net->Nlinks; ++j)
            {
                if (net->Link[j].N1 == n1) net->Link[j].N1 = n2;
                if (net->Link[j].N2 == n1) net->Link[j].N2 = n2;
            }
            for (j = 1; j <= net->Ncontrols; ++j)
            {
                control = &net->Control[j];
                if (control->Node == n1) control->Node = n2;
            }
            adjusttankrules(pr, -ndiff);
            if (qual->TraceNode == n1) qual->TraceNode = n2;
        }
        
        // Reallocate the Node array (shouldn't fail as new size < old size)
        parser->MaxJuncs = net->Njuncs;
        parser->MaxNodes = net->Njuncs + net->Ntanks;
        size = (net->Nnodes + 2) * sizeof(Snode);
        net->Node = (Snode *)realloc(net->Node, size);
    }
}    
    
