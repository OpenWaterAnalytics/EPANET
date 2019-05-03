/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       test_net_builder.cpp
Description:  Tests EPANET toolkit api functions
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 04/12/2019
******************************************************************************
*/


#define BOOST_TEST_MODULE net_builder

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <stdlib.h>
#endif

#include <boost/test/included/unit_test.hpp>

#include "epanet2_2.h"


#define DATA_PATH_TMP "./tmp.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"


struct FixtureInitClose {
    FixtureInitClose() {
        error = 0;
        ph = NULL;

        EN_createproject(&ph);
        EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);
    }

    ~FixtureInitClose() {
        EN_close(ph);
        EN_deleteproject(&ph);
    }
    int error;
    EN_Project ph;
};


BOOST_AUTO_TEST_SUITE(test_net_builder)


// BOOST_AUTO_TEST_CASE(net_builder_I)
// {
// 	int error = 0;
// 	int flag = 00;
// 	long t, tstep;
// 	int i, ind, Lindex, Nindex, Cindex;
// 	double h_orig, h_build, h_build_loaded;
//
// 	// first we load Net1.inp, run it and record the head in Tank 2 at the end of the simulation (h_orig)
// 	EN_Project ph = NULL;
// 	EN_createproject(&ph);
//
// 	std::string path_inp = std::string(DATA_PATH_NET1);
// 	std::string path_rpt = std::string(DATA_PATH_RPT);
// 	std::string path_out = std::string(DATA_PATH_OUT);
//
// 	error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
// 	BOOST_REQUIRE(error == 0);
//
// 	error = EN_getnodeindex(ph, (char *)"2", &Nindex);
// 	BOOST_REQUIRE(error == 0);
//
// 	error = EN_openH(ph);
// 	BOOST_REQUIRE(error == 0);
//
// 	error = EN_initH(ph, flag);
// 	BOOST_REQUIRE(error == 0);
//
// 	do {
// 		error = EN_runH(ph, &t);
// 		BOOST_REQUIRE(error == 0);
//
// 		// this is the head at the end of the simulation after loading the original Net1.inp
// 		error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_orig);
// 		BOOST_REQUIRE(error == 0);
//
// 		error = EN_nextH(ph, &tstep);
// 		BOOST_REQUIRE(error == 0);
//
// 	} while (tstep > 0);
//
// 	error = EN_closeH(ph);
// 	BOOST_REQUIRE(error == 0);
//
// 	error = EN_close(ph);
// 	BOOST_REQUIRE(error == 0);
//
// 	EN_deleteproject(&ph);
// }

