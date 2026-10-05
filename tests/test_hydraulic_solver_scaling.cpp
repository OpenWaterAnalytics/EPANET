/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_hydraulic_solver_scaling.cpp
 Description:  Regression tests for hydraulic solver scaling invariance
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 ******************************************************************************
*/

#include <cmath>
#include <cstddef>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"
#include "../src/types.h"
#include "../src/funcs.h"

namespace
{

const double HEAD_TOL = 1.e-6;
const double FLOW_TOL = 1.e-5;
const double VOLUME_TOL = 1.e-3;
const double SPECIAL_TOL = 1.e-7;
const double QUALITY_TOL = 1.e-9;
const double COMPAT_HEAD_TOL = 5.e-5;
const double COMPAT_FLOW_TOL = 5.e-6;

const char* EXAMPLE_NET1 = EPANET_SOURCE_DIR "/example-networks/Net1.inp";
const char* EXAMPLE_NET2 = EPANET_SOURCE_DIR "/example-networks/Net2.inp";
const char* EXAMPLE_NET3 = EPANET_SOURCE_DIR "/example-networks/Net3.inp";

struct SolverScale
{
    double head;
    double flow;
    const char* name;
};

// Exercise both larger and smaller numerical values than the legacy solver.
const SolverScale SCALES[] = {
    {10.0,  2.0,  "H10_Q2"},
    {0.1,   0.2,  "H0.1_Q0.2"},
    {100.0, 10.0, "H100_Q10"},
    {0.01,  0.01, "H0.01_Q0.01"}
};

void set_solver_scale(EN_Project ph, const SolverScale& scale)
{
    // White-box test hook: SolverScale is intentionally internal and is not
    // part of the public Toolkit API.
    const int error = sethydraulicsolverscale(ph, scale.head, scale.flow);
    BOOST_REQUIRE(error == 0);
}

void check_compiled_curve(EN_Project ph, int i)
{
    Network& net = ph->network;
    ShydSolverModel& model = ph->hydraul.SolverModel;
    Scurve& curve = net.Curve[i];
    ShydSolverCurve& compiled = model.Curve[i];

    BOOST_REQUIRE(model.Curve != NULL);
    BOOST_REQUIRE(i <= model.CurveCapacity);
    BOOST_CHECK_EQUAL(compiled.Npts, curve.Npts);
    BOOST_CHECK(compiled.Capacity >= curve.Npts);

    for (int j = 0; j < curve.Npts; ++j)
    {
        BOOST_CHECK_EQUAL(compiled.X[j],
            hydflowtosolver(ph, curve.X[j] / ph->Ucf[FLOW]));
    }
    for (int j = 0; j < curve.Npts - 1; ++j)
    {
        double slope = (curve.Y[j + 1] - curve.Y[j]) /
            (curve.X[j + 1] - curve.X[j]);
        double intercept = curve.Y[j] - slope * curve.X[j];
        intercept = intercept / ph->Ucf[HEAD];
        slope = slope * ph->Ucf[FLOW] / ph->Ucf[HEAD];
        BOOST_CHECK_EQUAL(compiled.H0[j], hydheadtosolver(ph, intercept));
        BOOST_CHECK_EQUAL(compiled.R[j],
            hydresistancetosolver(ph, slope, 1.0));
    }
}

void check_solver_model_compilation(EN_Project ph)
{
    Network& net = ph->network;
    Hydraul& hyd = ph->hydraul;
    ShydSolverModel& model = hyd.SolverModel;

    BOOST_REQUIRE(model.NodeElevation != NULL);
    BOOST_REQUIRE(model.NodeEmitterResistance != NULL);
    BOOST_REQUIRE(model.NodePdaMinGrade != NULL);
    BOOST_REQUIRE(model.LinkResistance != NULL);
    BOOST_REQUIRE(model.LinkMinorLoss != NULL);
    BOOST_REQUIRE(model.LinkViscosityFlow != NULL);
    BOOST_REQUIRE(model.LinkSetting != NULL);
    BOOST_REQUIRE(model.LinkDynamicLoss != NULL);
    BOOST_REQUIRE(model.LinkPumpH0 != NULL);
    BOOST_REQUIRE(model.LinkPumpResistance != NULL);
    BOOST_REQUIRE(model.LinkPumpMaxHead != NULL);
    BOOST_REQUIRE(model.ControlGrade != NULL);
    BOOST_REQUIRE(model.Curve != NULL);
    BOOST_CHECK(model.CurveCapacity >= net.Ncurves);
    BOOST_CHECK(model.ControlCapacity >= net.Ncontrols);
    BOOST_CHECK_EQUAL(model.CurveHeadScale, hyd.SolverScale.Head);
    BOOST_CHECK_EQUAL(model.CurveFlowScale, hyd.SolverScale.Flow);
    BOOST_CHECK_EQUAL(model.CurveHeadUcf, ph->Ucf[HEAD]);
    BOOST_CHECK_EQUAL(model.CurveFlowUcf, ph->Ucf[FLOW]);

    BOOST_CHECK_EQUAL(model.Htol, hydheadtosolver(ph, hyd.Htol));
    BOOST_CHECK_EQUAL(model.Qtol, hydflowtosolver(ph, hyd.Qtol));
    BOOST_CHECK_EQUAL(model.RQtol,
        hydresistancetosolver(ph, hyd.RQtol, 1.0));
    BOOST_CHECK_EQUAL(model.Pmin, hydheadtosolver(ph, hyd.Pmin));
    BOOST_CHECK_EQUAL(model.Preq, hydheadtosolver(ph, hyd.Preq));
    BOOST_CHECK_EQUAL(model.PdaPressureRange, hydheadtosolver(ph,
        MAX((hyd.Preq - hyd.Pmin), MINPDIFF)));
    BOOST_CHECK_EQUAL(model.FlowChangeLimit,
        hydflowtosolver(ph, hyd.FlowChangeLimit));
    BOOST_CHECK_EQUAL(model.HeadErrorLimit,
        hydheadtosolver(ph, hyd.HeadErrorLimit));
    BOOST_CHECK_EQUAL(model.RelativeErrorFlowCutoff,
        hydflowtosolver(ph, hyd.Hacc));
    BOOST_CHECK_EQUAL(model.TinyFlow, hydflowtosolver(ph, TINY));
    BOOST_CHECK_EQUAL(model.LeakageFlowTolerance,
        hydflowtosolver(ph, 0.0001));
    BOOST_CHECK_EQUAL(model.BigHead, hydheadtosolver(ph, BIG));
    BOOST_CHECK_EQUAL(model.TinyGradient,
        hydresistancetosolver(ph, TINY, 1.0));
    BOOST_CHECK_EQUAL(model.SmallGradient,
        hydresistancetosolver(ph, CSMALL, 1.0));
    BOOST_CHECK_EQUAL(model.BigGradient,
        hydresistancetosolver(ph, CBIG, 1.0));
    BOOST_CHECK_EQUAL(model.BigConductance,
        hydconductancetosolver(ph, CBIG));
    BOOST_CHECK_EQUAL(model.SmallConductance,
        hydconductancetosolver(ph, 1.0 / CBIG));
    BOOST_CHECK_EQUAL(model.BarrierGradient,
        hydresistancetosolver(ph, 1.e9, 1.0));
    BOOST_CHECK_EQUAL(model.BarrierSmoothingHead,
        hydheadtosolver(ph, 0.001));

    for (int i = 1; i <= net.Nnodes; ++i)
    {
        BOOST_CHECK_EQUAL(model.NodeElevation[i],
            hydheadtosolver(ph, net.Node[i].El));
        BOOST_CHECK_EQUAL(model.NodeEmitterResistance[i],
            hydresistancetosolver(ph, MAX(CSMALL, net.Node[i].Ke), hyd.Qexp));
        BOOST_CHECK_EQUAL(model.NodePdaMinGrade[i],
            hydheadtosolver(ph, net.Node[i].El + hyd.Pmin));
    }

    for (int i = 1; i <= net.Ncontrols; ++i)
    {
        BOOST_CHECK_EQUAL(model.ControlGrade[i],
            hydheadtosolver(ph, net.Control[i].Grade));
    }

    for (int i = 1; i <= net.Ncurves; ++i)
    {
        if (net.Curve[i].Type == PUMP_CURVE ||
            net.Curve[i].Type == HLOSS_CURVE)
        {
            check_compiled_curve(ph, i);
        }
        else
        {
            BOOST_CHECK_EQUAL(model.Curve[i].Npts, 0);
        }
    }

    for (int i = 1; i <= net.Nlinks; ++i)
    {
        Slink& link = net.Link[i];
        BOOST_CHECK_EQUAL(model.LinkMinorLoss[i],
            hydminorlosstosolver(ph, link.Km));
        BOOST_CHECK_EQUAL(model.LinkViscosityFlow[i],
            hydflowtosolver(ph, hyd.Viscos * link.Diam));

        if (link.Type == PIPE || link.Type == CVPIPE)
        {
            const double exponent = hyd.Formflag == DW ? 2.0 : hyd.Hexp;
            BOOST_CHECK_EQUAL(model.LinkResistance[i],
                hydresistancetosolver(ph, link.R, exponent));
        }
        else
        {
            BOOST_CHECK_EQUAL(model.LinkResistance[i], 0.0);
        }

        if (link.Type == PUMP)
        {
            const int p = findpump(&net, i);
            Spump& pump = net.Pump[p];
            if (pump.Ptype != CUSTOM && pump.Ptype != NOCURVE)
            {
                double exponent = pump.N;
                if (ABS(exponent - 1.0) < TINY) exponent = 1.0;
                BOOST_CHECK_EQUAL(model.LinkPumpH0[i],
                    hydheadtosolver(ph, pump.H0));
                BOOST_CHECK_EQUAL(model.LinkPumpResistance[i],
                    hydresistancetosolver(ph, pump.R, exponent));
            }
            else
            {
                BOOST_CHECK_EQUAL(model.LinkPumpH0[i], 0.0);
                BOOST_CHECK_EQUAL(model.LinkPumpResistance[i], 0.0);
            }
            if (pump.Ptype == CONST_HP)
                BOOST_CHECK_EQUAL(model.LinkPumpMaxHead[i], model.BigHead);
            else
                BOOST_CHECK_EQUAL(model.LinkPumpMaxHead[i],
                    hydheadtosolver(ph, SQR(hyd.LinkSetting[i]) * pump.Hmax));
        }
        else
        {
            BOOST_CHECK_EQUAL(model.LinkPumpH0[i], 0.0);
            BOOST_CHECK_EQUAL(model.LinkPumpResistance[i], 0.0);
            BOOST_CHECK_EQUAL(model.LinkPumpMaxHead[i], 0.0);
        }

        // Ordinary pipes do not consume numerical link settings or dynamic
        // setting-dependent losses, so those compiled slots remain zero.
        double expectedSetting = 0.0;
        double expectedDynamicLoss = 0.0;
        if (link.Type > PIPE)
        {
            expectedSetting = hyd.LinkSetting[i];
            expectedDynamicLoss = model.LinkMinorLoss[i];
        }
        if (link.Type > PIPE && expectedSetting != MISSING)
        {
            switch (link.Type)
            {
            case PRV:
                expectedSetting = hydheadtosolver(ph,
                    net.Node[link.N2].El + hyd.LinkSetting[i]);
                break;
            case PSV:
                expectedSetting = hydheadtosolver(ph,
                    net.Node[link.N1].El + hyd.LinkSetting[i]);
                break;
            case PBV:
                expectedSetting = hydheadtosolver(ph, hyd.LinkSetting[i]);
                break;
            case FCV:
                expectedSetting = hydflowtosolver(ph, hyd.LinkSetting[i]);
                break;
            case TCV:
            {
                const double km = 0.02517 * hyd.LinkSetting[i] /
                    (SQR(link.Diam) * SQR(link.Diam));
                expectedDynamicLoss = hydminorlosstosolver(ph, km);
                break;
            }
            case PCV:
                expectedDynamicLoss = hydminorlosstosolver(ph, link.R);
                break;
            default:
                break;
            }
        }
        BOOST_CHECK_EQUAL(model.LinkSetting[i], expectedSetting);
        BOOST_CHECK_EQUAL(model.LinkDynamicLoss[i], expectedDynamicLoss);
    }
}

void solve_hydraulics_with_scale(EN_Project ph, const SolverScale& scale, int initFlag)
{
    int error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    set_solver_scale(ph, scale);
    error = EN_initH(ph, initFlag);
    BOOST_REQUIRE(error == 0);

    long time = 0;
    long timeStep = 0;
    do
    {
        error = EN_runH(ph, &time);
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &timeStep);
        BOOST_REQUIRE(error == 0);
    }
    while (timeStep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
}

int get_node_index(EN_Project ph, const char* id)
{
    int index = 0;
    const int error = EN_getnodeindex(ph, id, &index);
    BOOST_REQUIRE(error == 0);
    return index;
}

int get_link_index(EN_Project ph, const char* id)
{
    int index = 0;
    const int error = EN_getlinkindex(ph, id, &index);
    BOOST_REQUIRE(error == 0);
    return index;
}

double get_node_value(EN_Project ph, int index, int property)
{
    double value = 0.0;
    const int error = EN_getnodevalue(ph, index, property, &value);
    BOOST_REQUIRE(error == 0);
    return value;
}

double get_link_value(EN_Project ph, int index, int property)
{
    double value = 0.0;
    const int error = EN_getlinkvalue(ph, index, property, &value);
    BOOST_REQUIRE(error == 0);
    return value;
}

void check_near(double actual, double expected, double tolerance,
    const SolverScale& scale, const char* quantity)
{
    BOOST_CHECK_MESSAGE(std::fabs(actual - expected) <= tolerance,
        scale.name << ": " << quantity << " expected " << expected <<
        ", got " << actual);
}

struct EpsSnapshot
{
    long time;
    std::vector<double> nodeHead;
    std::vector<double> nodeDemand;
    std::vector<double> linkFlow;
    std::vector<int> linkStatus;
    double tankVolume;
};

std::vector<EpsSnapshot> solve_net1_eps(const SolverScale& scale)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    int nodeCount = 0;
    int linkCount = 0;
    error = EN_getcount(ph, EN_NODECOUNT, &nodeCount);
    BOOST_REQUIRE(error == 0);
    error = EN_getcount(ph, EN_LINKCOUNT, &linkCount);
    BOOST_REQUIRE(error == 0);
    const int tank2 = get_node_index(ph, "2");

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    set_solver_scale(ph, scale);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    std::vector<EpsSnapshot> result;
    long time = 0;
    long timeStep = 0;

