/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_hydraulic_performance_guardrails.cpp
 Description:  Deterministic guardrails for hydraulic performance work
 Authors:      see AUTHORS
 Copyright:    see COPYRIGHT
 License:      see LICENSE
 ******************************************************************************
*/

#include <algorithm>
#include <cmath>

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

namespace
{

struct HydraulicBaseline
{
    const char* name;
    const char* path;
    long finalTime;
    long eventCount;
    long maxTotalIterations;
    long maxIterationsPerEvent;
    double finalHeadSum;
    double finalFlowSum;
    double finalHeadWeightedSum;
    double finalFlowWeightedSum;
};

struct HydraulicSignature
{
    long finalTime;
    long eventCount;
    long totalIterations;
    long maxIterationsPerEvent;
    double maxRelativeError;
    double maxHeadError;
    double maxFlowChange;
    double finalHeadSum;
    double finalFlowSum;
    double finalHeadWeightedSum;
    double finalFlowWeightedSum;
};

const HydraulicBaseline BASELINES[] = {
    {
        "Net1", EPANET_SOURCE_DIR "/example-networks/Net1.inp",
        86400, 27, 69, 15,
        10496.8693609944, 5634.4864524726,
        62061.6139682694, 33580.9711499533
    },
    {
        "Net2", EPANET_SOURCE_DIR "/example-networks/Net2.inp",
        198000, 56, 145, 5,
        10887.7112752713, 7330.8731654847,
        199956.564865858, 88776.0006709765
    },
    {
        "Net3", EPANET_SOURCE_DIR "/example-networks/Net3.inp",
        86400, 27, 86, 7,
        14802.4142303076, 142940.972520975,
        707053.285660327, 8198633.90500753
    },
    {
        "Grid20", EPANET_SOURCE_DIR "/benchmarks/data/Grid20.inp",
        0, 1, 4, 4,
        87578.7786800588, 9600.00000014587,
        17601670.1664525, 2758880.00003285
    }
};

void require_ok(int error)
{
    BOOST_REQUIRE_EQUAL(error, 0);
}

double statistic(EN_Project ph, int type)
{
    double value = 0.0;
    require_ok(EN_getstatistic(ph, type, &value));
    return value;
}

HydraulicSignature run_hydraulics(const HydraulicBaseline& baseline)
{
    HydraulicSignature result = {};
    EN_Project ph = NULL;
    require_ok(EN_createproject(&ph));
    require_ok(EN_open(ph, baseline.path, DATA_PATH_RPT, ""));
    require_ok(EN_openH(ph));
    require_ok(EN_initH(ph, EN_INITFLOW));

    long time = 0;
    long timeStep = 0;
    do
    {
        require_ok(EN_runH(ph, &time));
        ++result.eventCount;
        result.finalTime = time;

        const long iterations = static_cast<long>(
            std::llround(statistic(ph, EN_ITERATIONS)));
        result.totalIterations += iterations;
        result.maxIterationsPerEvent = std::max(
            result.maxIterationsPerEvent, iterations);
        result.maxRelativeError = std::max(result.maxRelativeError,
            std::fabs(statistic(ph, EN_RELATIVEERROR)));
        result.maxHeadError = std::max(result.maxHeadError,
            std::fabs(statistic(ph, EN_MAXHEADERROR)));
        result.maxFlowChange = std::max(result.maxFlowChange,
            std::fabs(statistic(ph, EN_MAXFLOWCHANGE)));

        require_ok(EN_nextH(ph, &timeStep));
    }
    while (timeStep > 0);

    int nodeCount = 0;
    int linkCount = 0;
    require_ok(EN_getcount(ph, EN_NODECOUNT, &nodeCount));
    require_ok(EN_getcount(ph, EN_LINKCOUNT, &linkCount));

    for (int i = 1; i <= nodeCount; ++i)
    {
        double head = 0.0;
        require_ok(EN_getnodevalue(ph, i, EN_HEAD, &head));
        result.finalHeadSum += head;
        result.finalHeadWeightedSum += i * head;
    }
    for (int i = 1; i <= linkCount; ++i)
    {
        double flow = 0.0;
        require_ok(EN_getlinkvalue(ph, i, EN_FLOW, &flow));
        result.finalFlowSum += flow;
        result.finalFlowWeightedSum += i * flow;
    }

    require_ok(EN_closeH(ph));
    require_ok(EN_close(ph));
    require_ok(EN_deleteproject(ph));
    return result;
}

void check_signature(double actual, double expected, const char* network,
    const char* quantity)
{
    const double tolerance = 1.e-6 + 1.e-9 * std::fabs(expected);
    BOOST_CHECK_MESSAGE(std::fabs(actual - expected) <= tolerance,
        network << ": " << quantity << " expected " << expected <<
        ", got " << actual << " (tolerance " << tolerance << ")");
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_hydraulic_performance_guardrails)

BOOST_AUTO_TEST_CASE(test_reference_network_signatures_and_iteration_budgets)
{
    const std::size_t count = sizeof(BASELINES) / sizeof(BASELINES[0]);
    for (std::size_t i = 0; i < count; ++i)
    {
        const HydraulicBaseline& baseline = BASELINES[i];
        const HydraulicSignature actual = run_hydraulics(baseline);

        BOOST_CHECK_MESSAGE(actual.finalTime == baseline.finalTime,
            baseline.name << ": final hydraulic time changed from " <<
            baseline.finalTime << " to " << actual.finalTime);
        BOOST_CHECK_MESSAGE(actual.eventCount == baseline.eventCount,
            baseline.name << ": hydraulic event count changed from " <<
            baseline.eventCount << " to " << actual.eventCount);

        // Performance work may reduce iteration counts, but should not silently
        // require more GGA work for these reference networks.
        BOOST_CHECK_MESSAGE(actual.totalIterations <= baseline.maxTotalIterations,
            baseline.name << ": total iterations increased from baseline " <<
            baseline.maxTotalIterations << " to " << actual.totalIterations);
        BOOST_CHECK_MESSAGE(actual.maxIterationsPerEvent <=
            baseline.maxIterationsPerEvent,
            baseline.name << ": max iterations/event increased from baseline " <<
            baseline.maxIterationsPerEvent << " to " <<
            actual.maxIterationsPerEvent);

        check_signature(actual.finalHeadSum, baseline.finalHeadSum,
            baseline.name, "final head sum");
        check_signature(actual.finalFlowSum, baseline.finalFlowSum,
            baseline.name, "final flow sum");
        check_signature(actual.finalHeadWeightedSum,
            baseline.finalHeadWeightedSum, baseline.name,
            "final weighted head sum");
        check_signature(actual.finalFlowWeightedSum,
            baseline.finalFlowWeightedSum, baseline.name,
            "final weighted flow sum");

        // Keep convergence diagnostics observable without freezing their exact
        // floating-point values as an optimization constraint.
        BOOST_CHECK_MESSAGE(std::isfinite(actual.maxRelativeError),
            baseline.name << ": relative error became non-finite");
        BOOST_CHECK_MESSAGE(std::isfinite(actual.maxHeadError),
            baseline.name << ": max head error became non-finite");
        BOOST_CHECK_MESSAGE(std::isfinite(actual.maxFlowChange),
            baseline.name << ": max flow change became non-finite");
    }
}

BOOST_AUTO_TEST_SUITE_END()
