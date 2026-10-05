/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       hydraulic_benchmark.cpp
 Description:  Reproducible hydraulic solver performance benchmark
 Authors:      see AUTHORS
 Copyright:    see COPYRIGHT
 License:      see LICENSE
 ******************************************************************************
*/

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "epanet2_2.h"

namespace
{

struct NetworkCase
{
    std::string name;
    std::string path;
};

struct HydraulicSignature
{
    long finalTime;
    long eventCount;
    long totalIterations;
    long maxIterations;
    double maxRelativeError;
    double maxHeadError;
    double maxFlowChange;
    double finalHeadSum;
    double finalFlowSum;
    double finalHeadWeightedSum;
    double finalFlowWeightedSum;
};

struct ProjectHandle
{
    ProjectHandle() : handle(NULL)
    {
        check(EN_createproject(&handle), "EN_createproject");
    }

    ~ProjectHandle()
    {
        if (handle)
        {
            EN_close(handle);
            EN_deleteproject(handle);
        }
    }

    static void check(int error, const char* operation)
    {
        if (error != 0)
        {
            throw std::runtime_error(std::string(operation) +
                " failed with EPANET error " + std::to_string(error));
        }
    }

    EN_Project handle;
};

void check(int error, const char* operation)
{
    ProjectHandle::check(error, operation);
}

double statistic(EN_Project ph, int type)
{
    double value = 0.0;
    check(EN_getstatistic(ph, type, &value), "EN_getstatistic");
    return value;
}

HydraulicSignature characterize(EN_Project ph)
{
    HydraulicSignature result = {};
    check(EN_initH(ph, EN_INITFLOW), "EN_initH");

    long time = 0;
    long timeStep = 0;
    do
    {
        check(EN_runH(ph, &time), "EN_runH");
        ++result.eventCount;
        result.finalTime = time;

        const long iterations = static_cast<long>(
            std::llround(statistic(ph, EN_ITERATIONS)));
        result.totalIterations += iterations;
        result.maxIterations = std::max(result.maxIterations, iterations);
        result.maxRelativeError = std::max(result.maxRelativeError,
            std::fabs(statistic(ph, EN_RELATIVEERROR)));
        result.maxHeadError = std::max(result.maxHeadError,
            std::fabs(statistic(ph, EN_MAXHEADERROR)));
        result.maxFlowChange = std::max(result.maxFlowChange,
            std::fabs(statistic(ph, EN_MAXFLOWCHANGE)));

        check(EN_nextH(ph, &timeStep), "EN_nextH");
    }
    while (timeStep > 0);

    int nodeCount = 0;
    int linkCount = 0;
    check(EN_getcount(ph, EN_NODECOUNT, &nodeCount), "EN_getcount(nodes)");
    check(EN_getcount(ph, EN_LINKCOUNT, &linkCount), "EN_getcount(links)");

    for (int i = 1; i <= nodeCount; ++i)
    {
        double head = 0.0;
        check(EN_getnodevalue(ph, i, EN_HEAD, &head), "EN_getnodevalue(head)");
        result.finalHeadSum += head;
        result.finalHeadWeightedSum += i * head;
    }
    for (int i = 1; i <= linkCount; ++i)
    {
        double flow = 0.0;
        check(EN_getlinkvalue(ph, i, EN_FLOW, &flow), "EN_getlinkvalue(flow)");
        result.finalFlowSum += flow;
        result.finalFlowWeightedSum += i * flow;
    }

    return result;
}

void run_hydraulics(EN_Project ph)
{
    check(EN_initH(ph, EN_INITFLOW), "EN_initH");
    long time = 0;
    long timeStep = 0;
    do
    {
        check(EN_runH(ph, &time), "EN_runH");
        check(EN_nextH(ph, &timeStep), "EN_nextH");
    }
    while (timeStep > 0);
}

std::string basename_without_extension(const std::string& path)
{
    const std::string::size_type slash = path.find_last_of("/\\");
    const std::string filename = slash == std::string::npos ?
        path : path.substr(slash + 1);
    const std::string::size_type dot = filename.find_last_of('.');
    return dot == std::string::npos ? filename : filename.substr(0, dot);
}

int parse_nonnegative(const char* text, const char* option)
{
    char* end = NULL;
    const long value = std::strtol(text, &end, 10);
    if (!text[0] || !end || *end != '\0' || value < 0 ||
        value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error(std::string("invalid value for ") + option);
    }
    return static_cast<int>(value);
}

void print_usage(const char* program)
{
    std::cout << "Usage: " << program <<
        " [--repetitions N] [--warmup N] [network.inp ...]\n\n"
        "With no network paths, benchmarks Net1, Net2, Net3 and the bundled "
        "Grid20 synthetic network.\n"
        "Timing is informational only and is intentionally not registered as "
        "a CTest test.\n";
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        int repetitions = 25;
        int warmup = 3;
        std::vector<std::string> paths;

        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if (arg == "--help" || arg == "-h")
            {
                print_usage(argv[0]);
                return 0;
            }
            if (arg == "--repetitions" || arg == "-n")
            {
                if (++i >= argc) throw std::runtime_error("missing repetitions");
                repetitions = parse_nonnegative(argv[i], "--repetitions");
                if (repetitions == 0)
                    throw std::runtime_error("--repetitions must be greater than zero");
                continue;
            }
            if (arg == "--warmup")
            {
                if (++i >= argc) throw std::runtime_error("missing warmup count");
                warmup = parse_nonnegative(argv[i], "--warmup");
                continue;
            }
            paths.push_back(arg);
        }

