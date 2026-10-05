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
    const std::string hydscale = read_source("src/hydscale.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    check_no_solver_fixed_unit_constants("hydsolver.c", hydsolver);
    check_no_solver_fixed_unit_constants("hydstatus.c", hydstatus);

    // Fixed-basis constants may appear only while constructing dimensional
    // model coefficients or compiling dimensional settings into solver units.
    // Static pipe formulas remain in hydcoeffs.c; the dynamic TCV formula now
    // belongs to the hydscale.c compilation boundary.
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "4.727"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "32.2"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "1.49"), 1u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "0.02517"), 0u);
    BOOST_CHECK_EQUAL(count_occurrences(hydcoeffs, "8.814"), 0u);
    BOOST_CHECK_EQUAL(count_occurrences(hydscale, "0.02517"), 1u);

    BOOST_CHECK(hydcoeffs.find(
        "link->R = 4.727 * L / pow(e, hyd->Hexp) / pow(d, 4.871);") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "link->R = L / 2.0 / 32.2 / d / SQR(PI * SQR(d) / 4.0);") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "SQR(4.0 * e / (1.49 * PI * SQR(d)))") != std::string::npos);
    BOOST_CHECK(hydscale.find(
        "km = 0.02517 * setting /") != std::string::npos);
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
    BOOST_CHECK(hydcoeffs.find("hyd->SolverModel.BigConductance") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hyd->SolverModel.SmallConductance") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydconductancetosolver") == std::string::npos);
    BOOST_CHECK(hydstatus.find("hydheadtosolver(pr, hyd->Htol)") ==
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hydflowtosolver(pr, hyd->Qtol)") ==
        std::string::npos);
    BOOST_CHECK(leakage.find("hydheadtosolver(pr, net->Node[i].El)") ==
        std::string::npos);
}


BOOST_AUTO_TEST_CASE(static_link_coefficients_use_compiled_solver_model)
{
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");

    BOOST_CHECK(hydcoeffs.find(
        "ml = hyd->SolverModel.LinkMinorLoss[k];") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "r = hyd->SolverModel.LinkResistance[k];") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "double s = hyd->SolverModel.LinkViscosityFlow[k];") !=
        std::string::npos);

    BOOST_CHECK(hydcoeffs.find(
        "hydminorlosstosolver(pr, pr->network.Link[k].Km)") ==
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "hydresistancetosolver(pr, pr->network.Link[k].R, hyd->Hexp)") ==
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find(
        "hydflowtosolver(pr, hyd->Viscos * link->Diam)") ==
        std::string::npos);
}


BOOST_AUTO_TEST_CASE(dynamic_link_settings_use_compiled_solver_model)
{
    const std::string hydscale = read_source("src/hydscale.c");
    const std::string hydraul = read_source("src/hydraul.c");
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    // Dynamic settings are compiled once at mutation boundaries. Pressure and
    // flow settings become solver values, while TCV/PCV loss is cached as the
    // exact coefficient consumed by the iterative solver.
    BOOST_CHECK(hydscale.find("compilehydraulicsolversetting") !=
        std::string::npos);
    BOOST_CHECK(hydscale.find("model->LinkSetting[i]") != std::string::npos);
    BOOST_CHECK(hydscale.find("model->LinkDynamicLoss[i]") !=
        std::string::npos);
    BOOST_CHECK(hydraul.find("compilehydraulicsolversetting(pr, index)") !=
        std::string::npos);
    BOOST_CHECK(hydraul.find("compilehydraulicsolversetting(pr, k)") !=
        std::string::npos);
    BOOST_CHECK(hydsolver.find("compilehydraulicsolversetting(pr, k)") !=
        std::string::npos);

    // GGA coefficient/status code consumes those compiled values directly.
    BOOST_CHECK(hydcoeffs.find("hyd->SolverModel.LinkSetting[k]") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hyd->SolverModel.LinkDynamicLoss[k]") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydflowtosolver(pr, hyd->LinkSetting[k])") ==
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("0.02517 * hyd->LinkSetting[k]") ==
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hyd->SolverModel.LinkSetting[k]") !=
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hyd->SolverModel.LinkMinorLoss[k]") !=
        std::string::npos);
    BOOST_CHECK(hydstatus.find("hydflowtosolver") == std::string::npos);
    BOOST_CHECK(hydstatus.find("hydminorlosstosolver") == std::string::npos);
}


BOOST_AUTO_TEST_CASE(pump_and_gpv_curves_use_compiled_solver_model)
{
    const std::string hydscale = read_source("src/hydscale.c");
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");

    // Pump/GPV flow-head curves are compiled at the model boundary. Their
    // flow breakpoints and segment intercept/slope coefficients are stored in
    // SolverModel instead of converting flow back to curve units in the GGA.
    BOOST_CHECK(hydscale.find("compilehydraulicsolvercurve") !=
        std::string::npos);
    BOOST_CHECK(hydscale.find("compiled->X[j] = hydflowtosolver") !=
        std::string::npos);
    BOOST_CHECK(hydscale.find("compiled->H0[j] = hydheadtosolver") !=
        std::string::npos);
    BOOST_CHECK(hydscale.find("compiled->R[j] = hydresistancetosolver") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("solvercurvecoeff") != std::string::npos);
    BOOST_CHECK(hydcoeffs.find("static void    curvecoeff") ==
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("hydflowfromsolver(pr, q)") ==
        std::string::npos);

    // Curve interpolation itself is now solver-native. Other GGA hot paths
    // are checked separately below.
    BOOST_CHECK(hydcoeffs.find("hydflowfromsolver") == std::string::npos);
}


