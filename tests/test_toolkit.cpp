//
// test_epanet_toolkit.cpp
//
// Date Created: January 24, 2018
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//

//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet2.h"

// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

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

BOOST_AUTO_TEST_CASE (test_open_close)
{
    EN_ProjectHandle ph = NULL;
    EN_alloc(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    int error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_free(&ph);
}

BOOST_AUTO_TEST_CASE(test_epanet)
{
    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    int error = ENepanet(path_inp.c_str(), path_rpt.c_str(), path_out.c_str(), NULL);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()


struct Fixture{
    Fixture() {
        path_inp = std::string(DATA_PATH_INP);
        path_rpt = std::string(DATA_PATH_RPT);
        path_out = std::string(DATA_PATH_OUT);

        EN_alloc(&ph);
        error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());

    }

    ~Fixture() {
      error = EN_close(&ph);
      EN_free(&ph);
  }

  std::string path_inp;
  std::string path_rpt;
  std::string path_out;

  int error;
  EN_ProjectHandle ph;

};




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