        std::vector<NetworkCase> cases;
        if (paths.empty())
        {
            cases.push_back({"Net1", EPANET_SOURCE_DIR "/example-networks/Net1.inp"});
            cases.push_back({"Net2", EPANET_SOURCE_DIR "/example-networks/Net2.inp"});
            cases.push_back({"Net3", EPANET_SOURCE_DIR "/example-networks/Net3.inp"});
            cases.push_back({"Grid20", EPANET_SOURCE_DIR "/benchmarks/data/Grid20.inp"});
        }
        else
        {
            for (std::size_t i = 0; i < paths.size(); ++i)
                cases.push_back({basename_without_extension(paths[i]), paths[i]});
        }

        std::cout << "network,nodes,links,repetitions,warmup,hydraulic_events,"
            "total_iterations,max_iterations,final_time_s,max_relative_error,"
            "max_head_error,max_flow_change,final_head_sum,final_flow_sum,"
            "final_head_weighted_sum,final_flow_weighted_sum,elapsed_ms,us_per_run\n";
        std::cout << std::setprecision(15);

        for (std::size_t c = 0; c < cases.size(); ++c)
        {
            ProjectHandle project;
#ifdef _WIN32
            const char* nullReport = "NUL";
#else
            const char* nullReport = "/dev/null";
#endif
            check(EN_open(project.handle, cases[c].path.c_str(), nullReport, ""), "EN_open");
            check(EN_openH(project.handle), "EN_openH");

            int nodeCount = 0;
            int linkCount = 0;
            check(EN_getcount(project.handle, EN_NODECOUNT, &nodeCount),
                "EN_getcount(nodes)");
            check(EN_getcount(project.handle, EN_LINKCOUNT, &linkCount),
                "EN_getcount(links)");

            const HydraulicSignature signature = characterize(project.handle);

            for (int i = 0; i < warmup; ++i)
                run_hydraulics(project.handle);

            const std::chrono::steady_clock::time_point start =
                std::chrono::steady_clock::now();
            for (int i = 0; i < repetitions; ++i)
                run_hydraulics(project.handle);
            const std::chrono::steady_clock::time_point stop =
                std::chrono::steady_clock::now();

            const double elapsedMs =
                std::chrono::duration<double, std::milli>(stop - start).count();
            const double usPerRun = elapsedMs * 1000.0 / repetitions;

            std::cout << cases[c].name << ',' << nodeCount << ',' << linkCount << ','
                << repetitions << ',' << warmup << ',' << signature.eventCount << ','
                << signature.totalIterations << ',' << signature.maxIterations << ','
                << signature.finalTime << ',' << signature.maxRelativeError << ','
                << signature.maxHeadError << ',' << signature.maxFlowChange << ','
                << signature.finalHeadSum << ',' << signature.finalFlowSum << ','
                << signature.finalHeadWeightedSum << ','
                << signature.finalFlowWeightedSum << ',' << elapsedMs << ',' << usPerRun << '\n';

            check(EN_closeH(project.handle), "EN_closeH");
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "hydraulic_benchmark: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