BOOST_FIXTURE_TEST_CASE(test_build_net1, FixtureInitClose)
{
    int flag = 00;
    long t, tstep;
    int i, ind, Lindex, Nindex, Cindex;
    double h_build;

    // now we build Net1 from scratch...
    char juncs[9][10] = { "10", "11", "12", "13", "21", "22", "23", "31", "32" };
    double e[9] = { 710, 710, 700, 695, 700, 695, 690, 700, 710 };
    double d[9] = { 0, 150, 150, 100, 150, 200, 150, 100, 100 };
    double X[9] = { 20, 30, 50, 70, 30, 50, 70, 30, 50 };
    double Y[9] = { 70, 70, 70, 70, 40, 40, 40, 10, 10 };
    double L[12] = { 10530, 5280, 5280, 5280, 5280, 5280, 200, 5280, 5280, 5280, 5280, 5280 };
    double dia[12] = { 18, 14, 10, 10, 12, 6, 18, 10, 12, 8, 8, 6 };
    double P[12] = { 1.0f, 1.2f, 1.4f, 1.6f, 1.4f, 1.2f, 1.0f, 0.8f, 0.6f, 0.4f, 0.6f, 0.8f };

    error = EN_addpattern(ph, (char *)"pat1");
    BOOST_REQUIRE(error == 0);
    error = EN_setpattern(ph, 1, P, 12);
    BOOST_REQUIRE(error == 0);
    //error = EN_setoption(ph, EN_DEFDEMANDPAT, 1);
    //BOOST_REQUIRE(error == 0);
    for (i = 0; i < 9; i++)
    {
        error = EN_addnode(ph, juncs[i], EN_JUNCTION, &ind);
        BOOST_REQUIRE(error == 0);
        error = EN_setnodevalue(ph, i + 1, EN_ELEVATION, e[i]);
        BOOST_REQUIRE(error == 0);
        error = EN_setnodevalue(ph, i + 1, EN_BASEDEMAND, d[i]);
        BOOST_REQUIRE(error == 0);
        error = EN_setnodevalue(ph, i+1, EN_PATTERN, 1);
        BOOST_REQUIRE(error == 0);
        error = EN_setcoord(ph, i + 1, X[i], Y[i]);
        BOOST_REQUIRE(error == 0);
        //error = EN_setdemandpattern(ph, i + 1, 1, 1);
        //BOOST_REQUIRE(error == 0);
    }
    error = EN_addnode(ph, (char *)"9", EN_RESERVOIR, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setcoord(ph, 10, 10, 70);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 10, EN_ELEVATION, 800);
    BOOST_REQUIRE(error == 0);

    error = EN_addnode(ph, (char *)"2", EN_TANK, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setcoord(ph, 11, 50, 90);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_TANKDIAM, 50.5);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_ELEVATION, 850);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MAXLEVEL, 150);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_TANKLEVEL, 120);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MINLEVEL, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MIXFRACTION, 1);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"10", EN_PIPE, (char *)"10", (char *)"11", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"11", EN_PIPE, (char *)"11", (char *)"12", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"12", EN_PIPE, (char *)"12", (char *)"13", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"21", EN_PIPE, (char *)"21", (char *)"22", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"22", EN_PIPE, (char *)"22", (char *)"23", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"31", EN_PIPE, (char *)"31", (char *)"32", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"110", EN_PIPE, (char *)"2", (char *)"12", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"111", EN_PIPE, (char *)"11", (char *)"21", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"112", EN_PIPE, (char *)"12", (char *)"22", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"113", EN_PIPE, (char *)"13", (char *)"23", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"121", EN_PIPE, (char *)"21", (char *)"31", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"122", EN_PIPE, (char *)"22", (char *)"32", &ind);
    BOOST_REQUIRE(error == 0);
    for (i = 0; i < 12; i++)
    {
        error = EN_setlinkvalue(ph, i + 1, EN_LENGTH, L[i]);
        BOOST_REQUIRE(error == 0);
        error = EN_setlinkvalue(ph, i + 1, EN_DIAMETER, dia[i]);
        BOOST_REQUIRE(error == 0);
    }

    error = EN_addlink(ph, (char *)"9", EN_PUMP, (char *)"9", (char *)"10", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, (char *)"1");
    BOOST_REQUIRE(error == 0);
    error = EN_setcurvevalue(ph, 1, 1, 1500, 250);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"9", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setheadcurveindex(ph, ind, 1);
    BOOST_REQUIRE(error == 0);

    error = EN_settimeparam(ph, EN_DURATION, 24 * 3600);
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_PATTERNSTEP, 2 * 3600);
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"9", &Lindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, (char *)"2", &Nindex);
    BOOST_REQUIRE(error == 0);

    // Add controls
    error = EN_addcontrol(ph, EN_LOWLEVEL, Lindex, 1, Nindex, 110, &Cindex);
    BOOST_REQUIRE(error == 0);
    error = EN_addcontrol(ph, EN_HILEVEL, Lindex, 0, Nindex, 140, &Cindex);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, 0);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        // this is the head at the end of the simulation after building the network *without* saving it to file
        error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_build);
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);
    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_saveinpfile(ph, "net_builder.inp");
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(test_open_net1, * boost::unit_test::depends_on("test_net_builder/test_build_net1"))
{
    int error = 0;
    int flag = 00;
    long t, tstep;
    int Nindex = -1;
    double h_orig = 0.0, h_build = 0.0, h_build_loaded = 0.0;


    EN_Project ph = NULL;

    // now we load the netwok we just built and saved
    EN_createproject(&ph);
    error = EN_open(ph, "net_builder.inp", DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"2", &Nindex);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        // this is the head at the end of the simulation after building the network and saving it to file
        error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_build_loaded);
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);

    //---------------------------------------------------------------------
    // if we got this far we can compare results

    // compare the original to the build & saved network
    //   BOOST_CHECK(abs(h_orig - h_build_loaded) < 0.0001);

    // compare the original to the build without saving
    //   BOOST_CHECK(abs(h_orig - h_build) < 0.0001);
}


