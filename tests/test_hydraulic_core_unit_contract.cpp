/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_hydraulic_core_unit_contract.cpp
 Description:  Guards the hydraulic numerical core's solver-unit contract
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 ******************************************************************************
*/

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

namespace
{

std::string read_source(const char* relativePath)
{
    const std::string path = std::string(EPANET_SOURCE_DIR) + "/" + relativePath;
    std::ifstream stream(path.c_str(), std::ios::in | std::ios::binary);
    BOOST_REQUIRE_MESSAGE(stream.good(), "Cannot read " << path);

    std::ostringstream contents;
    contents << stream.rdbuf();
    return contents.str();
}

std::size_t count_occurrences(const std::string& text, const std::string& token)
{
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::string::npos)
    {
        ++count;
        pos += token.size();
    }
    return count;
}

void check_no_dimensional_state_access(const char* sourceName,
    const std::string& source)
{
    const char* forbidden[] = {
        "hyd->NodeHead",
        "hyd->NodeDemand",
        "hyd->FullDemand",
        "hyd->DemandFlow",
        "hyd->EmitterFlow",
        "hyd->LeakageFlow",
        "hyd->LinkFlow"
    };

    for (const char* token : forbidden)
    {
        BOOST_CHECK_MESSAGE(source.find(token) == std::string::npos,
            sourceName << " directly accesses dimensional hydraulic state through "
            << token << "; use SolverState inside the GGA core");
    }
}

void check_no_solver_fixed_unit_constants(const char* sourceName,
    const std::string& source)
{
    // These constants are valid only while constructing dimensional model-side
    // coefficients. They must never appear in pure solver/status arithmetic.
    const char* forbidden[] = {"4.727", "32.2", "1.49", "0.02517", "8.814"};

    for (const char* token : forbidden)
    {
        BOOST_CHECK_MESSAGE(source.find(token) == std::string::npos,
            sourceName << " contains fixed-unit constant " << token);
    }
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_hydraulic_core_unit_contract)

BOOST_AUTO_TEST_CASE(numerical_core_uses_solver_state)
{
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    check_no_dimensional_state_access("hydsolver.c", hydsolver);
    check_no_dimensional_state_access("hydcoeffs.c", hydcoeffs);
    check_no_dimensional_state_access("hydstatus.c", hydstatus);
}

BOOST_AUTO_TEST_CASE(fixed_unit_constants_stay_at_model_compilation_boundary)
{
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    check_no_solver_fixed_unit_constants("hydsolver.c", hydsolver);
    check_no_solver_fixed_unit_constants("hydstatus.c", hydstatus);

    // hydcoeffs.c still constructs dimensional EPANET model coefficients before
    // compiling them into solver units. Keep the known fixed-basis constants
    // confined to these four model-side formulas.
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "4.727"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "32.2"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "1.49"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "0.02517"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "8.814"), 0u);

    BOOST_CHECK(hydcoeffs.find(
        "link->R = 4.727 * L / pow(e, hyd->Hexp) / pow(d, 4.871);") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "link->R = L / 2.0 / 32.2 / d / SQR(PI * SQR(d) / 4.0);") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "SQR(4.0 * e / (1.49 * PI * SQR(d)))") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "link->Km = 0.02517 * hyd->LinkSetting[k]") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(global_and_node_inputs_use_compiled_solver_model)
{
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");
    const std::string leakage = read_source("src/leakage.c");

    BOOST_CHECK(hydsolver.find("hyd->SolverModel.NodeElevation") !=
        std::string::npos);
    BOOST_CHECK(hydsolver.find("hydheadtosolver(pr, hyd->Htol)") ==
        std::string::npos);
    BOOST_CHECK(hydsolver.find("hydheadtosolver(pr, hyd->Pmin)") ==
        std::string::npos);
    BOOST_CHECK(hydsolver.find("hydflowtosolver(pr, hyd->FlowChangeLimit)") ==
        std::string::npos);

    BOOST_CHECK(hydcoeffs.find("hydresistancetosolver(pr, hyd->RQtol, 1.0)") ==
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hydheadtosolver(pr, hyd->Htol)") ==
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hydflowtosolver(pr, hyd->Qtol)") ==
        std::string::npos);
    BOOST_CHECK(leakage.find("hydheadtosolver(pr, net->Node[i].El)") ==
        std::string::npos);
}


BOOST_AUTO_TEST_CASE(dimensions_enter_solver_through_scaling_helpers)
{
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    BOOST_CHECK(hydsolver.find("hydheadtosolver") != std::string::npos);
    BOOST_CHECK(hydsolver.find("hydflowtosolver") != std::string::npos);

    BOOST_CHECK(hydcoeffs.find("hydheadtosolver") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydflowtosolver") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydresistancetosolver") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydconductancetosolver") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydminorlosstosolver") != std::string::npos);

    BOOST_CHECK(hydstatus.find("hydheadtosolver") != std::string::npos);
    BOOST_CHECK(hydstatus.find("hydflowtosolver") != std::string::npos);
    BOOST_CHECK(hydstatus.find("hydminorlosstosolver") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()