BOOST_AUTO_TEST_CASE(gga_hot_paths_are_solver_native)
{
    const std::string hydscale = read_source("src/hydscale.c");
    const std::string hydsolver = read_source("src/hydsolver.c");
    const std::string hydcoeffs = read_source("src/hydcoeffs.c");
    const std::string hydstatus = read_source("src/hydstatus.c");
    const std::string leakage = read_source("src/leakage.c");

    // Matrix/nonlinear coefficient assembly must not cross back into the
    // dimensional model. Emitters, PDA reference grades, barriers, pumps,
    // valves, and curves all consume precompiled SolverModel values.
    const char* helpers[] = {
        "hydheadtosolver", "hydflowtosolver", "hydresistancetosolver",
        "hydconductancetosolver", "hydminorlosstosolver",
        "hydheadfromsolver", "hydflowfromsolver"
    };
    for (const char* helper : helpers)
    {
        BOOST_CHECK_MESSAGE(hydcoeffs.find(helper) == std::string::npos,
            "hydcoeffs.c still crosses the dimensional boundary through " << helper);
    }
    BOOST_CHECK(hydcoeffs.find("SolverModel.NodeEmitterResistance") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("SolverModel.NodePdaMinGrade") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("SolverModel.LinkPumpH0") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("SolverModel.LinkPumpResistance") !=
        std::string::npos);
    BOOST_CHECK(hydcoeffs.find("SolverModel.BarrierGradient") !=
        std::string::npos);

    // Status arithmetic is solver-native too. The only dimensional crossing
    // left here is tankstatus(), whose tank-volume simulation state remains on
    // EPANET's dimensional compatibility side of the solver boundary.
    BOOST_CHECK(hydstatus.find("hydheadtosolver") == std::string::npos);
    BOOST_CHECK(hydstatus.find("hydflowtosolver") == std::string::npos);
    BOOST_CHECK(hydstatus.find("hydresistancetosolver") == std::string::npos);
    BOOST_CHECK_EQUAL(count_occurrences(hydstatus, "hydflowfromsolver"), 2u);
    BOOST_CHECK(hydstatus.find("tankstatus(pr, k, n1") != std::string::npos);
    BOOST_CHECK(hydstatus.find("tankstatus(pr, k, n2") != std::string::npos);
    BOOST_CHECK(hydstatus.find("SolverModel.LinkPumpMaxHead") !=
        std::string::npos);

    // hydsolver.c converts only when publishing diagnostics/report values.
    // Simple-control trigger grades are compiled into SolverModel.
    BOOST_CHECK(hydsolver.find("hydheadtosolver") == std::string::npos);
    BOOST_CHECK(hydsolver.find("hydflowtosolver") == std::string::npos);
    BOOST_CHECK(hydsolver.find("SolverModel.ControlGrade") !=
        std::string::npos);
    BOOST_CHECK_EQUAL(count_occurrences(hydsolver, "hydheadfromsolver"), 2u);
    BOOST_CHECK_EQUAL(count_occurrences(hydsolver, "hydflowfromsolver"), 2u);

    // Leakage coefficient conversion is initialization-boundary work. The
    // iterative leakage barrier itself no longer converts solver flow/head.
    BOOST_CHECK(leakage.find("hydheadtosolver") == std::string::npos);
    BOOST_CHECK(leakage.find("hydflowfromsolver") == std::string::npos);
    BOOST_CHECK_EQUAL(count_occurrences(leakage, "hydresistancetosolver"), 2u);
    BOOST_CHECK_EQUAL(count_occurrences(leakage, "hydflowtosolver"), 2u);
    BOOST_CHECK(leakage.find("SolverModel.BarrierGradient") !=
        std::string::npos);

    // All of the hot-path inputs above must be compiled at hydscale.c's model
    // boundary rather than reconstructed ad hoc in solver code.
    BOOST_CHECK(hydscale.find("NodeEmitterResistance") != std::string::npos);
    BOOST_CHECK(hydscale.find("NodePdaMinGrade") != std::string::npos);
    BOOST_CHECK(hydscale.find("LinkPumpH0") != std::string::npos);
    BOOST_CHECK(hydscale.find("LinkPumpResistance") != std::string::npos);
    BOOST_CHECK(hydscale.find("LinkPumpMaxHead") != std::string::npos);
    BOOST_CHECK(hydscale.find("ControlGrade") != std::string::npos);
    BOOST_CHECK(hydscale.find("BarrierGradient") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()
