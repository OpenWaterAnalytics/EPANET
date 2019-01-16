// Test of ENsetlinktype EPANET API Function
#define _CRT_SECURE_NO_DEPRECATE

/*
This is a test for the API function that changes a link's type.
Two links in Net1.inp are changed: Pipe 113 is reversed with a CV added
and Pipe 121 is changed to a 100 psi PRV. After running the revised model,
at hour 0 the flow in Pipe 113 should be zero and the pressure at node 31
of the PRV 121 should be 100.
*/

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet2_2.h"

#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE(test_setlinktype)
{
    int error = 0;
    int p113, n31, p121, n113_1, n113_2;
    double q113 = 0.0, p31 = 0.0, diam;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Change duration to 0
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);

    // Get indexes of pipe 113 and node 31
    error = EN_getlinkindex(ph, (char *)"113", &p113);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, (char *)"31", &n31);
    BOOST_REQUIRE(error == 0);

    // Reverse pipe 113 and give it a check valve
    error = EN_getlinknodes(ph, p113, &n113_1, &n113_2);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinknodes(ph, p113, n113_2, n113_1);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinktype(ph, &p113, EN_CVPIPE, 0);
    BOOST_REQUIRE(error == 0);

    // Get index & diameter of pipe 121 connected to node 31
    error = EN_getlinkindex(ph, (char *)"121", &p121);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, p121, EN_DIAMETER, &diam);
    BOOST_REQUIRE(error == 0);

    // Replace it with a PRV
    error = EN_setlinktype(ph, &p121, EN_PRV, 0);
    BOOST_REQUIRE(error == 0);

    // Set diameter & setting of new PRV
    error = EN_setlinkvalue(ph, p121, EN_INITSETTING, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, p121, EN_DIAMETER, diam);
    BOOST_REQUIRE(error == 0);

    // Solve for hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Get flow in link 113 and pressure at node 31
    error = EN_getlinkvalue(ph, p113, EN_FLOW, &q113);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, n31, EN_PRESSURE, &p31);
    BOOST_REQUIRE(error == 0);

    // Require that link 113 flow be 0
    q113 = fabs(q113);
    BOOST_REQUIRE(q113 < 0.001);

    // Require that node 31 pressure be 100
    p31 = fabs(p31 - 100.0f);
    BOOST_REQUIRE(p31 < 0.001);

    // Close and delete project
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()