    do
    {
        error = EN_runH(ph, &time);
        BOOST_REQUIRE(error == 0);

        EpsSnapshot snapshot;
        snapshot.time = time;
        snapshot.nodeHead.resize(nodeCount);
        snapshot.nodeDemand.resize(nodeCount);
        snapshot.linkFlow.resize(linkCount);
        snapshot.linkStatus.resize(linkCount);
        snapshot.tankVolume = get_node_value(ph, tank2, EN_TANKVOLUME);

        for (int i = 1; i <= nodeCount; ++i)
        {
            snapshot.nodeHead[i - 1] = get_node_value(ph, i, EN_HEAD);
            snapshot.nodeDemand[i - 1] = get_node_value(ph, i, EN_DEMAND);
        }
        for (int i = 1; i <= linkCount; ++i)
        {
            snapshot.linkFlow[i - 1] = get_link_value(ph, i, EN_FLOW);
            snapshot.linkStatus[i - 1] = static_cast<int>(
                get_link_value(ph, i, EN_STATUS));
        }
        result.push_back(snapshot);

        error = EN_nextH(ph, &timeStep);
        BOOST_REQUIRE(error == 0);
    }
    while (timeStep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    return result;
}

void compare_eps(const std::vector<EpsSnapshot>& actual,
    const std::vector<EpsSnapshot>& expected, const SolverScale& scale)
{
    BOOST_REQUIRE_MESSAGE(actual.size() == expected.size(),
        scale.name << ": expected " << expected.size() <<
        " hydraulic events, got " << actual.size());

    for (std::size_t event = 0; event < expected.size(); ++event)
    {
        const EpsSnapshot& a = actual[event];
        const EpsSnapshot& e = expected[event];
        BOOST_REQUIRE_MESSAGE(a.time == e.time,
            scale.name << ": event " << event << " expected at " << e.time <<
            " s, got " << a.time << " s");

        BOOST_REQUIRE(a.nodeHead.size() == e.nodeHead.size());
        BOOST_REQUIRE(a.linkFlow.size() == e.linkFlow.size());

        for (std::size_t i = 0; i < e.nodeHead.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(std::fabs(a.nodeHead[i] - e.nodeHead[i]) <= HEAD_TOL,
                scale.name << " at t=" << e.time << " s: node " << (i + 1) <<
                " head differs by " << std::fabs(a.nodeHead[i] - e.nodeHead[i]));
            BOOST_CHECK_MESSAGE(std::fabs(a.nodeDemand[i] - e.nodeDemand[i]) <= FLOW_TOL,
                scale.name << " at t=" << e.time << " s: node " << (i + 1) <<
                " demand differs by " << std::fabs(a.nodeDemand[i] - e.nodeDemand[i]));
        }
        for (std::size_t i = 0; i < e.linkFlow.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(std::fabs(a.linkFlow[i] - e.linkFlow[i]) <= FLOW_TOL,
                scale.name << " at t=" << e.time << " s: link " << (i + 1) <<
                " flow differs by " << std::fabs(a.linkFlow[i] - e.linkFlow[i]));
            BOOST_CHECK_MESSAGE(a.linkStatus[i] == e.linkStatus[i],
                scale.name << " at t=" << e.time << " s: link " << (i + 1) <<
                " status expected " << e.linkStatus[i] << ", got " << a.linkStatus[i]);
        }
        BOOST_CHECK_MESSAGE(std::fabs(a.tankVolume - e.tankVolume) <= VOLUME_TOL,
            scale.name << " at t=" << e.time << " s: tank volume differs by " <<
            std::fabs(a.tankVolume - e.tankVolume));
    }
}

struct CompatibilitySnapshot
{
    long time;
    std::vector<double> nodeHead;
    std::vector<double> nodeDemand;
    std::vector<double> linkFlow;
    std::vector<int> linkStatus;
};

std::vector<CompatibilitySnapshot> solve_example_eps(
    const char* inputPath, bool forceLegacyScale)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, inputPath, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    if (forceLegacyScale)
    {
        error = sethydraulicsolverscale(ph, 1.0, 1.0);
        BOOST_REQUIRE(error == 0);
    }
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    const int nodeCount = ph->network.Nnodes;
    const int linkCount = ph->network.Nlinks;
    std::vector<CompatibilitySnapshot> result;
    long time = 0;
    long timeStep = 0;

