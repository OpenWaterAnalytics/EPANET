// Test of EPANET's Network Building Functions
//
// This is a test of the API functions EN_setjuncdata, EN_settankdata & EN_setpipedata
//
#define _CRT_SECURE_NO_DEPRECATE

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
BOOST_AUTO_TEST_CASE(test_setlinktype)
{
#else
int main(int argc, char *argv[])
{
#endif

    int index;
    char id[EN_MAXID+1];
    double p1_1, p2_1, p1_2, p2_2;
    double q1_1, q2_1, q1_2, q2_2;

    // Create & initialize a project
    EN_Project ph = NULL;
    EN_createproject(&ph);
    EN_init(ph, "", "", EN_GPM, EN_HW);
    
    // Build a network
    EN_addnode(ph, (char *)"N1", EN_JUNCTION);
    EN_addnode(ph, (char *)"N2", EN_JUNCTION);
    EN_addnode(ph, (char *)"N3", EN_RESERVOIR);
    EN_addnode(ph, (char *)"N4", EN_TANK);
    EN_addlink(ph, (char *)"L1", EN_PUMP, (char *)"N3", (char *)"N1");
    EN_addlink(ph, (char *)"L2", EN_PIPE, (char *)"N1", (char *)"N3");
    EN_addlink(ph, (char *)"L3", EN_PIPE, (char *)"N1", (char *)"N2");
    EN_addcurve(ph, (char *)"C1");

    // Set network data using the new helper functions
    EN_setcurvevalue(ph, 1, 1, 1500, 250);
    EN_setjuncdata(ph, 1, 700, 500, "");
    EN_setjuncdata(ph, 2, 710, 500, "");
    EN_setnodevalue(ph, 3, EN_ELEVATION, 800);
    EN_settankdata(ph, 4, 850, 120, 100, 150, 50.5, 0, "");
    EN_setlinkvalue(ph, 1, EN_PUMP_HCURVE, 1);
    EN_setpipedata(ph, 2, 10560, 12, 100, 0);
    EN_setpipedata(ph, 3, 5280, 14, 100, 0);

    // Run hydraulics
    EN_solveH(ph);

    // Save results
    EN_getnodevalue(ph, 1, EN_PRESSURE, &p1_1);
    EN_getnodevalue(ph, 2, EN_PRESSURE, &p2_1);
    EN_getlinkvalue(ph, 1, EN_FLOW, &q1_1);
    EN_getlinkvalue(ph, 2, EN_FLOW, &q2_1);

    // Save project
    EN_saveinpfile(ph, "test2.inp");

    // Close project
    EN_close(ph);

    // Open the saved project file
    EN_open(ph, "test2.inp", "", "");

    // Run hydraulics
    EN_solveH(ph);

    // Save these new results
    EN_getnodevalue(ph, 1, EN_PRESSURE, &p1_2);
    EN_getnodevalue(ph, 2, EN_PRESSURE, &p2_2);
    EN_getlinkindex(ph, (char *)"L1", &index);
    EN_getlinkvalue(ph, index, EN_FLOW, &q1_2);
    EN_getlinkindex(ph, (char *)"L2", &index);
    EN_getlinkvalue(ph, index, EN_FLOW, &q2_2);

    // Display old & new results
    cout << "\n  Node N1 Pressure: " << p1_1 << "  " << p1_2;
    cout << "\n  Node N2 Pressure: " << p2_1 << "  " << p2_2;
    cout << "\n  Link L1 Flow:     " << q1_1 << "  " << q1_2;
    cout << "\n  Link L2 Flow:     " << q2_1 << "  " << q2_2;

    // Compare old & new results
    BOOST_REQUIRE(abs(p1_1 - p1_2) < 1.e-5);
    BOOST_REQUIRE(abs(p2_1 - p2_2) < 1.e-5);
    BOOST_REQUIRE(abs(q1_1 - q1_2) < 1.e-5);
    BOOST_REQUIRE(abs(q2_1 - q2_2) < 1.e-5);

    // Close project
    EN_close(ph);
    EN_deleteproject(&ph);
}
#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE_END()
#endif