BOOST_FIXTURE_TEST_CASE(test_save_net2, FixtureInitClose)
{
    //char id[EN_MAXID+1];
    double p1_1, p2_1; // p1_2, p2_2;
    double q1_1, q2_1; // q1_2, q2_2;
    int ind;

    // Build a network
    error = EN_addnode(ph, (char *)"N1", EN_JUNCTION, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"N2", EN_JUNCTION, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"N3", EN_RESERVOIR, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"N4", EN_TANK, &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"L1", EN_PUMP, (char *)"N3", (char *)"N1", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"L2", EN_PIPE, (char *)"N1", (char *)"N3", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"L3", EN_PIPE, (char *)"N1", (char *)"N2", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, (char *)"C1");
    BOOST_REQUIRE(error == 0);

    // Set network data using the new helper functions
    error = EN_setcurvevalue(ph, 1, 1, 1500, 250);
    BOOST_REQUIRE(error == 0);
    error = EN_setjuncdata(ph, 1, 700, 500, (char *)"");
    BOOST_REQUIRE(error == 0);
    error = EN_setjuncdata(ph, 2, 710, 500, (char *)"");
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 3, EN_ELEVATION, 800);
    BOOST_REQUIRE(error == 0);
    error = EN_settankdata(ph, 4, 850, 120, 100, 150, 50.5, 0, (char *)"");
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, 1, EN_PUMP_HCURVE, 1);
    BOOST_REQUIRE(error == 0);
    error = EN_setpipedata(ph, 2, 10560, 12, 100, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setpipedata(ph, 3, 5280, 14, 100, 0);
    BOOST_REQUIRE(error == 0);

    // Run hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Save results
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p1_1);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, 2, EN_PRESSURE, &p2_1);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, 1, EN_FLOW, &q1_1);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, 2, EN_FLOW, &q2_1);
    BOOST_REQUIRE(error == 0);

    // Save project
    error = EN_saveinpfile(ph, "netbuilder_test2.inp");
    BOOST_REQUIRE(error == 0);
}


BOOST_AUTO_TEST_CASE(test_reopen_net2, *boost::unit_test::depends_on("test_net_builder/test_save_net2"))
{
    int error, index;

    double p1_2, p2_2;
    double q1_2, q2_2;

    // Open the saved project file
    EN_Project ph = NULL;
    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, "netbuilder_test2.inp", DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    // Run hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Save these new results
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p1_2);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, 2, EN_PRESSURE, &p2_2);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"L1", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, index, EN_FLOW, &q1_2);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"L2", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, index, EN_FLOW, &q2_2);
    BOOST_REQUIRE(error == 0);

    // Display old & new results
    //cout << "\n  Node N1 Pressure: " << p1_1 << "  " << p1_2;
    //cout << "\n  Node N2 Pressure: " << p2_1 << "  " << p2_2;
    //cout << "\n  Link L1 Flow:     " << q1_1 << "  " << q1_2;
    //cout << "\n  Link L2 Flow:     " << q2_1 << "  " << q2_2;

    // Compare old & new results
    //    BOOST_CHECK(abs(p1_1 - p1_2) < 1.e-5);
    //    BOOST_CHECK(abs(q1_1 - q1_2) < 1.e-5);
    //    BOOST_CHECK(abs(p2_1 - p2_2) < 1.e-5);
    //    BOOST_CHECK(abs(q2_1 - q2_2) < 1.e-5);

    // Close project
    EN_close(ph);
    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()