    do
    {
        error = EN_runH(ph, &time);
        BOOST_REQUIRE(error == 0);

        CompatibilitySnapshot snapshot;
        snapshot.time = time;
        snapshot.nodeHead.resize(nodeCount);
        snapshot.nodeDemand.resize(nodeCount);
        snapshot.linkFlow.resize(linkCount);
        snapshot.linkStatus.resize(linkCount);

        // Compare the dimensional state published by hydsolve(). This avoids
        // making the compatibility check depend on each example's public units.
        for (int i = 1; i <= nodeCount; ++i)
        {
            snapshot.nodeHead[i - 1] = ph->hydraul.NodeHead[i];
            snapshot.nodeDemand[i - 1] = ph->hydraul.NodeDemand[i];
        }
        for (int i = 1; i <= linkCount; ++i)
        {
            snapshot.linkFlow[i - 1] = ph->hydraul.LinkFlow[i];
            snapshot.linkStatus[i - 1] = ph->hydraul.LinkStatus[i];
        }
        result.push_back(snapshot);

        error = EN_nextH(ph, &timeStep);
        BOOST_REQUIRE(error == 0);
    }
    while (timeStep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    return result;
}

void compare_example_eps(const std::vector<CompatibilitySnapshot>& actual,
    const std::vector<CompatibilitySnapshot>& expected, const char* networkName)
{
    BOOST_REQUIRE_MESSAGE(actual.size() == expected.size(),
        networkName << ": expected " << expected.size() <<
        " hydraulic events, got " << actual.size());

    for (std::size_t event = 0; event < expected.size(); ++event)
    {
        const CompatibilitySnapshot& a = actual[event];
        const CompatibilitySnapshot& e = expected[event];
        BOOST_REQUIRE_MESSAGE(a.time == e.time,
            networkName << ": event " << event << " expected at " << e.time <<
            " s, got " << a.time << " s");

        for (std::size_t i = 0; i < e.nodeHead.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(std::fabs(a.nodeHead[i] - e.nodeHead[i]) <=
                COMPAT_HEAD_TOL, networkName << " at t=" << e.time <<
                " s: node " << (i + 1) << " head differs by " <<
                std::fabs(a.nodeHead[i] - e.nodeHead[i]));
            BOOST_CHECK_MESSAGE(std::fabs(a.nodeDemand[i] - e.nodeDemand[i]) <=
                COMPAT_FLOW_TOL, networkName << " at t=" << e.time <<
                " s: node " << (i + 1) << " demand differs by " <<
                std::fabs(a.nodeDemand[i] - e.nodeDemand[i]));
        }
        for (std::size_t i = 0; i < e.linkFlow.size(); ++i)
        {
            BOOST_CHECK_MESSAGE(std::fabs(a.linkFlow[i] - e.linkFlow[i]) <=
                COMPAT_FLOW_TOL, networkName << " at t=" << e.time <<
                " s: link " << (i + 1) << " flow differs by " <<
                std::fabs(a.linkFlow[i] - e.linkFlow[i]));
            BOOST_CHECK_MESSAGE(a.linkStatus[i] == e.linkStatus[i],
                networkName << " at t=" << e.time << " s: link " << (i + 1) <<
                " status expected " << e.linkStatus[i] << ", got " <<
                a.linkStatus[i]);
        }
    }
}

struct ConvergenceSnapshot
{
    int iterations;
    double relativeError;
    std::vector<double> nodeHead;
    std::vector<double> linkFlow;
};

ConvergenceSnapshot solve_net1_convergence_case(double flowScale)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    // Isolate flow scaling: retain the model-derived head scale while forcing
    // a flow scale large enough to cross the legacy low-flow Hacc branch.
    const double headScale = ph->hydraul.SolverScale.Head;
    error = sethydraulicsolverscale(ph, headScale, flowScale);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);

    long time = 0;
    error = EN_runH(ph, &time);
    BOOST_REQUIRE(error == 0);

    double value = 0.0;
    error = EN_getstatistic(ph, EN_ITERATIONS, &value);
    BOOST_REQUIRE(error == 0);

    ConvergenceSnapshot result;
    result.iterations = static_cast<int>(value);
    error = EN_getstatistic(ph, EN_RELATIVEERROR, &result.relativeError);
    BOOST_REQUIRE(error == 0);
    result.nodeHead.resize(ph->network.Nnodes);
    result.linkFlow.resize(ph->network.Nlinks);
    for (int i = 1; i <= ph->network.Nnodes; ++i)
        result.nodeHead[i - 1] = ph->hydraul.NodeHead[i];
    for (int i = 1; i <= ph->network.Nlinks; ++i)
        result.linkFlow[i - 1] = ph->hydraul.LinkFlow[i];

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
    return result;
}

