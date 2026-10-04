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

namespace
{

const double HEAD_TOL = 1.e-6;
const double FLOW_TOL = 1.e-5;
const double VOLUME_TOL = 1.e-3;
const double SPECIAL_TOL = 1.e-7;
const double QUALITY_TOL = 1.e-9;

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
    ph->hydraul.SolverScale.Head = scale.head;
    ph->hydraul.SolverScale.Flow = scale.flow;
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

    set_solver_scale(ph, scale);

    int nodeCount = 0;
    int linkCount = 0;
    error = EN_getcount(ph, EN_NODECOUNT, &nodeCount);
    BOOST_REQUIRE(error == 0);
    error = EN_getcount(ph, EN_LINKCOUNT, &linkCount);
    BOOST_REQUIRE(error == 0);
    const int tank2 = get_node_index(ph, "2");

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
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

    set_solver_scale(ph, scale);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

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

    set_solver_scale(ph, scale);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

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
    set_solver_scale(ph, scale);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);
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
    set_solver_scale(ph, scale);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);
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

const SolverScale LEGACY_SCALE = {1.0, 1.0, "legacy"};

} // namespace


BOOST_AUTO_TEST_SUITE(test_hydraulic_solver_scaling)

BOOST_AUTO_TEST_CASE(test_net1_eps_is_invariant_to_solver_scaling)
{
    const std::vector<EpsSnapshot> expected = solve_net1_eps(LEGACY_SCALE);
    const std::size_t scaleCount = sizeof(SCALES) / sizeof(SCALES[0]);

    for (std::size_t i = 0; i < scaleCount; ++i)
    {
        compare_eps(solve_net1_eps(SCALES[i]), expected, SCALES[i]);
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
