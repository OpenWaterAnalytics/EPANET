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
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "funcs.h"


static void includecharacteristic(double value, double *magnitude)
/* Adds a finite, non-sentinel dimensional magnitude to a scale estimate. */
{
    value = fabs(value);
    if (isfinite(value) && value < BIG && value > *magnitude)
    {
        *magnitude = value;
    }
}


static double decadescale(double magnitude)
/* Returns the lower power-of-ten scale for a dimensional magnitude. */
{
    double scale;

    if (!isfinite(magnitude) || magnitude <= 0.0) return 1.0;
    scale = pow(10.0, floor(log10(magnitude)));
    if (!isfinite(scale) || scale <= 0.0) return 1.0;
    return scale;
}


int allochydraulicsolvermodel(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: allocates storage for the compiled hydraulic model
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    ShydSolverModel *model = &pr->hydraul.SolverModel;
    int errcode = 0;

    model->NodeElevation =
        (double *) calloc(net->Nnodes + 1, sizeof(double));
    model->LinkResistance =
        (double *) calloc(net->Nlinks + 1, sizeof(double));
    model->LinkMinorLoss =
        (double *) calloc(net->Nlinks + 1, sizeof(double));
    model->LinkViscosityFlow =
        (double *) calloc(net->Nlinks + 1, sizeof(double));
    model->LinkSetting =
        (double *) calloc(net->Nlinks + 1, sizeof(double));
    model->LinkDynamicLoss =
        (double *) calloc(net->Nlinks + 1, sizeof(double));
    model->Curve =
        (ShydSolverCurve *) calloc(net->Ncurves + 1, sizeof(ShydSolverCurve));
    model->CurveCapacity = net->Ncurves;
    model->CurveHeadScale = 0.0;
    model->CurveFlowScale = 0.0;
    model->CurveHeadUcf = 0.0;
    model->CurveFlowUcf = 0.0;

    ERRCODE(MEMCHECK(model->NodeElevation));
    ERRCODE(MEMCHECK(model->LinkResistance));
    ERRCODE(MEMCHECK(model->LinkMinorLoss));
    ERRCODE(MEMCHECK(model->LinkViscosityFlow));
    ERRCODE(MEMCHECK(model->LinkSetting));
    ERRCODE(MEMCHECK(model->LinkDynamicLoss));
    ERRCODE(MEMCHECK(model->Curve));
    return errcode;
}