struct SpecialSnapshot
{
    std::vector<double> nodeHead;
    std::vector<double> nodeDemand;
    std::vector<double> emitterFlow;
    std::vector<double> leakageFlow;
    std::vector<double> linkFlow;
    std::vector<double> linkHeadloss;
    std::vector<int> linkStatus;
};

SpecialSnapshot solve_special_case(const SolverScale& scale)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_HEADLOSSFORM, EN_DW);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_DEMANDMULT, 10.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setdemandmodel(ph, EN_PDA, 20.0, 100.0, 0.5);
    BOOST_REQUIRE(error == 0);

    const int node21 = get_node_index(ph, "21");
    const int pipe21 = get_link_index(ph, "21");
    error = EN_setnodevalue(ph, node21, EN_EMITTER, 5.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe21, EN_LEAK_AREA, 1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe21, EN_LEAK_EXPAN, 0.1);
    BOOST_REQUIRE(error == 0);

    solve_hydraulics_with_scale(ph, scale, EN_NOSAVE);

    int nodeCount = 0;
    int linkCount = 0;
    error = EN_getcount(ph, EN_NODECOUNT, &nodeCount);
    BOOST_REQUIRE(error == 0);
    error = EN_getcount(ph, EN_LINKCOUNT, &linkCount);
    BOOST_REQUIRE(error == 0);

    SpecialSnapshot result;
    result.nodeHead.resize(nodeCount);
    result.nodeDemand.resize(nodeCount);
    result.emitterFlow.resize(nodeCount);
    result.leakageFlow.resize(nodeCount);
    result.linkFlow.resize(linkCount);
    result.linkHeadloss.resize(linkCount);
    result.linkStatus.resize(linkCount);

    for (int i = 1; i <= nodeCount; ++i)
    {
        result.nodeHead[i - 1] = get_node_value(ph, i, EN_HEAD);
        result.nodeDemand[i - 1] = get_node_value(ph, i, EN_DEMAND);
        result.emitterFlow[i - 1] = get_node_value(ph, i, EN_EMITTERFLOW);
        result.leakageFlow[i - 1] = get_node_value(ph, i, EN_LEAKAGEFLOW);
    }
    for (int i = 1; i <= linkCount; ++i)
    {
        result.linkFlow[i - 1] = get_link_value(ph, i, EN_FLOW);
        result.linkHeadloss[i - 1] = get_link_value(ph, i, EN_HEADLOSS);
        result.linkStatus[i - 1] = static_cast<int>(
            get_link_value(ph, i, EN_STATUS));
    }

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    return result;
}

void compare_special(const SpecialSnapshot& actual, const SpecialSnapshot& expected,
    const SolverScale& scale)
{
    for (std::size_t i = 0; i < expected.nodeHead.size(); ++i)
    {
        check_near(actual.nodeHead[i], expected.nodeHead[i], SPECIAL_TOL,
            scale, "special-case node head");
        check_near(actual.nodeDemand[i], expected.nodeDemand[i], SPECIAL_TOL,
            scale, "special-case node demand");
        check_near(actual.emitterFlow[i], expected.emitterFlow[i], SPECIAL_TOL,
            scale, "special-case emitter flow");
        check_near(actual.leakageFlow[i], expected.leakageFlow[i], SPECIAL_TOL,
            scale, "special-case leakage flow");
    }
    for (std::size_t i = 0; i < expected.linkFlow.size(); ++i)
    {
        check_near(actual.linkFlow[i], expected.linkFlow[i], SPECIAL_TOL,
            scale, "special-case link flow");
        check_near(actual.linkHeadloss[i], expected.linkHeadloss[i], SPECIAL_TOL,
            scale, "special-case link headloss");
        BOOST_CHECK_MESSAGE(actual.linkStatus[i] == expected.linkStatus[i],
            scale.name << ": special-case link " << (i + 1) <<
            " status expected " << expected.linkStatus[i] <<
            ", got " << actual.linkStatus[i]);
    }
}

struct CurveSnapshot
{
    double pumpFlow;
    double pumpDischargeHead;
    double gpvFlow;
    double gpvHeadloss;
};

CurveSnapshot solve_curve_case(const SolverScale& scale)
{
    CurveSnapshot result;

    // Multi-segment custom pump curve.
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    double pumpX[] = {0.0, 1000.0, 2200.0, 4000.0};
    double pumpY[] = {300.0, 270.0, 190.0, 70.0};
    error = EN_setcurve(ph, 1, pumpX, pumpY, 4);
    BOOST_REQUIRE(error == 0);
    const int pump = get_link_index(ph, "9");
    const int discharge = get_node_index(ph, "10");
    solve_hydraulics_with_scale(ph, scale, EN_NOSAVE);
    result.pumpFlow = get_link_value(ph, pump, EN_FLOW);
    result.pumpDischargeHead = get_node_value(ph, discharge, EN_HEAD);
    EN_close(ph);
    EN_deleteproject(ph);

    // General-purpose valve with a multi-segment headloss curve.
    ph = NULL;
    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    int gpv = get_link_index(ph, "22");
    error = EN_setlinktype(ph, &gpv, EN_GPV, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, "solver-gpv-curve");
    BOOST_REQUIRE(error == 0);
    int curve = 0;
    error = EN_getcurveindex(ph, "solver-gpv-curve", &curve);
    BOOST_REQUIRE(error == 0);
    double gpvX[] = {0.0, 200.0, 800.0, 2000.0};
    double gpvY[] = {0.0, 2.0, 25.0, 120.0};
    error = EN_setcurve(ph, curve, gpvX, gpvY, 4);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurvetype(ph, curve, EN_HLOSS_CURVE);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, gpv, EN_GPV_CURVE, curve);
    BOOST_REQUIRE(error == 0);
    solve_hydraulics_with_scale(ph, scale, EN_NOSAVE);
    result.gpvFlow = get_link_value(ph, gpv, EN_FLOW);
    result.gpvHeadloss = get_link_value(ph, gpv, EN_HEADLOSS);
    EN_close(ph);
    EN_deleteproject(ph);

    return result;
}


