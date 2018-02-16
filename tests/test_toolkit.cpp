//
// test_epanet_toolkit.cpp
//
// Date Created: January 24, 2018
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet2.h"

// NOTE: Project Home needs to be updated to run unit test
#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/epanet"
#define DATA_PATH "/tests/network_tests/net1"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE (test_alloc_free)
{
   int error = 0;
   EN_ProjectHandle ph = NULL;

   error = EN_alloc(&ph);

   BOOST_REQUIRE(error == 0);
   BOOST_CHECK(ph != NULL);

   error = EN_free(&ph);

   BOOST_REQUIRE(error == 0);
   BOOST_CHECK(ph == NULL);
}

BOOST_AUTO_TEST_SUITE_END( )

// int main(int argc, char *argv[])
// {
//     int error = 0;
//     EN_ProjectHandle project = NULL;

//     std::string data_path = std::string(PROJECT_HOME) + std::string(DATA_PATH);
//     std::string inputFile(data_path);
//     inputFile.append(std::string("/net1.inp"));
//     std::string reportFile(data_path);
//     reportFile.append(std::string("/net1.rpt"));
//     std::string outputFile(data_path);
//     outputFile.append(std::string("/net1.out"));

//     error = EN_alloc(&project);
//     error = EN_open(project, inputFile.c_str(), reportFile.c_str(), outputFile.c_str());

//     error = EN_solveH(project);
//     error = EN_solveQ(project);
//     error = EN_report(project);

//     error = EN_close(project);
//     error = EN_free(&project);

//     return error;
// }
