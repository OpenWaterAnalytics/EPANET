/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_hydraulic_characterization.cpp
 Description:  Characterization tests for current hydraulic solver behavior
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 ******************************************************************************
*/

#include <cmath>

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

namespace
{

const double HEAD_TOL = 1.e-5;
const double FLOW_TOL = 1.e-5;
const double VOLUME_TOL = 1.e-3;

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

double get_statistic(EN_Project ph, int statistic)
{
    double value = 0.0;
    int error = EN_getstatistic(ph, statistic, &value);
    BOOST_REQUIRE(error == 0);
    return value;
}

void check_near(double actual, double expected, double tolerance,
    const char* quantity)
{
    BOOST_CHECK_MESSAGE(std::fabs(actual - expected) <= tolerance,
        quantity << ": expected " << expected << ", got " << actual);
}

struct EpsSnapshot
{
    long time;
    double node12Head;
    double node21Head;
    double node32Head;
    double tankHead;
    double tankVolume;
    double pumpFlow;
    int pumpStatus;
    double pipe22Flow;
    double pipe121Flow;
};

const EpsSnapshot NET1_EPS_BASELINE[] = {
    {0,     970.069822265, 971.546635491, 965.689326351, 970.000000000,
             240355.399945, 1866.17582999, 1, 120.664865410, 140.810519126},
    {21600, 982.377202012, 977.060782670, 965.872506661, 982.376703669,
             265145.462948, 1813.12872280, 1, 142.193909778, 209.989315046},
    {43200, 988.624439509, 989.422837016, 983.832835608, 988.571911344,
             277554.226434, 1757.03555295, 1, 115.839822875, 138.752638016},
    {64800, 971.222198385, 970.410396302, 969.603968836, 971.247195702,
             242853.485127,    0.00000000, 0,  24.572981453,  51.467088692},
    {86400, 965.476349923, 967.121091206, 961.194579712, 965.402064491,
             231145.911376, 1892.24322655, 1, 121.813754620, 141.333170673}
};

void characterize_headloss_formula(int headlossForm, double roughness,
    double expectedNode22Head, double expectedPipe22Flow,
    double expectedPipe22Headloss)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_HEADLOSSFORM, headlossForm);
    BOOST_REQUIRE(error == 0);

    int linkCount = 0;
    error = EN_getcount(ph, EN_LINKCOUNT, &linkCount);
    BOOST_REQUIRE(error == 0);

    for (int i = 1; i <= linkCount; ++i)
    {
        int linkType = 0;
        error = EN_getlinktype(ph, i, &linkType);
        BOOST_REQUIRE(error == 0);
        if (linkType == EN_PIPE || linkType == EN_CVPIPE)
        {
            error = EN_setlinkvalue(ph, i, EN_ROUGHNESS, roughness);
            BOOST_REQUIRE(error == 0);
        }
    }

    const int node22 = get_node_index(ph, "22");
    const int pipe22 = get_link_index(ph, "22");

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_node_value(ph, node22, EN_HEAD), expectedNode22Head,
        HEAD_TOL, "node 22 head");
    check_near(get_link_value(ph, pipe22, EN_FLOW), expectedPipe22Flow,
        FLOW_TOL, "pipe 22 flow");
    check_near(get_link_value(ph, pipe22, EN_HEADLOSS), expectedPipe22Headloss,
        HEAD_TOL, "pipe 22 headloss");

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}

} // namespace


BOOST_AUTO_TEST_SUITE(test_hydraulic_characterization)

