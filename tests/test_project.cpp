/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_project.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <string.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_project)

BOOST_AUTO_TEST_CASE (test_create_delete)
{
    int error = 0;
    EN_Project ph = NULL;

    error = EN_createproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph != NULL);

    error = EN_deleteproject(ph);

    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE (test_open_close)
{
	int error;

	EN_Project ph = NULL;

    EN_createproject(&ph);

    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(ph);
}

BOOST_AUTO_TEST_CASE(test_init_close)
{
	EN_Project ph = NULL;
	EN_createproject(&ph);

	int error = EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);
	BOOST_REQUIRE(error == 0);

	error = EN_close(ph);
	BOOST_REQUIRE(error == 0);

	EN_deleteproject(ph);
}

BOOST_AUTO_TEST_CASE(test_save)
{
    int error;

	EN_Project ph_save;

	EN_createproject(&ph_save);
    error = EN_open(ph_save, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    error = EN_saveinpfile(ph_save, "test_reopen.inp");
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(boost::filesystem::exists("test_reopen.inp") == true);

    error = EN_close(ph_save);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(ph_save);
}

BOOST_AUTO_TEST_CASE(test_reopen, * boost::unit_test::depends_on("test_project/test_save"))
{
    int error;

    EN_Project ph_reopen;

    EN_createproject(&ph_reopen);
	error = EN_open(ph_reopen, "test_reopen.inp", DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph_reopen);
    BOOST_REQUIRE(error == 0);
	EN_deleteproject(ph_reopen);
}

BOOST_AUTO_TEST_CASE(test_run)
{
	int error;

    EN_Project ph;

    EN_createproject(&ph);

    error = EN_runproject(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT, NULL);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(ph);
}

BOOST_FIXTURE_TEST_CASE(test_pressure_units, FixtureInitClose)
{
    int index;
    long t;
    double p, units;

    // Create basic network
    error = EN_addnode(ph, "R1", EN_RESERVOIR, &index);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, index, EN_ELEVATION, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, "J1", EN_JUNCTION, &index);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, "P1", EN_PIPE, "R1", "J1", &index);
    BOOST_REQUIRE(error == 0);

    // Run simulation and get junction pressure
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &t);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 43.33) < 1.e-5);

    // Get pressure unit and check that it is PSI
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_PSI);

    // Check that pressure unit is PSI
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_METERS);
    BOOST_REQUIRE(error == 0);

    // Change to meters and confirm that units are still PSI
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_PSI);

    // Change flow units to LPS to change to metric units and rerun simulation
    error = EN_setflowunits(ph, EN_LPS);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &t);
    BOOST_REQUIRE(error == 0);

    // Confirm that pressure is now in meters
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 30.48) < 1.e-5);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_METERS);

    // Set and check that pressure units are in kPa
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_KPA);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_KPA);
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 298.76035) < 1.e-5);

    // Set pressure to PSI and check that it remains in kPa
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_PSI);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_KPA);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
 
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_proj_fixture)

BOOST_FIXTURE_TEST_CASE(test_title, FixtureOpenClose)
{
    // How is the API user supposed to know array size?
    char c_test[3][80];

    // ref is an automatic variable and therefore doesn't need to be deleted
    std::string ref[3] = {
        " EPANET Example Network 1",
        "A simple example of modeling chlorine decay. Both bulk and",
        "wall reactions are included. "};

    error = EN_gettitle(ph, c_test[0], c_test[1], c_test[2]);
    BOOST_REQUIRE(error == 0);

    for (int i = 0; i < 3; i++) {
        std::string test (c_test[i]);
        BOOST_CHECK(check_string(test, ref[i]));
    }

   // Need a test for EN_settitle
}

BOOST_FIXTURE_TEST_CASE(test_getcount, FixtureOpenClose)
{
    int i;

	std::vector<int> test(7);
    int *array = test.data();

    std::vector<int> ref = { 11, 2, 13, 1, 1, 2, 0 };

    for (i=EN_NODECOUNT; i<=EN_RULECOUNT; i++) {
        error = EN_getcount(ph, i, array++);
        BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

	error = EN_getcount(ph, 7, &i);
	BOOST_CHECK(error == 251);
}

BOOST_FIXTURE_TEST_CASE(test_setdemandpattern, FixtureOpenClose)
{
    int i, j, pat_index, pat_index_2, numDemands, nnodes;
	char newpat[] = "new_pattern";

	// get the number of nodes
    error = EN_getcount(ph, EN_NODECOUNT, &nnodes);
    BOOST_REQUIRE(error == 0);

	// add a new pattern
    error = EN_addpattern(ph, newpat);
	BOOST_REQUIRE(error == 0);

	// get the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
	BOOST_REQUIRE(error == 0);

	for (i = 1; i <= nnodes; i++) {
		// get the number of demand categories
		error = EN_getnumdemands(ph, i, &numDemands);
		BOOST_REQUIRE(error == 0);

		for (j = 1; j <= numDemands; j++) {
			// set demand patterns
			error = EN_setdemandpattern(ph, i, j, pat_index);
			BOOST_REQUIRE(error == 0);
			// get demand patterns should be the same with set
			error = EN_getdemandpattern(ph, i, j, &pat_index_2);
			BOOST_REQUIRE(error == 0);
			BOOST_CHECK(pat_index == pat_index_2);
		}
	}
}

BOOST_FIXTURE_TEST_CASE(test_addpattern, FixtureOpenClose)
{
    int pat_index, n_patterns_1, n_patterns_2;
    char newpat[] = "new_pattern";

    // get the number of current patterns
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_1);
    BOOST_REQUIRE(error == 0);

    // add a new pattern
    error = EN_addpattern(ph, newpat);
    BOOST_REQUIRE(error == 0);

    // get the new patterns count, shoul dbe the old one + 1
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_2);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n_patterns_1 + 1 == n_patterns_2);

    // gwt the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
    BOOST_CHECK(pat_index == n_patterns_2);
}

BOOST_FIXTURE_TEST_CASE(test_add_control, FixtureOpenClose)
{
    int flag = 00;
    long t, tstep;
    double h1, h2;
    int Cindex;

    // run with original controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h1); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    // disable current controls
    error = EN_setcontrol(ph, 1, 0, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setcontrol(ph, 2, 1, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);

    // add new controls
    error = EN_addcontrol(ph, 0, 13, 1, 11, 110, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 3);
    error = EN_addcontrol(ph, 1, 13, 0, 11, 140, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 4);

    // run with new controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h2); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(abs(h1 - h2) < 1.e-5); // end head should be the same with new controls
}

BOOST_AUTO_TEST_SUITE_END()