struct PrvSnapshot
{
    double node31Head;
    double node31Pressure;
    double checkValveFlow;
    double prvFlow;
    int prvStatus;
};

PrvSnapshot solve_prv_case(const SolverScale& scale)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    int pipe113 = get_link_index(ph, "113");
    int pipe121 = get_link_index(ph, "121");
    const int node31 = get_node_index(ph, "31");
    double diameter = 0.0;

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinktype(ph, &pipe113, EN_CVPIPE, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, pipe121, EN_DIAMETER, &diameter);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinktype(ph, &pipe121, EN_PRV, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe121, EN_INITSETTING, 100.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe121, EN_DIAMETER, diameter);
    BOOST_REQUIRE(error == 0);

    solve_hydraulics_with_scale(ph, scale, EN_NOSAVE);

    pipe113 = get_link_index(ph, "113");
    pipe121 = get_link_index(ph, "121");

    PrvSnapshot result;
    result.node31Head = get_node_value(ph, node31, EN_HEAD);
    result.node31Pressure = get_node_value(ph, node31, EN_PRESSURE);
    result.checkValveFlow = get_link_value(ph, pipe113, EN_FLOW);
    result.prvFlow = get_link_value(ph, pipe121, EN_FLOW);
    result.prvStatus = static_cast<int>(get_link_value(ph, pipe121, EN_STATUS));

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    return result;
}

struct BoundarySnapshot
{
    double pumpEnergy;
    double pumpEfficiency;
    double quality21;
    double quality32;
    double massBalance;
};

BoundarySnapshot solve_boundary_case(const SolverScale& scale)
{
    BoundarySnapshot result;

    // Energy uses the dimensional hydraulic state published after the solve.
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    const int pump9 = get_link_index(ph, "9");
    solve_hydraulics_with_scale(ph, scale, EN_NOSAVE);
    result.pumpEnergy = get_link_value(ph, pump9, EN_ENERGY);
    result.pumpEfficiency = get_link_value(ph, pump9, EN_PUMP_EFFIC);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    // Quality likewise consumes dimensional, published hydraulic results.
    ph = NULL;
    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    const int node21 = get_node_index(ph, "21");
    const int node32 = get_node_index(ph, "32");
    solve_hydraulics_with_scale(ph, scale, EN_SAVE);
    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);
    result.quality21 = get_node_value(ph, node21, EN_QUALITY);
    result.quality32 = get_node_value(ph, node32, EN_QUALITY);
    error = EN_getstatistic(ph, EN_MASSBALANCE, &result.massBalance);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

    return result;
}

SolverScale get_production_scale(int flowUnits, const char* name)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_setflowunits(ph, flowUnits);
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    SolverScale result = {
        ph->hydraul.SolverScale.Head,
        ph->hydraul.SolverScale.Flow,
        name
    };

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
    return result;
}

const SolverScale LEGACY_SCALE = {1.0, 1.0, "legacy"};

} // namespace


BOOST_AUTO_TEST_SUITE(test_hydraulic_solver_scaling)

