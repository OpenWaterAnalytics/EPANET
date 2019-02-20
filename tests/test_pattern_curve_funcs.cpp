// Test of the EN_setpatternid, EN_setcurveid, EN_deletepattern & EN_deletecurve
// EPANET 2.2 API functions
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

    EN_Project ph = NULL;
    EN_createproject(&ph);
    
    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Get the default pattern index
    double dblPatIdx;
    int defPatIdx;
    int patIdx;
    EN_getoption(ph, EN_DEFDEMANDPAT, &dblPatIdx);
    defPatIdx = (int)dblPatIdx;

    // Rename the default pattern
    EN_setpatternid(ph, defPatIdx, (char *)"Pat1");
    EN_getpatternindex(ph, (char *)"Pat1", &patIdx);
    BOOST_REQUIRE(defPatIdx == patIdx);
    
    // Add 2 new patterns
    EN_addpattern(ph, (char *)"Pat2");
    EN_addpattern(ph, (char *)"Pat3");
    double f2[] = {2.1, 2.2};
    double f3[] = {3.1, 3.2, 3.3, 3.4};
    EN_setpattern(ph, 2, f2, 2);
    EN_setpattern(ph, 3, f3, 4);
    
    // Assign Pat3 to 3rd junction
    EN_setdemandpattern(ph, 3, 1, 3);

    // Delete Pat2
    EN_deletepattern(ph, 2);

    // Check that there are now 2 patterns
    int n;
    EN_getcount(ph, EN_PATCOUNT, &n);
    BOOST_REQUIRE(n == 2);
    
    // Check that Pat3 with 4 factors is still assigned to 3rd junction
    EN_getdemandpattern(ph, 3, 1, &patIdx);
    EN_getpatternlen(ph, patIdx, &n);
    BOOST_REQUIRE(n == 4);

    // Delete the default pattern
    EN_deletepattern(ph, defPatIdx);

    // Check that junction 4 has no pattern
    EN_getdemandpattern(ph, 4, 1, &patIdx);
    BOOST_REQUIRE(patIdx == 0);
    
    // And that junction 3 still uses Pat3
    EN_getdemandpattern(ph, 3, 1, &patIdx);
    char patID[EN_MAXID+1];
    EN_getpatternid(ph, patIdx, patID);
    BOOST_REQUIRE(strcmp(patID, "Pat3") == 0);

    // Re-name the pump's (Link 9) head curve
    int pumpIdx;
    char pumpID[] = "9";
    int curveIdx;
    char newCurveID[] = "PumpHeadCurve";
    EN_getlinkindex(ph, pumpID, &pumpIdx);
    EN_getheadcurveindex(ph, pumpIdx, &curveIdx);
    EN_setcurveid(ph, curveIdx, newCurveID);

    // Check that rename was successful
    char curveID[EN_MAXID + 1];
    EN_getcurveid(ph, curveIdx, curveID);
    BOOST_REQUIRE(strcmp(curveID, newCurveID) == 0);

    // Add two new curves
    double x2[] = {0.0, 1.0, 2.0};
    double y2[] = {400.0, 60.0, 30.0};
    double x3[] = {2000.0};
    double y3[] = {100.0};
    char curve2[] = "Curve2";
    char curve3[] = "Curve3";
    EN_addcurve(ph, curve2);
    EN_setcurve(ph, 2, x2, y2, 3);
    EN_addcurve(ph, curve3);
    EN_setcurve(ph, 3, x3, y3, 1);

    // Assign Curve3 to pump's head curve
    EN_getcurveindex(ph, curve3, &curveIdx);
    EN_setheadcurveindex(ph, pumpIdx, curveIdx);
    
    // Delete Curve2
    EN_getcurveindex(ph, curve2, &curveIdx);
    EN_deletecurve(ph, curveIdx);

    // Check that pump's head curve is still Curve3
    EN_getheadcurveindex(ph, pumpIdx, &curveIdx);
    EN_getcurveid(ph, curveIdx, curveID);
    BOOST_REQUIRE(strcmp(curveID, curve3) == 0);

    // And that it contains the correct data
    double x, y;
    EN_getcurvevalue(ph, curveIdx, 1, &x, &y);
    BOOST_REQUIRE(x == x3[0]);
    BOOST_REQUIRE(y == y3[0]);

    EN_close(ph);
    EN_deleteproject(&ph);
}

#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE_END()
#endif
