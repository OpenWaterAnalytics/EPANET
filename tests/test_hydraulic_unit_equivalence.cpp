/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_hydraulic_unit_equivalence.cpp
 Description:  Cross-unit equivalence tests for hydraulic solver results
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 ******************************************************************************
*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

namespace
{

const double M_PER_FT = 0.3048;
const double M3_PER_FT3 = M_PER_FT * M_PER_FT * M_PER_FT;

const double HEAD_TOL_FT = 1.e-6;
const double FLOW_TOL_CFS = 1.e-9;
const double VOLUME_TOL_FT3 = 2.e-4;
const double VELOCITY_TOL_FPS = 1.e-9;
const double PRESSURE_TOL_PSI = 1.e-6;
const double REL_TOL = 1.e-7;

struct FlowUnit
{
    int code;
    const char* name;
    double perCfs;
    bool metric;
};

const FlowUnit FLOW_UNITS[] = {
    {EN_CFS,  "CFS",  1.0,       false},
    {EN_GPM,  "GPM",  448.831,   false},
    {EN_MGD,  "MGD",  0.64632,   false},
    {EN_IMGD, "IMGD", 0.5382,    false},
    {EN_AFD,  "AFD",  1.9837,    false},
    {EN_LPS,  "LPS",  28.317,    true},
    {EN_LPM,  "LPM",  1699.0,    true},
    {EN_MLD,  "MLD",  2.4466,    true},
    {EN_CMH,  "CMH",  101.94,    true},
    {EN_CMD,  "CMD",  2446.6,    true},
    {EN_CMS,  "CMS",  0.028317,  true}
};

struct HydraulicSnapshot
{
    long time;
    double node21HeadFt;
    double node21PressurePsi;
    double node21DemandCfs;
    double node22HeadFt;
    double tankHeadFt;
    double tankLevelFt;
    double tankVolumeFt3;
    double pumpFlowCfs;
    int pumpStatus;
    int pumpState;
    double pipe22FlowCfs;
    double pipe22VelocityFps;
};

int get_node_index(EN_Project ph, const char* id)
{
    int index = 0;
    int error = EN_getnodeindex(ph, id, &index);
    BOOST_REQUIRE(error == 0);
    return index;
}

int get_link_index(EN_Project ph, const char* id)
{
    int index = 0;
    int error = EN_getlinkindex(ph, id, &index);
    BOOST_REQUIRE(error == 0);
    return index;
}

double get_node_value(EN_Project ph, int index, int property)
{
    double value = 0.0;
    int error = EN_getnodevalue(ph, index, property, &value);
    BOOST_REQUIRE(error == 0);
    return value;
}

double get_link_value(EN_Project ph, int index, int property)
{
    double value = 0.0;
    int error = EN_getlinkvalue(ph, index, property, &value);
    BOOST_REQUIRE(error == 0);
    return value;
}

double head_to_ft(double value, const FlowUnit& units)
{
    if (units.metric) return value / M_PER_FT;
    return value;
}

double volume_to_ft3(double value, const FlowUnit& units)
{
    if (units.metric) return value / M3_PER_FT3;
    return value;
}

double velocity_to_fps(double value, const FlowUnit& units)
{
    if (units.metric) return value / M_PER_FT;
    return value;
}

double flow_to_cfs(double value, const FlowUnit& units)
{
    return value / units.perCfs;
}

std::vector<HydraulicSnapshot> solve_net1(const FlowUnit& units)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    error = EN_setflowunits(ph, units.code);
    BOOST_REQUIRE(error == 0);

    // Keep pressure in one common public unit while the flow unit system changes.
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_PSI);
    BOOST_REQUIRE(error == 0);

    const int node21 = get_node_index(ph, "21");
    const int node22 = get_node_index(ph, "22");
    const int tank2 = get_node_index(ph, "2");
    const int pump9 = get_link_index(ph, "9");
    const int pipe22 = get_link_index(ph, "22");

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    std::vector<HydraulicSnapshot> snapshots;
    long time = 0;
    long timeStep = 0;

    do
    {
        error = EN_runH(ph, &time);
        BOOST_REQUIRE(error == 0);

        HydraulicSnapshot snapshot;
        snapshot.time = time;
        snapshot.node21HeadFt = head_to_ft(
            get_node_value(ph, node21, EN_HEAD), units);
        snapshot.node21PressurePsi = get_node_value(ph, node21, EN_PRESSURE);
        snapshot.node21DemandCfs = flow_to_cfs(
            get_node_value(ph, node21, EN_DEMAND), units);
        snapshot.node22HeadFt = head_to_ft(
            get_node_value(ph, node22, EN_HEAD), units);
        snapshot.tankHeadFt = head_to_ft(
            get_node_value(ph, tank2, EN_HEAD), units);
        snapshot.tankLevelFt = head_to_ft(
            get_node_value(ph, tank2, EN_TANKLEVEL), units);
        snapshot.tankVolumeFt3 = volume_to_ft3(
            get_node_value(ph, tank2, EN_TANKVOLUME), units);
        snapshot.pumpFlowCfs = flow_to_cfs(
            get_link_value(ph, pump9, EN_FLOW), units);
        snapshot.pumpStatus = static_cast<int>(
            get_link_value(ph, pump9, EN_STATUS));
        snapshot.pumpState = static_cast<int>(
            get_link_value(ph, pump9, EN_PUMP_STATE));
        snapshot.pipe22FlowCfs = flow_to_cfs(
            get_link_value(ph, pipe22, EN_FLOW), units);
        snapshot.pipe22VelocityFps = velocity_to_fps(
            get_link_value(ph, pipe22, EN_VELOCITY), units);
        snapshots.push_back(snapshot);

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

    return snapshots;
}