BOOST_AUTO_TEST_CASE(test_solver_model_compiles_dimensional_inputs)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET3, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    // EN_openH compiles the validated model using the production scale.
    check_solver_model_compilation(ph);

    // Replacing the scale must refresh all scale-dependent compiled values.
    const SolverScale forced = {10.0, 2.0, "compiled-model"};
    set_solver_scale(ph, forced);
    check_solver_model_compilation(ph);

    // EN_initH computes dimensional link resistances and recompiles the model.
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);
    check_solver_model_compilation(ph);

    // The scale setter also refreshes an already initialized compiled model.
    const SolverScale forcedAfterInit = {0.1, 0.2, "compiled-model-after-init"};
    set_solver_scale(ph, forcedAfterInit);
    check_solver_model_compilation(ph);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph->hydraul.SolverModel.NodeElevation == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.NodeEmitterResistance == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.NodePdaMinGrade == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkResistance == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkMinorLoss == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkViscosityFlow == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkSetting == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkDynamicLoss == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkPumpH0 == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkPumpResistance == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.LinkPumpMaxHead == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.ControlGrade == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.Curve == NULL);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.CurveCapacity, 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.ControlCapacity, 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.CurveHeadScale, 0.0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.CurveFlowScale, 0.0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(test_solver_state_boundary_copies_only_consumed_inputs)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);

    const SolverScale forced = {100.0, 10.0, "state-boundary"};
    set_solver_scale(ph, forced);

    Network& net = ph->network;
    Hydraul& hyd = ph->hydraul;
    ShydSolverState& state = hyd.SolverState;

    // Seed dimensional input state and a distinct solver sentinel. NodeDemand
    // is output-only for GGA: newflows() reconstructs fixed-grade inflows and
    // junction totals are assembled after convergence, so it must not be
    // copied into the solver at the entry boundary.
    const double solverSentinel = -9876.5;
    const double fixedDimensionalSentinel = 4321.25;
    for (int i = 1; i <= net.Nnodes; ++i)
    {
        hyd.NodeHead[i] = 1000.0 + i;
        hyd.NodeDemand[i] = 2000.0 + i;
        hyd.FullDemand[i] = 3000.0 + i;
        hyd.DemandFlow[i] = 4000.0 + i;
        hyd.EmitterFlow[i] = 5000.0 + i;
        hyd.LeakageFlow[i] = 6000.0 + i;

        state.NodeHead[i] = solverSentinel;
        state.NodeDemand[i] = solverSentinel;
        state.FullDemand[i] = solverSentinel;
        state.DemandFlow[i] = solverSentinel;
        state.EmitterFlow[i] = solverSentinel;
        state.LeakageFlow[i] = solverSentinel;
    }
    for (int i = 1; i <= net.Nlinks; ++i)
    {
        hyd.LinkFlow[i] = 7000.0 + i;
        state.LinkFlow[i] = solverSentinel;
    }

    loadhydraulicsolverstate(ph);

    for (int i = 1; i <= net.Nnodes; ++i)
    {
        BOOST_CHECK_EQUAL(state.NodeHead[i], hyd.NodeHead[i] / forced.head);
        BOOST_CHECK_EQUAL(state.NodeDemand[i], solverSentinel);
    }
    for (int i = 1; i <= net.Njuncs; ++i)
    {
        BOOST_CHECK_EQUAL(state.FullDemand[i], hyd.FullDemand[i] / forced.flow);
        BOOST_CHECK_EQUAL(state.DemandFlow[i], hyd.DemandFlow[i] / forced.flow);
        BOOST_CHECK_EQUAL(state.EmitterFlow[i], hyd.EmitterFlow[i] / forced.flow);
        BOOST_CHECK_EQUAL(state.LeakageFlow[i], hyd.LeakageFlow[i] / forced.flow);
    }
    for (int i = net.Njuncs + 1; i <= net.Nnodes; ++i)
    {
        BOOST_CHECK_EQUAL(state.FullDemand[i], solverSentinel);
        BOOST_CHECK_EQUAL(state.DemandFlow[i], solverSentinel);
        BOOST_CHECK_EQUAL(state.EmitterFlow[i], solverSentinel);
        BOOST_CHECK_EQUAL(state.LeakageFlow[i], solverSentinel);
    }
    for (int i = 1; i <= net.Nlinks; ++i)
    {
        BOOST_CHECK_EQUAL(state.LinkFlow[i], hyd.LinkFlow[i] / forced.flow);
    }

    // Publishing is the inverse boundary. Node head/demand are externally
    // visible for every node. Junction-side demand components are published
    // only where those components are meaningful; fixed-grade entries remain
    // untouched on the dimensional side.
    for (int i = 1; i <= net.Nnodes; ++i)
    {
        state.NodeHead[i] = 10.0 + i;
        state.NodeDemand[i] = 20.0 + i;
        if (i <= net.Njuncs)
        {
            state.DemandFlow[i] = 30.0 + i;
            state.EmitterFlow[i] = 40.0 + i;
            state.LeakageFlow[i] = 50.0 + i;
        }
        else
        {
            hyd.DemandFlow[i] = fixedDimensionalSentinel;
            hyd.EmitterFlow[i] = fixedDimensionalSentinel;
            hyd.LeakageFlow[i] = fixedDimensionalSentinel;
        }
    }
    for (int i = 1; i <= net.Nlinks; ++i)
    {
        state.LinkFlow[i] = 60.0 + i;
    }

    savehydraulicsolverstate(ph);

    for (int i = 1; i <= net.Nnodes; ++i)
    {
        BOOST_CHECK_EQUAL(hyd.NodeHead[i], state.NodeHead[i] * forced.head);
        BOOST_CHECK_EQUAL(hyd.NodeDemand[i], state.NodeDemand[i] * forced.flow);
    }
    for (int i = 1; i <= net.Njuncs; ++i)
    {
        BOOST_CHECK_EQUAL(hyd.DemandFlow[i], state.DemandFlow[i] * forced.flow);
        BOOST_CHECK_EQUAL(hyd.EmitterFlow[i], state.EmitterFlow[i] * forced.flow);
        BOOST_CHECK_EQUAL(hyd.LeakageFlow[i], state.LeakageFlow[i] * forced.flow);
    }
    for (int i = net.Njuncs + 1; i <= net.Nnodes; ++i)
    {
        BOOST_CHECK_EQUAL(hyd.DemandFlow[i], fixedDimensionalSentinel);
        BOOST_CHECK_EQUAL(hyd.EmitterFlow[i], fixedDimensionalSentinel);
        BOOST_CHECK_EQUAL(hyd.LeakageFlow[i], fixedDimensionalSentinel);
    }
    for (int i = 1; i <= net.Nlinks; ++i)
    {
        BOOST_CHECK_EQUAL(hyd.LinkFlow[i], state.LinkFlow[i] * forced.flow);
    }

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_solver_model_tracks_consumed_toolkit_updates)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET3, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);

    // Node elevations are now consumed from SolverModel inside the GGA, so
    // Toolkit edits must refresh the compiled value immediately.
    double elevation = 0.0;
    error = EN_getnodevalue(ph, 1, EN_ELEVATION, &elevation);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 1, EN_ELEVATION, elevation + 7.0);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.NodeElevation[1],
        hydheadtosolver(ph, ph->network.Node[1].El));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.NodePdaMinGrade[1],
        hydheadtosolver(ph, ph->network.Node[1].El + ph->hydraul.Pmin));

    // Emitter coefficients are consumed directly from SolverModel and must
    // track both emitter-property and global emitter-exponent changes.
    error = EN_setnodevalue(ph, 1, EN_EMITTER, 5.0);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.NodeEmitterResistance[1],
        hydresistancetosolver(ph,
            MAX(CSMALL, ph->network.Node[1].Ke), ph->hydraul.Qexp));
    error = EN_setoption(ph, EN_EMITEXPON, 0.6);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.NodeEmitterResistance[1],
        hydresistancetosolver(ph,
            MAX(CSMALL, ph->network.Node[1].Ke), ph->hydraul.Qexp));

    // Convergence limits are also consumed from SolverModel.
    error = EN_setoption(ph, EN_HEADERROR, 1.25);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_FLOWCHANGE, 0.75);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_ACCURACY, 0.0005);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.HeadErrorLimit,
        hydheadtosolver(ph, ph->hydraul.HeadErrorLimit));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.FlowChangeLimit,
        hydflowtosolver(ph, ph->hydraul.FlowChangeLimit));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.RelativeErrorFlowCutoff,
        hydflowtosolver(ph, ph->hydraul.Hacc));

    // PDA pressure parameters can be changed while hydraulics are open.
    error = EN_setdemandmodel(ph, EN_PDA, 10.0, 30.0, 0.5);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.Pmin,
        hydheadtosolver(ph, ph->hydraul.Pmin));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.Preq,
        hydheadtosolver(ph, ph->hydraul.Preq));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.PdaPressureRange,
        hydheadtosolver(ph, MAX((ph->hydraul.Preq - ph->hydraul.Pmin), MINPDIFF)));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.NodePdaMinGrade[1],
        hydheadtosolver(ph, ph->network.Node[1].El + ph->hydraul.Pmin));

    // Static pipe coefficients are now consumed directly from SolverModel.
    // Toolkit edits that change resistance, minor loss, diameter, or viscosity
    // must refresh the compiled link immediately.
    int pipe = 0;
    for (int i = 1; i <= ph->network.Nlinks; ++i)
    {
        if (ph->network.Link[i].Type == PIPE)
        {
            pipe = i;
            break;
        }
    }
    BOOST_REQUIRE(pipe > 0);

    double value = 0.0;
    error = EN_getlinkvalue(ph, pipe, EN_LENGTH, &value);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe, EN_LENGTH, value * 1.1);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkResistance[pipe],
        hydresistancetosolver(ph, ph->network.Link[pipe].R,
            ph->hydraul.Formflag == DW ? 2.0 : ph->hydraul.Hexp));

    error = EN_getlinkvalue(ph, pipe, EN_MINORLOSS, &value);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe, EN_MINORLOSS, value + 0.25);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkMinorLoss[pipe],
        hydminorlosstosolver(ph, ph->network.Link[pipe].Km));

    error = EN_getlinkvalue(ph, pipe, EN_DIAMETER, &value);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe, EN_DIAMETER, value * 1.05);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkResistance[pipe],
        hydresistancetosolver(ph, ph->network.Link[pipe].R,
            ph->hydraul.Formflag == DW ? 2.0 : ph->hydraul.Hexp));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkMinorLoss[pipe],
        hydminorlosstosolver(ph, ph->network.Link[pipe].Km));
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkViscosityFlow[pipe],
        hydflowtosolver(ph, ph->hydraul.Viscos * ph->network.Link[pipe].Diam));

    error = EN_setoption(ph, EN_SP_VISCOS, 1.2);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkViscosityFlow[pipe],
        hydflowtosolver(ph, ph->hydraul.Viscos * ph->network.Link[pipe].Diam));

    // Simple-control trigger grades are compiled too, including controls
    // created or replaced while hydraulics are already open.
    int control = 0;
    error = EN_addcontrol(ph, EN_LOWLEVEL, pipe, 0.0, 1, elevation + 3.0,
        &control);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph->hydraul.SolverModel.ControlCapacity >= control);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.ControlGrade[control],
        hydheadtosolver(ph, ph->network.Control[control].Grade));
    error = EN_setcontrol(ph, control, EN_HILEVEL, pipe, 1.0, 1,
        elevation + 5.0);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.ControlGrade[control],
        hydheadtosolver(ph, ph->network.Control[control].Grade));

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_solver_model_tracks_dynamic_link_settings)
{
    // Pressure-control setting: compile the complete target grade and refresh
    // it both when the setting changes and when its reference elevation moves.
    {
        EN_Project ph = NULL;
        int error = EN_createproject(&ph);
        BOOST_REQUIRE(error == 0);
        error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
        BOOST_REQUIRE(error == 0);

        int link = get_link_index(ph, "121");
        double diameter = get_link_value(ph, link, EN_DIAMETER);
        error = EN_setlinktype(ph, &link, EN_PRV, EN_UNCONDITIONAL);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_DIAMETER, diameter);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_INITSETTING, 100.0);
        BOOST_REQUIRE(error == 0);
        error = EN_openH(ph);
        BOOST_REQUIRE(error == 0);
        error = EN_initH(ph, EN_INITFLOW);
        BOOST_REQUIRE(error == 0);

        error = EN_setlinkvalue(ph, link, EN_SETTING, 90.0);
        BOOST_REQUIRE(error == 0);
        Slink& prv = ph->network.Link[link];
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkSetting[link],
            hydheadtosolver(ph,
                ph->network.Node[prv.N2].El + ph->hydraul.LinkSetting[link]));

        double elevation = get_node_value(ph, prv.N2, EN_ELEVATION);
        error = EN_setnodevalue(ph, prv.N2, EN_ELEVATION, elevation + 3.0);
        BOOST_REQUIRE(error == 0);
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkSetting[link],
            hydheadtosolver(ph,
                ph->network.Node[prv.N2].El + ph->hydraul.LinkSetting[link]));

        error = EN_setlinkvalue(ph, link, EN_STATUS, 1.0);
        BOOST_REQUIRE(error == 0);
        BOOST_CHECK_EQUAL(ph->hydraul.LinkSetting[link], MISSING);
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkSetting[link], MISSING);

        error = EN_closeH(ph);
        BOOST_REQUIRE(error == 0);
        EN_close(ph);
        EN_deleteproject(ph);
    }

    // FCV settings are dimensional flows outside the solver and must be
    // compiled exactly once when a Toolkit setting changes.
    {
        EN_Project ph = NULL;
        int error = EN_createproject(&ph);
        BOOST_REQUIRE(error == 0);
        error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
        BOOST_REQUIRE(error == 0);

        int link = get_link_index(ph, "121");
        double diameter = get_link_value(ph, link, EN_DIAMETER);
        error = EN_setlinktype(ph, &link, EN_FCV, EN_UNCONDITIONAL);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_DIAMETER, diameter);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_INITSETTING, 100.0);
        BOOST_REQUIRE(error == 0);
        error = EN_openH(ph);
        BOOST_REQUIRE(error == 0);
        error = EN_initH(ph, EN_INITFLOW);
        BOOST_REQUIRE(error == 0);

        error = EN_setlinkvalue(ph, link, EN_SETTING, 150.0);
        BOOST_REQUIRE(error == 0);
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkSetting[link],
            hydflowtosolver(ph, ph->hydraul.LinkSetting[link]));

        error = EN_closeH(ph);
        BOOST_REQUIRE(error == 0);
        EN_close(ph);
        EN_deleteproject(ph);
    }

    // Pump speed changes also refresh the precompiled speed-adjusted maximum
    // head used by iterative status checks.
    {
        EN_Project ph = NULL;
        int error = EN_createproject(&ph);
        BOOST_REQUIRE(error == 0);
        error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
        BOOST_REQUIRE(error == 0);
        error = EN_openH(ph);
        BOOST_REQUIRE(error == 0);
        error = EN_initH(ph, EN_INITFLOW);
        BOOST_REQUIRE(error == 0);

        int link = get_link_index(ph, "9");
        error = EN_setlinkvalue(ph, link, EN_SETTING, 0.8);
        BOOST_REQUIRE(error == 0);
        int p = findpump(&ph->network, link);
        if (ph->network.Pump[p].Ptype == CONST_HP)
        {
            BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkPumpMaxHead[link],
                ph->hydraul.SolverModel.BigHead);
        }
        else
        {
            BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkPumpMaxHead[link],
                hydheadtosolver(ph, SQR(ph->hydraul.LinkSetting[link]) *
                    ph->network.Pump[p].Hmax));
        }

        error = EN_closeH(ph);
        BOOST_REQUIRE(error == 0);
        EN_close(ph);
        EN_deleteproject(ph);
    }

    // TCV/PCV settings are dimensionless, but their setting-dependent loss
    // coefficients are compiled into solver units at the same mutation point.
    const int valveTypes[] = {EN_TCV, EN_PCV};
    for (int type : valveTypes)
    {
        EN_Project ph = NULL;
        int error = EN_createproject(&ph);
        BOOST_REQUIRE(error == 0);
        error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
        BOOST_REQUIRE(error == 0);

        int link = get_link_index(ph, "22");
        error = EN_setlinktype(ph, &link, type, EN_UNCONDITIONAL);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_DIAMETER, 12.0);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_MINORLOSS, 0.19);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, link, EN_INITSETTING, 35.0);
        BOOST_REQUIRE(error == 0);
        error = EN_openH(ph);
        BOOST_REQUIRE(error == 0);
        error = EN_initH(ph, EN_INITFLOW);
        BOOST_REQUIRE(error == 0);

        error = EN_setlinkvalue(ph, link, EN_SETTING, 45.0);
        BOOST_REQUIRE(error == 0);
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkSetting[link],
            ph->hydraul.LinkSetting[link]);

        double expectedLoss = 0.0;
        if (type == EN_TCV)
        {
            Slink& tcv = ph->network.Link[link];
            const double km = 0.02517 * ph->hydraul.LinkSetting[link] /
                (SQR(tcv.Diam) * SQR(tcv.Diam));
            expectedLoss = hydminorlosstosolver(ph, km);
        }
        else
        {
            expectedLoss = hydminorlosstosolver(ph, ph->network.Link[link].R);
        }
        BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.LinkDynamicLoss[link],
            expectedLoss);

        error = EN_closeH(ph);
        BOOST_REQUIRE(error == 0);
        EN_close(ph);
        EN_deleteproject(ph);
    }
}


