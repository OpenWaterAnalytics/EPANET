//
// test_setid.cpp
//

/*
This is a test for the API functions that change a node or link ID name.
A node and link name are changed, the network is saved, reopened and the new names are checked.
*/

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string.h>
#include "epanet2.h"

// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE(test_setid)
{
    int error = 0;
    int index;
    char errmsg[256];
    char id[80];
    char newid[80];
    
    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);
    
    error = ENopen(path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    // Test of illegal node name change
    error = ENgetnodeid(3, id);
    BOOST_REQUIRE(error == 0);
    strncpy(newid, "Illegal; node name", 79);
    error = ENsetnodeid(3, newid);
    BOOST_REQUIRE(error > 0);

    // Test of legal node name change
    strncpy(newid, "Node3", 79);
    error = ENsetnodeid(3, newid);
    BOOST_REQUIRE(error == 0);
    
    // Test of illegal link name change
    error = ENgetlinkid(3, id);
    BOOST_REQUIRE(error == 0);
    strncpy(newid, "Illegal; link name", 79);
    error = ENsetlinkid(3, newid);
    BOOST_REQUIRE(error > 0);

    // Test of legal link name change
    strncpy(newid, "Link3", 79);
    error = ENsetlinkid(3, newid);
    BOOST_REQUIRE(error == 0);

    // Save the project
    error = ENsaveinpfile("net1_setid.inp");
    BOOST_REQUIRE(error == 0);
    error = ENclose();
    BOOST_REQUIRE(error == 0);
    
    // Re-open the saved project
    error = ENopen("net1_setid.inp", path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);
    
    // Check that 3rd node has its new name
    error = ENgetnodeindex((char *)"Node3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);
    
    // Check that 3rd link has its new name
    error = ENgetlinkindex((char *)"Link3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);
    ENclose();
}

BOOST_AUTO_TEST_SUITE_END()