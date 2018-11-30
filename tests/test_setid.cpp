// Test of ENsetid EPANET API Function
#define _CRT_SECURE_NO_DEPRECATE

/*
This is a test for the API functions that change a node or link ID name.
A node and link name are changed, the network is saved, reopened and the new names are checked.
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

#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE (test_toolkit)
BOOST_AUTO_TEST_CASE(test_setid)
#else
int main(int argc, char *argv[])
#endif
{
	string path_inp(DATA_PATH_INP);
	string path_rpt(DATA_PATH_RPT);
	string path_out(DATA_PATH_OUT);
	string inp_save("net1_setid.inp");

    int error = 0;
    int index;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Test of illegal node name change
    char newid_1[] = "Illegal; node name";
    error = EN_setnodeid(ph, 3, newid_1);
    BOOST_REQUIRE(error > 0);

    // Test of legal node name change
    char newid_2[] = "Node3";
    error = EN_setnodeid(ph, 3, newid_2);
    BOOST_REQUIRE(error == 0);

    // Test of illegal link name change
    char newid_3[] = "Illegal; link name";
    error = EN_setlinkid(ph, 3, newid_3);
    BOOST_REQUIRE(error > 0);

    // Test of legal link name change
    char newid_4[] = "Link3";
    error = EN_setlinkid(ph, 3, newid_4);
    BOOST_REQUIRE(error == 0);

    // Save the project
    error = EN_saveinpfile(ph, inp_save.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);

    // Re-open the saved project
    EN_createproject(&ph);
    error = EN_open(ph, inp_save.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Check that 3rd node has its new name
    error = EN_getnodeindex(ph, newid_2, &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);

    // Check that 3rd link has its new name
    error = EN_getlinkindex(ph, newid_4, &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}

#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE_END()
#endif