BOOST_AUTO_TEST_CASE(test_whole_model_compile_skips_nonhydraulic_curves)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // A generic curve is not a GGA flow/head relation and should not receive
    // solver-space segment storage during whole-model compilation.
    error = EN_addcurve(ph, "non-hydraulic-curve");
    BOOST_REQUIRE(error == 0);
    int curve = 0;
    error = EN_getcurveindex(ph, "non-hydraulic-curve", &curve);
    BOOST_REQUIRE(error == 0);
    double x[] = {0.0, 1.0, 2.0};
    double y[] = {1.0, 2.0, 3.0};
    error = EN_setcurve(ph, curve, x, y, 3);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(ph->hydraul.SolverModel.CurveCapacity >= curve);
    BOOST_CHECK_EQUAL(ph->hydraul.SolverModel.Curve[curve].Npts, 0);
    BOOST_CHECK(ph->hydraul.SolverModel.Curve[curve].X == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.Curve[curve].H0 == NULL);
    BOOST_CHECK(ph->hydraul.SolverModel.Curve[curve].R == NULL);

    // Changing the same curve to a GPV headloss curve while hydraulics are
    // open compiles it immediately through the existing curve mutation path.
    error = EN_setcurvetype(ph, curve, EN_HLOSS_CURVE);
    BOOST_REQUIRE(error == 0);
    check_compiled_curve(ph, curve);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    EN_close(ph);
    EN_deleteproject(ph);
}