BOOST_AUTO_TEST_CASE(test_net1_extended_period_baseline)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int node12 = get_node_index(ph, "12");
    const int node21 = get_node_index(ph, "21");
    const int node32 = get_node_index(ph, "32");
    const int tank2 = get_node_index(ph, "2");
    const int pump9 = get_link_index(ph, "9");
    const int pipe22 = get_link_index(ph, "22");
    const int pipe121 = get_link_index(ph, "121");

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    long time = 0;
    long timeStep = 0;
    std::size_t snapshot = 0;
    const std::size_t snapshotCount =
        sizeof(NET1_EPS_BASELINE) / sizeof(NET1_EPS_BASELINE[0]);

    do
    {
        error = EN_runH(ph, &time);
        BOOST_REQUIRE(error == 0);

        if (snapshot < snapshotCount &&
            time == NET1_EPS_BASELINE[snapshot].time)
        {
            const EpsSnapshot& expected = NET1_EPS_BASELINE[snapshot];

            check_near(get_node_value(ph, node12, EN_HEAD), expected.node12Head,
                HEAD_TOL, "node 12 head");
            check_near(get_node_value(ph, node21, EN_HEAD), expected.node21Head,
                HEAD_TOL, "node 21 head");
            check_near(get_node_value(ph, node32, EN_HEAD), expected.node32Head,
                HEAD_TOL, "node 32 head");
            check_near(get_node_value(ph, tank2, EN_HEAD), expected.tankHead,
                HEAD_TOL, "tank 2 head");
            check_near(get_node_value(ph, tank2, EN_TANKVOLUME),
                expected.tankVolume, VOLUME_TOL, "tank 2 volume");
            check_near(get_link_value(ph, pump9, EN_FLOW), expected.pumpFlow,
                FLOW_TOL, "pump 9 flow");
            check_near(get_link_value(ph, pipe22, EN_FLOW), expected.pipe22Flow,
                FLOW_TOL, "pipe 22 flow");
            check_near(get_link_value(ph, pipe121, EN_FLOW), expected.pipe121Flow,
                FLOW_TOL, "pipe 121 flow");

            const int pumpStatus = static_cast<int>(
                get_link_value(ph, pump9, EN_STATUS));
            BOOST_CHECK_EQUAL(pumpStatus, expected.pumpStatus);

            ++snapshot;
        }

        error = EN_nextH(ph, &timeStep);
        BOOST_REQUIRE(error == 0);
    }
    while (timeStep > 0);

    BOOST_CHECK_EQUAL(snapshot, snapshotCount);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_pressure_driven_demand_baseline)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int node21 = get_node_index(ph, "21");
    const int pipe21 = get_link_index(ph, "21");

    error = EN_setoption(ph, EN_DEMANDMULT, 10.0);
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setdemandmodel(ph, EN_PDA, 20.0, 100.0, 0.5);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_node_value(ph, node21, EN_HEAD), 842.993722613,
        HEAD_TOL, "PDA node 21 head");
    check_near(get_node_value(ph, node21, EN_PRESSURE), 61.9591800082,
        HEAD_TOL, "PDA node 21 pressure");
    check_near(get_node_value(ph, node21, EN_DEMAND), 1086.32497638,
        FLOW_TOL, "PDA node 21 delivered demand");
    check_near(get_node_value(ph, node21, EN_FULLDEMAND), 1500.0,
        FLOW_TOL, "PDA node 21 full demand");
    check_near(get_node_value(ph, node21, EN_DEMANDDEFICIT), 413.675023617,
        FLOW_TOL, "PDA node 21 demand deficit");
    check_near(get_link_value(ph, pipe21, EN_FLOW), -139.153545830,
        FLOW_TOL, "PDA pipe 21 flow");
    check_near(get_statistic(ph, EN_DEFICIENTNODES), 6.0,
        1.e-12, "PDA deficient node count");
    check_near(get_statistic(ph, EN_DEMANDREDUCTION), 32.6588656951,
        1.e-6, "PDA demand reduction");

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_emitter_baseline)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int node21 = get_node_index(ph, "21");
    const int pipe21 = get_link_index(ph, "21");

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, node21, EN_EMITTER, 5.0);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_node_value(ph, node21, EN_HEAD), 970.431762072,
        HEAD_TOL, "emitter node 21 head");
    check_near(get_node_value(ph, node21, EN_PRESSURE), 117.178082506,
        HEAD_TOL, "emitter node 21 pressure");
    check_near(get_node_value(ph, node21, EN_EMITTERFLOW), 54.1244322031,
        FLOW_TOL, "node 21 emitter flow");
    check_near(get_link_value(ph, pipe21, EN_FLOW), 155.493612066,
        FLOW_TOL, "emitter case pipe 21 flow");

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_pipe_leakage_baseline)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int node21 = get_node_index(ph, "21");
    const int node22 = get_node_index(ph, "22");
    const int pipe21 = get_link_index(ph, "21");

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe21, EN_LEAK_AREA, 1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pipe21, EN_LEAK_EXPAN, 0.1);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_link_value(ph, pipe21, EN_FLOW), 138.262509077,
        FLOW_TOL, "leaking pipe 21 flow");
    check_near(get_link_value(ph, pipe21, EN_LINK_LEAKAGE), 187.000855580,
        FLOW_TOL, "pipe 21 leakage");
    check_near(get_node_value(ph, node21, EN_HEAD), 968.933973747,
        HEAD_TOL, "leakage node 21 head");
    check_near(get_node_value(ph, node22, EN_HEAD), 967.579288090,
        HEAD_TOL, "leakage node 22 head");
    check_near(get_node_value(ph, node21, EN_LEAKAGEFLOW), 92.6245504158,
        FLOW_TOL, "node 21 leakage flow");
    check_near(get_node_value(ph, node22, EN_LEAKAGEFLOW), 94.3767521343,
        FLOW_TOL, "node 22 leakage flow");

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_prv_baseline)
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
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Link indices can change when link types are replaced.
    pipe113 = get_link_index(ph, "113");
    pipe121 = get_link_index(ph, "121");

    check_near(get_node_value(ph, node31, EN_HEAD), 930.786983614,
        HEAD_TOL, "PRV node 31 head");
    check_near(get_node_value(ph, node31, EN_PRESSURE), 100.0,
        HEAD_TOL, "PRV node 31 pressure");
    check_near(get_link_value(ph, pipe113, EN_FLOW), 36.7107145791,
        FLOW_TOL, "check-valve pipe 113 flow");
    check_near(get_link_value(ph, pipe121, EN_FLOW), 7.15680063758,
        FLOW_TOL, "PRV 121 flow");
    BOOST_CHECK_EQUAL(static_cast<int>(get_link_value(ph, pipe121, EN_STATUS)), 2);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_pump_energy_uses_dimensional_hydraulic_results)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int pump9 = get_link_index(ph, "9");

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_link_value(ph, pump9, EN_ENERGY), 95.8448203536107,
        1.e-9, "pump 9 energy");
    check_near(get_link_value(ph, pump9, EN_PUMP_EFFIC), 0.75,
        1.e-12, "pump 9 efficiency");

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_constant_power_pump_baseline)
{
    EN_Project ph = NULL;
    int error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    const int pump9 = get_link_index(ph, "9");
    const int node10 = get_node_index(ph, "10");

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, pump9, EN_PUMP_POWER, 100.0);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    check_near(get_link_value(ph, pump9, EN_FLOW), 1917.79333392,
        FLOW_TOL, "constant-power pump flow");
    check_near(get_link_value(ph, pump9, EN_HEADLOSS), -206.278557931,
        HEAD_TOL, "constant-power pump head gain");
    check_near(get_node_value(ph, node10, EN_HEAD), 1006.27855793,
        HEAD_TOL, "constant-power pump discharge head");
    BOOST_CHECK_EQUAL(static_cast<int>(get_link_value(ph, pump9, EN_PUMP_STATE)),
        EN_PUMP_OPEN);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_darcy_weisbach_baseline)
{
    characterize_headloss_formula(EN_DW, 1.0,
        969.552798995, 125.059684945, 0.266395271059);
}


BOOST_AUTO_TEST_CASE(test_chezy_manning_baseline)
{
    characterize_headloss_formula(EN_CM, 0.013,
        969.343587737, 122.395933959, 0.307526513073);
}

BOOST_AUTO_TEST_SUITE_END()