void freehydraulicsolvermodel(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: frees storage for the compiled hydraulic model
**----------------------------------------------------------------
*/
{
    ShydSolverModel *model = &pr->hydraul.SolverModel;
    int i;

    if (model->Curve != NULL)
    {
        for (i = 1; i <= model->CurveCapacity; i++)
        {
            free(model->Curve[i].X);
            free(model->Curve[i].H0);
            free(model->Curve[i].R);
        }
    }
    free(model->Curve);
    free(model->NodeElevation);
    free(model->LinkResistance);
    free(model->LinkMinorLoss);
    free(model->LinkViscosityFlow);
    free(model->LinkSetting);
    free(model->LinkDynamicLoss);
    model->NodeElevation = NULL;
    model->LinkResistance = NULL;
    model->LinkMinorLoss = NULL;
    model->LinkViscosityFlow = NULL;
    model->LinkSetting = NULL;
    model->LinkDynamicLoss = NULL;
    model->Curve = NULL;
    model->CurveCapacity = 0;
    model->CurveHeadScale = 0.0;
    model->CurveFlowScale = 0.0;
    model->CurveHeadUcf = 0.0;
    model->CurveFlowUcf = 0.0;
}


void compilehydraulicsolverglobals(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: compiles global dimensional hydraulic inputs into solver units
**----------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    ShydSolverModel *model = &hyd->SolverModel;

    if (model->NodeElevation == NULL) return;

    model->Htol = hydheadtosolver(pr, hyd->Htol);
    model->Qtol = hydflowtosolver(pr, hyd->Qtol);
    model->RQtol = hydresistancetosolver(pr, hyd->RQtol, 1.0);
    model->Pmin = hydheadtosolver(pr, hyd->Pmin);
    model->Preq = hydheadtosolver(pr, hyd->Preq);
    model->PdaPressureRange = hydheadtosolver(pr,
        MAX((hyd->Preq - hyd->Pmin), MINPDIFF));
    model->FlowChangeLimit = hydflowtosolver(pr, hyd->FlowChangeLimit);
    model->HeadErrorLimit = hydheadtosolver(pr, hyd->HeadErrorLimit);
    model->TinyFlow = hydflowtosolver(pr, TINY);
    model->LeakageFlowTolerance = hydflowtosolver(pr, 0.0001);
    model->BigHead = hydheadtosolver(pr, BIG);
    model->TinyGradient = hydresistancetosolver(pr, TINY, 1.0);
    model->SmallGradient = hydresistancetosolver(pr, CSMALL, 1.0);
    model->BigGradient = hydresistancetosolver(pr, CBIG, 1.0);
    model->BigConductance = hydconductancetosolver(pr, CBIG);
    model->SmallConductance = hydconductancetosolver(pr, 1.0 / CBIG);
}


void compilehydraulicsolvernode(Project *pr, int i)
/*
**----------------------------------------------------------------
**  Purpose: refreshes one node's compiled solver elevation
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    ShydSolverModel *model = &pr->hydraul.SolverModel;

    int k;

    if (model->NodeElevation == NULL) return;
    if (i < 1 || i > net->Nnodes) return;
    model->NodeElevation[i] = hydheadtosolver(pr, net->Node[i].El);

    // PRV/PSV compiled settings are complete target grades so the GGA retains
    // the legacy one-conversion arithmetic. Refresh any target that depends on
    // this node when its elevation changes through the Toolkit.
    if (model->LinkSetting == NULL) return;
    for (k = 1; k <= net->Nlinks; k++)
    {
        Slink *link = &net->Link[k];
        if ((link->Type == PRV && link->N2 == i) ||
            (link->Type == PSV && link->N1 == i))
        {
            compilehydraulicsolversetting(pr, k);
        }
    }
}


void compilehydraulicsolversetting(Project *pr, int i)
/*
**----------------------------------------------------------------
**  Purpose: compiles one link's dynamic setting into solver units
**----------------------------------------------------------------
**  LinkSetting remains dimensional for compatibility/reporting. This cache
**  is refreshed at each mutation boundary so iterative hydraulic code never
**  has to convert a dynamic setting repeatedly. TCV and PCV settings are
**  dimensionless positions/coefficients; for those links LinkDynamicLoss
**  stores the setting-dependent loss coefficient consumed by the GGA.
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    ShydSolverModel *model = &hyd->SolverModel;
    Slink *link;
    double setting, km;

    if (model->LinkSetting == NULL || model->LinkDynamicLoss == NULL) return;
    if (i < 1 || i > net->Nlinks) return;

    link = &net->Link[i];
    setting = hyd->LinkSetting[i];
    model->LinkSetting[i] = setting;
    model->LinkDynamicLoss[i] =
        (model->LinkMinorLoss != NULL) ? model->LinkMinorLoss[i] : 0.0;

    // MISSING is a dimensional status sentinel, not a numerical setting.
    if (setting == MISSING) return;

    switch (link->Type)
    {
    case PRV:
        // Preserve the legacy evaluation order exactly: compile the complete
        // downstream target grade, not elevation and pressure separately.
        model->LinkSetting[i] = hydheadtosolver(pr,
            net->Node[link->N2].El + setting);
        break;

    case PSV:
        // Preserve the legacy evaluation order exactly for the upstream grade.
        model->LinkSetting[i] = hydheadtosolver(pr,
            net->Node[link->N1].El + setting);
        break;

    case PBV:
        model->LinkSetting[i] = hydheadtosolver(pr, setting);
        break;

    case FCV:
        model->LinkSetting[i] = hydflowtosolver(pr, setting);
        break;

    case TCV:
        // The TCV setting produces a dimensional minor-loss coefficient.
        km = 0.02517 * setting /
            (SQR(link->Diam) * SQR(link->Diam));
        model->LinkDynamicLoss[i] = hydminorlosstosolver(pr, km);
        break;

    case PCV:
        // setlinksetting()/resistcoeff() keep link->R synchronized with the
        // position-dependent PCV loss curve. Compile that loss once here.
        model->LinkDynamicLoss[i] = hydminorlosstosolver(pr, link->R);
        break;

    default:
        // Pump speed, GPV curve index, and other dimensionless settings are
        // already numerical quantities and remain unchanged.
        break;
    }
}


void compilehydraulicsolverlink(Project *pr, int i)
/*
**----------------------------------------------------------------
**  Purpose: refreshes one link's compiled solver coefficients
**----------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    ShydSolverModel *model = &hyd->SolverModel;
    Slink *link;
    double exponent;

    if (model->LinkResistance == NULL || model->LinkMinorLoss == NULL ||
        model->LinkViscosityFlow == NULL) return;
    if (i < 1 || i > net->Nlinks) return;

    link = &net->Link[i];
    model->LinkMinorLoss[i] = hydminorlosstosolver(pr, link->Km);
    model->LinkViscosityFlow[i] =
        hydflowtosolver(pr, hyd->Viscos * link->Diam);
    model->LinkResistance[i] = 0.0;

    if (link->Type == PIPE || link->Type == CVPIPE)
    {
        exponent = (hyd->Formflag == DW) ? 2.0 : hyd->Hexp;
        model->LinkResistance[i] =
            hydresistancetosolver(pr, link->R, exponent);
    }

    // Diameter, base loss, and PCV resistance can affect the compiled dynamic
    // setting, so refresh it whenever a link coefficient is recompiled.
    compilehydraulicsolversetting(pr, i);
}


static int ensuresolvercurvecapacity(Project *pr, int capacity)
/* Ensures SolverModel.Curve can address a network curve index. */
{
    ShydSolverModel *model = &pr->hydraul.SolverModel;
    ShydSolverCurve *curves;
    int oldCapacity;

    if (capacity <= model->CurveCapacity) return 0;
    oldCapacity = model->CurveCapacity;
    curves = (ShydSolverCurve *) realloc(model->Curve,
        (capacity + 1) * sizeof(ShydSolverCurve));
    if (curves == NULL) return 101;
    model->Curve = curves;
    memset(&model->Curve[oldCapacity + 1], 0,
        (capacity - oldCapacity) * sizeof(ShydSolverCurve));
    model->CurveCapacity = capacity;
    return 0;
}


int compilehydraulicsolvercurve(Project *pr, int i)
/*
**----------------------------------------------------------------
**  Purpose: compiles one flow/head curve into solver coordinates
**----------------------------------------------------------------
**  X stores solver-flow breakpoints. H0/R store the piecewise-linear segment
**  coefficients obtained with the same arithmetic used by the legacy
**  curvecoeff() path, followed by the usual solver scaling transforms.
*/
{
    Network *net = &pr->network;
    ShydSolverModel *model = &pr->hydraul.SolverModel;
    Scurve *curve;
    ShydSolverCurve *compiled;
    double *x, *h0, *r;
    double slope, intercept;
    int j, npts, errcode;

    if (i < 1 || i > net->Ncurves) return 206;
    if (model->Curve == NULL && model->CurveCapacity == 0) return 0;
    errcode = ensuresolvercurvecapacity(pr, i);
    if (errcode) return errcode;

    curve = &net->Curve[i];
    compiled = &model->Curve[i];
    npts = curve->Npts;

    // Curve edits are rare; hydraulic reinitialization is not. Grow storage
    // only when a curve gains points and otherwise reuse the existing cache.
    if (npts > compiled->Capacity)
    {
        x = (double *) calloc(npts, sizeof(double));
        h0 = NULL;
        r = NULL;
        if (npts > 1)
        {
            h0 = (double *) calloc(npts - 1, sizeof(double));
            r = (double *) calloc(npts - 1, sizeof(double));
        }
        if (x == NULL || (npts > 1 && (h0 == NULL || r == NULL)))
        {
            free(x);
            free(h0);
            free(r);
            return 101;
        }
        free(compiled->X);
        free(compiled->H0);
        free(compiled->R);
        compiled->X = x;
        compiled->H0 = h0;
        compiled->R = r;
        compiled->Capacity = npts;
    }

    for (j = 0; j < npts; j++)
    {
        // Curves retain Toolkit/user flow units outside the solver.
        compiled->X[j] = hydflowtosolver(pr,
            curve->X[j] / pr->Ucf[FLOW]);
    }
    for (j = 0; j < npts - 1; j++)
    {
        // Preserve curvecoeff() arithmetic exactly before crossing the solver
        // boundary so segment intercepts/slopes do not drift numerically.
        slope = (curve->Y[j + 1] - curve->Y[j]) /
            (curve->X[j + 1] - curve->X[j]);
        intercept = curve->Y[j] - slope * curve->X[j];
        intercept = intercept / pr->Ucf[HEAD];
        slope = slope * pr->Ucf[FLOW] / pr->Ucf[HEAD];
        compiled->H0[j] = hydheadtosolver(pr, intercept);
        compiled->R[j] = hydresistancetosolver(pr, slope, 1.0);
    }

    compiled->Npts = npts;
    return 0;
}


void compilehydraulicsolvermodel(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: compiles dimensional hydraulic model inputs into the
**           numerical representation consumed by the solver
**----------------------------------------------------------------
**  This is the dimensional -> numerical boundary for model data.
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    ShydSolverModel *model = &hyd->SolverModel;
    int i;

    if (model->NodeElevation == NULL || model->LinkResistance == NULL ||
        model->LinkMinorLoss == NULL || model->LinkViscosityFlow == NULL ||
        model->LinkSetting == NULL || model->LinkDynamicLoss == NULL)
    {
        return;
    }

    compilehydraulicsolverglobals(pr);

    for (i = 1; i <= net->Nnodes; i++)
    {
        model->NodeElevation[i] = hydheadtosolver(pr, net->Node[i].El);
    }

    // Curve source data is refreshed directly by Toolkit curve setters. The
    // full curve cache only needs rebuilding here when its unit/scaling basis
    // changes (including the initial compile after allocation).
    if (model->CurveHeadScale != hyd->SolverScale.Head ||
        model->CurveFlowScale != hyd->SolverScale.Flow ||
        model->CurveHeadUcf != pr->Ucf[HEAD] ||
        model->CurveFlowUcf != pr->Ucf[FLOW])
    {
        for (i = 1; i <= net->Ncurves; i++)
        {
            if (compilehydraulicsolvercurve(pr, i)) return;
        }
        model->CurveHeadScale = hyd->SolverScale.Head;
        model->CurveFlowScale = hyd->SolverScale.Flow;
        model->CurveHeadUcf = pr->Ucf[HEAD];
        model->CurveFlowUcf = pr->Ucf[FLOW];
    }

    for (i = 1; i <= net->Nlinks; i++)
    {
        compilehydraulicsolverlink(pr, i);
    }
}


void inithydraulicscaling(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes the hydraulic solver's numerical scales
**----------------------------------------------------------------
**  The model remains in EPANET's dimensional internal representation. The
**  GGA scales are chosen from the loaded physical network so typical heads
**  and flows enter the numerical solve near order one. Rounding each scale
**  down to a power of ten keeps the policy stable when model values differ
**  only by small input/conversion roundoff.
**
**  This routine is also called while project defaults are initialized, before
**  a network exists. In that case the 1.0 fallbacks are replaced when the
**  hydraulic system is opened after project validation.
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    double headMagnitude = 0.0;
    double flowMagnitude = 0.0;
    double demandMagnitude = 0.0;
    double headScale;
    double flowScale;
    double typicalFlow;
    int i;
    Pdemand demand;

    // Total base demand is a useful characteristic network throughput.
    for (i = 1; i <= net->Njuncs; i++)
    {
        for (demand = net->Node[i].D; demand != NULL; demand = demand->next)
        {
            demandMagnitude += fabs(demand->Base * hyd->Dmult);
        }
    }
    includecharacteristic(demandMagnitude, &flowMagnitude);

    // Pump operating points and FCV settings cover networks with little demand.
    for (i = 1; i <= net->Npumps; i++)
    {
        includecharacteristic(net->Pump[i].Q0, &flowMagnitude);
        includecharacteristic(net->Pump[i].Qmax, &flowMagnitude);
    }
    for (i = 1; i <= net->Nlinks; i++)
    {
        if (net->Link[i].Type == FCV)
        {
            includecharacteristic(net->Link[i].Kc, &flowMagnitude);
        }
    }
    flowScale = decadescale(flowMagnitude);
    typicalFlow = (flowMagnitude > 0.0) ? flowMagnitude : flowScale;

    // Absolute grades dominate the head unknowns solved by the GGA.
    for (i = 1; i <= net->Nnodes; i++)
    {
        includecharacteristic(net->Node[i].El, &headMagnitude);
    }
    for (i = 1; i <= net->Ntanks; i++)
    {
        includecharacteristic(net->Tank[i].H0, &headMagnitude);
        includecharacteristic(net->Tank[i].Hmin, &headMagnitude);
        includecharacteristic(net->Tank[i].Hmax, &headMagnitude);
    }

    includecharacteristic(hyd->Pmin, &headMagnitude);
    includecharacteristic(hyd->Preq, &headMagnitude);

    // Pump curve heads are head differences, but they still set the scale of
    // the node grades a pump can create. Constant-power pumps have no finite
    // Hmax, so estimate their head at the characteristic network flow instead.
    for (i = 1; i <= net->Npumps; i++)
    {
        Spump *pump = &net->Pump[i];
        includecharacteristic(pump->H0, &headMagnitude);
        includecharacteristic(pump->Hmax, &headMagnitude);
        if (pump->Ptype == CONST_HP && typicalFlow > 0.0)
        {
            includecharacteristic(pump->R / typicalFlow, &headMagnitude);
        }
    }

    // Pressure-control valve settings and simple control grades also represent
    // physical heads that can dominate otherwise low-elevation networks.
    for (i = 1; i <= net->Nlinks; i++)
    {
        LinkType type = net->Link[i].Type;
        if (type == PRV || type == PSV || type == PBV)
        {
            includecharacteristic(net->Link[i].Kc, &headMagnitude);
        }
    }
    for (i = 1; i <= net->Ncontrols; i++)
    {
        if (net->Control[i].Node > 0)
        {
            includecharacteristic(net->Control[i].Grade, &headMagnitude);
        }
    }
    headScale = decadescale(headMagnitude);
    sethydraulicsolverscale(pr, headScale, flowScale);
}


void sethydraulicsolverscale(Project *pr, double headScale, double flowScale)
/*
**----------------------------------------------------------------
**  Purpose: sets solver scales and caches their invariant transforms
**----------------------------------------------------------------
**  The cached powers are consumed inside hydraulic iteration loops. Keeping
**  the common resistance powers here avoids repeated pow() calls while the
**  original multiply/divide evaluation order remains unchanged.
*/
{
    Hydraul *hyd = &pr->hydraul;
    ShydScale *scale = &hyd->SolverScale;
    double exponent = hyd->Hexp;

    if (!isfinite(headScale) || headScale <= 0.0) headScale = 1.0;
    if (!isfinite(flowScale) || flowScale <= 0.0) flowScale = 1.0;
    if (!isfinite(exponent)) exponent = 0.0;

    scale->Head = headScale;
    scale->Flow = flowScale;
    scale->FlowPower1 = pow(flowScale, 1.0);
    scale->FlowPower2 = pow(flowScale, 2.0);
    scale->FlowPowerHexp = pow(flowScale, exponent);

    // SolverModel is another cache of scale-dependent values. Keep it in
    // sync when tests or future callers deliberately replace the scale.
    if (hyd->SolverModel.NodeElevation != NULL)
    {
        compilehydraulicsolvermodel(pr);
    }
}


void loadhydraulicsolverstate(Project *pr)
/*
**----------------------------------------------------------------
**  Purpose: copies dimensional hydraulic state into solver state
**----------------------------------------------------------------
**  This is the dimensional -> numerical boundary for hydraulic state.
**  It includes tank node head and net inflow (NodeDemand): tank/event logic
**  keeps using the dimensional arrays while GGA uses SolverState. Keep unit
**  conversion here rather than scattering it through either subsystem.
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
**  Tank head and net inflow are published here before timestep, control, rule,
**  energy, and quality calculations consume the dimensional hydraulic state.
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
    Hydraul *hyd = &pr->hydraul;
    ShydScale *scale = &hyd->SolverScale;

    if (exponent == 1.0)
        return resistance * scale->FlowPower1 / scale->Head;
    if (exponent == 2.0)
        return resistance * scale->FlowPower2 / scale->Head;
    if (exponent == hyd->Hexp)
        return resistance * scale->FlowPowerHexp / scale->Head;
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
    ShydScale *scale = &pr->hydraul.SolverScale;

    return resistance * scale->FlowPower2 / scale->Head;
}