BOOST_AUTO_TEST_CASE(test_solver_model_tracks_live_curve_edits)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, EXAMPLE_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Make Net1's pump curve a genuine multi-segment custom curve before the
    // hydraulic model is compiled.
    double pumpX[] = {0.0, 1000.0, 2200.0, 4000.0};
    double pumpY[] = {300.0, 270.0, 190.0, 70.0};
    error = EN_setcurve(ph, 1, pumpX, pumpY, 4);
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);
    check_compiled_curve(ph, 1);

    const double oldSlope = ph->hydraul.SolverModel.Curve[1].R[0];
    error = EN_setcurvevalue(ph, 1, 2, 1000.0, 250.0);
    BOOST_REQUIRE(error == 0);
    check_compiled_curve(ph, 1);
    BOOST_CHECK(ph->hydraul.SolverModel.Curve[1].R[0] != oldSlope);

    // Curves can be added while hydraulics are open. Setting their data must
    // grow the solver cache and compile the new curve immediately.
    error = EN_addcurve(ph, "live-gpv-curve");
    BOOST_REQUIRE(error == 0);
    int curve = 0;
    error = EN_getcurveindex(ph, "live-gpv-curve", &curve);
    BOOST_REQUIRE(error == 0);
    double gpvX[] = {0.0, 200.0, 800.0, 2000.0};
    double gpvY[] = {0.0, 2.0, 25.0, 120.0};
    error = EN_setcurve(ph, curve, gpvX, gpvY, 4);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurvetype(ph, curve, EN_HLOSS_CURVE);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph->hydraul.SolverModel.CurveCapacity >= curve);
    check_compiled_curve(ph, curve);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    EN_close(ph);
    EN_deleteproject(ph);
}


BOOST_AUTO_TEST_CASE(test_production_scale_is_model_based_and_unit_independent)
{
    const SolverScale reference = get_production_scale(EN_CFS, "CFS");
    const int flowUnits[] = {
        EN_GPM, EN_MGD, EN_IMGD, EN_AFD, EN_LPS, EN_LPM,
        EN_MLD, EN_CMH, EN_CMD, EN_CMS
    };

    BOOST_CHECK(reference.head > 0.0);
    BOOST_CHECK(reference.flow > 0.0);
    BOOST_CHECK_MESSAGE(reference.head != 1.0 || reference.flow != 1.0,
        "Net1 production solver scale should not remain the legacy 1/1 pair");

    for (int units : flowUnits)
    {
        const SolverScale actual = get_production_scale(units, "alternate units");
        BOOST_CHECK_EQUAL(actual.head, reference.head);
        BOOST_CHECK_EQUAL(actual.flow, reference.flow);
    }
}

BOOST_AUTO_TEST_CASE(test_production_scaling_preserves_example_network_results)
{
    struct ExampleNetwork
    {
        const char* path;
        const char* name;
    };
    const ExampleNetwork examples[] = {
        {EXAMPLE_NET1, "Net1"},
        {EXAMPLE_NET2, "Net2"},
        {EXAMPLE_NET3, "Net3"}
    };

    for (const ExampleNetwork& example : examples)
    {
        const std::vector<CompatibilitySnapshot> expected =
            solve_example_eps(example.path, true);
        const std::vector<CompatibilitySnapshot> actual =
            solve_example_eps(example.path, false);
        compare_example_eps(actual, expected, example.name);
    }
}

BOOST_AUTO_TEST_CASE(test_net1_eps_is_invariant_to_solver_scaling)
{
    const std::vector<EpsSnapshot> expected = solve_net1_eps(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        compare_eps(solve_net1_eps(SCALES[i]), expected, SCALES[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_low_flow_convergence_is_invariant_to_flow_scaling)
{
    const ConvergenceSnapshot expected = solve_net1_convergence_case(1.0);
    const ConvergenceSnapshot actual = solve_net1_convergence_case(1.e6);

    BOOST_CHECK_EQUAL(actual.iterations, expected.iterations);
    BOOST_CHECK_SMALL(actual.relativeError - expected.relativeError, 1.e-12);
    BOOST_REQUIRE_EQUAL(actual.nodeHead.size(), expected.nodeHead.size());
    BOOST_REQUIRE_EQUAL(actual.linkFlow.size(), expected.linkFlow.size());
    for (std::size_t i = 0; i < expected.nodeHead.size(); ++i)
    {
        BOOST_CHECK_SMALL(actual.nodeHead[i] - expected.nodeHead[i], HEAD_TOL);
    }
    for (std::size_t i = 0; i < expected.linkFlow.size(); ++i)
    {
        BOOST_CHECK_SMALL(actual.linkFlow[i] - expected.linkFlow[i], FLOW_TOL);
    }
}

BOOST_AUTO_TEST_CASE(test_nonlinear_features_are_invariant_to_solver_scaling)
{
    const SpecialSnapshot expected = solve_special_case(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        compare_special(solve_special_case(SCALES[i]), expected, SCALES[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_custom_pump_and_gpv_curves_are_invariant_to_solver_scaling)
{
    const CurveSnapshot expected = solve_curve_case(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        const CurveSnapshot actual = solve_curve_case(SCALES[i]);
        check_near(actual.pumpFlow, expected.pumpFlow, SPECIAL_TOL,
            SCALES[i], "custom-pump flow");
        check_near(actual.pumpDischargeHead, expected.pumpDischargeHead,
            SPECIAL_TOL, SCALES[i], "custom-pump discharge head");
        check_near(actual.gpvFlow, expected.gpvFlow, SPECIAL_TOL,
            SCALES[i], "GPV flow");
        check_near(actual.gpvHeadloss, expected.gpvHeadloss, SPECIAL_TOL,
            SCALES[i], "GPV headloss");
    }
}


BOOST_AUTO_TEST_CASE(test_pressure_valve_is_invariant_to_solver_scaling)
{
    const PrvSnapshot expected = solve_prv_case(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        const PrvSnapshot actual = solve_prv_case(SCALES[i]);
        check_near(actual.node31Head, expected.node31Head, HEAD_TOL,
            SCALES[i], "PRV node head");
        check_near(actual.node31Pressure, expected.node31Pressure, HEAD_TOL,
            SCALES[i], "PRV node pressure");
        check_near(actual.checkValveFlow, expected.checkValveFlow, FLOW_TOL,
            SCALES[i], "check-valve flow");
        check_near(actual.prvFlow, expected.prvFlow, FLOW_TOL,
            SCALES[i], "PRV flow");
        BOOST_CHECK_MESSAGE(actual.prvStatus == expected.prvStatus,
            SCALES[i].name << ": PRV status expected " << expected.prvStatus <<
            ", got " << actual.prvStatus);
    }
}

BOOST_AUTO_TEST_CASE(test_dimensional_consumers_are_invariant_to_solver_scaling)
{
    const BoundarySnapshot expected = solve_boundary_case(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        const BoundarySnapshot actual = solve_boundary_case(SCALES[i]);
        check_near(actual.pumpEnergy, expected.pumpEnergy, QUALITY_TOL,
            SCALES[i], "pump energy");
        check_near(actual.pumpEfficiency, expected.pumpEfficiency, QUALITY_TOL,
            SCALES[i], "pump efficiency");
        check_near(actual.quality21, expected.quality21, QUALITY_TOL,
            SCALES[i], "node 21 quality");
        check_near(actual.quality32, expected.quality32, QUALITY_TOL,
            SCALES[i], "node 32 quality");
        check_near(actual.massBalance, expected.massBalance, QUALITY_TOL,
            SCALES[i], "quality mass balance");
    }
}

BOOST_AUTO_TEST_SUITE_END()
