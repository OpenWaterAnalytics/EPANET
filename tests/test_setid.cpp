// Test of ENsetid EPANET API Function
#define _CRT_SECURE_NO_DEPRECATE

/*
This is a test for the API functions that change a node or link ID name.
A node and link name are changed, the network is saved, reopened and the new names are checked.
*/

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet_2_2.h"

#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE(test_setid)
{
    int error = 0;
    int index;
    string newid;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Test of illegal node name change
    newid = "Illegal; node name";
    error = EN_setnodeid(ph, 3, (char *)newid.c_str());
    BOOST_REQUIRE(error > 0);

    // Test of legal node name change
    newid = "Node3";
    error = EN_setnodeid(ph, 3, (char *)newid.c_str());
    BOOST_REQUIRE(error == 0);

    // Test of illegal link name change
    newid = "Illegal; link name";
    error = EN_setlinkid(ph, 3, (char *)newid.c_str());
    BOOST_REQUIRE(error > 0);

    // Test of legal link name change
    newid = "Link3";
    error = EN_setlinkid(ph, 3, (char *)newid.c_str());
    BOOST_REQUIRE(error == 0);

    // Save the project
    error = EN_saveinpfile(ph, "net1_setid.inp");
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);

    // Re-open the saved project
    EN_createproject(&ph);
    error = EN_open(ph, "net1_setid.inp", path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Check that 3rd node has its new name
    error = EN_getnodeindex(ph, "Node3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);

    // Check that 3rd link has its new name
    error = EN_getlinkindex(ph, "Link3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()