void check_near(double actual, double expected, double absTolerance,
    const FlowUnit& units, long time, const char* quantity)
{
    const double scale = std::max(std::fabs(actual), std::fabs(expected));
    const double tolerance = std::max(absTolerance, REL_TOL * scale);
    BOOST_CHECK_MESSAGE(std::fabs(actual - expected) <= tolerance,
        units.name << " at t=" << time << " s: " << quantity <<
        " expected " << expected << ", got " << actual <<
        " (tolerance " << tolerance << ")");
}

void compare_snapshots(const std::vector<HydraulicSnapshot>& actual,
    const std::vector<HydraulicSnapshot>& expected, const FlowUnit& units)
{
    BOOST_REQUIRE_MESSAGE(actual.size() == expected.size(),
        units.name << ": expected " << expected.size() <<
        " hydraulic events, got " << actual.size());

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const HydraulicSnapshot& a = actual[i];
        const HydraulicSnapshot& e = expected[i];

        BOOST_CHECK_MESSAGE(a.time == e.time,
            units.name << ": hydraulic event " << i << " expected at t=" <<
            e.time << " s, got t=" << a.time << " s");

        check_near(a.node21HeadFt, e.node21HeadFt, HEAD_TOL_FT,
            units, e.time, "node 21 head (ft)");
        check_near(a.node21PressurePsi, e.node21PressurePsi, PRESSURE_TOL_PSI,
            units, e.time, "node 21 pressure (psi)");
        check_near(a.node21DemandCfs, e.node21DemandCfs, FLOW_TOL_CFS,
            units, e.time, "node 21 demand (cfs)");
        check_near(a.node22HeadFt, e.node22HeadFt, HEAD_TOL_FT,
            units, e.time, "node 22 head (ft)");
        check_near(a.tankHeadFt, e.tankHeadFt, HEAD_TOL_FT,
            units, e.time, "tank 2 head (ft)");
        check_near(a.tankLevelFt, e.tankLevelFt, HEAD_TOL_FT,
            units, e.time, "tank 2 level (ft)");
        check_near(a.tankVolumeFt3, e.tankVolumeFt3, VOLUME_TOL_FT3,
            units, e.time, "tank 2 volume (ft3)");
        check_near(a.pumpFlowCfs, e.pumpFlowCfs, FLOW_TOL_CFS,
            units, e.time, "pump 9 flow (cfs)");
        check_near(a.pipe22FlowCfs, e.pipe22FlowCfs, FLOW_TOL_CFS,
            units, e.time, "pipe 22 flow (cfs)");
        check_near(a.pipe22VelocityFps, e.pipe22VelocityFps, VELOCITY_TOL_FPS,
            units, e.time, "pipe 22 velocity (ft/s)");

        BOOST_CHECK_MESSAGE(a.pumpStatus == e.pumpStatus,
            units.name << " at t=" << e.time << " s: pump 9 status expected " <<
            e.pumpStatus << ", got " << a.pumpStatus);
        BOOST_CHECK_MESSAGE(a.pumpState == e.pumpState,
            units.name << " at t=" << e.time << " s: pump 9 state expected " <<
            e.pumpState << ", got " << a.pumpState);
    }
}

} // namespace


BOOST_AUTO_TEST_SUITE(test_hydraulic_unit_equivalence)

BOOST_AUTO_TEST_CASE(test_net1_eps_is_equivalent_in_all_flow_unit_systems)
{
    const std::vector<HydraulicSnapshot> reference = solve_net1(FLOW_UNITS[0]);
    const std::size_t unitCount = sizeof(FLOW_UNITS) / sizeof(FLOW_UNITS[0]);

    BOOST_REQUIRE(!reference.empty());

    for (std::size_t i = 1; i < unitCount; ++i)
    {
        const std::vector<HydraulicSnapshot> actual = solve_net1(FLOW_UNITS[i]);
        compare_snapshots(actual, reference, FLOW_UNITS[i]);
    }
}

BOOST_AUTO_TEST_SUITE_END()
