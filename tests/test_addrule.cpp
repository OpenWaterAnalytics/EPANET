// Test of EN_addrule, EN_deletenode & EN_deletelink EPANET API Functions
#define _CRT_SECURE_NO_DEPRECATE

/*
This is a test for the API functions that adds rules and deletes
nodes and links from a project. Deletion can be conditional on
node or link appearing in any simple or rule-based controls.
*/

//#define NO_BOOST

#ifndef NO_BOOST
#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>
#endif

#include <iostream>
#include <string>
#include "epanet2_2.h"

#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

#ifdef NO_BOOST
#define BOOST_REQUIRE(x) (((x)) ? cout << "\nPassed at line " << __LINE__ : cout << "\nFailed at line " << __LINE__ )
#endif

using namespace std;

char R1[] = "RULE 1 \n IF NODE 2 LEVEL < 100 \n THEN LINK 9 STATUS = OPEN";
char R2[] = "RULE 2\nIF SYSTEM TIME = 4\nTHEN LINK 9 STATUS = CLOSED\nAND LINK 31 STATUS = CLOSED";
char R3[] = "RULE 3\nIF NODE 23 PRESSURE ABOVE 140\nAND NODE 2 LEVEL > 120\n"
            "THEN LINK 113 STATUS = CLOSED\nELSE LINK 22 STATUS = CLOSED";

#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE (test_toolkit)
BOOST_AUTO_TEST_CASE(test_setlinktype)
#else
int main(int argc, char *argv[])
#endif
{
    int error = 0;
    int ruleCount, nP, nTA, nEA;
    int link113, node23, link22, pump9_before, pump9_after;
    double priority;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Add the 3 rules to the project
    error = EN_addrule(ph, R1);
    BOOST_REQUIRE(error == 0);
    error = EN_addrule(ph, R2);
    BOOST_REQUIRE(error == 0);
    error = EN_addrule(ph, R3);
    BOOST_REQUIRE(error == 0);

    // Check that rules were added
    error = EN_getcount(ph, EN_RULECOUNT, &ruleCount);
    BOOST_REQUIRE(ruleCount == 3);

    // Check the number of clauses in rule 3
    error = EN_getrule(ph, 3, &nP, &nTA, &nEA, &priority);
    BOOST_REQUIRE(nP == 2);
    BOOST_REQUIRE(nTA == 1);
    BOOST_REQUIRE(nTA == 1);

    // Try to delete link 113 conditionally which will fail
    // because it's in rule 3
    EN_getlinkindex(ph, "113", &link113);
    error = EN_deletelink(ph, link113, EN_CONDITIONAL);
    BOOST_REQUIRE(error == 261);

    // Delete node 23 unconditionally which will result in the
    // deletion of rule 3 as well as links 22 and 113
    EN_getnodeindex(ph, "23", &node23);
    EN_getlinkindex(ph, "22", &link22);
    EN_getlinkindex(ph, "9", &pump9_before);
    error = EN_deletenode(ph, node23, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);

    // Check that there are now only 2 rules
    error = EN_getcount(ph, EN_RULECOUNT, &ruleCount);
    BOOST_REQUIRE(ruleCount == 2);

    // Check that link 22 no longer exists
    error = EN_getlinkindex(ph, "22", &link22);
    BOOST_REQUIRE(error > 0);

    // Check that the index of pump9 has been reduced by 2
    error = EN_getlinkindex(ph, "9", &pump9_after);
    BOOST_REQUIRE(pump9_before - pump9_after == 2);

    // Close and delete project
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}
#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE_END()
#endif
