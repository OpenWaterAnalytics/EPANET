/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_analysis.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_demand)

BOOST_AUTO_TEST_CASE(test_categories_save)
{
	int error = 0;
    int Nindex, ndem;

    EN_Project ph = NULL;

    error = EN_createproject(&ph);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);

    error = EN_getnodeindex(ph, (char *)"12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

	error = EN_setdemandname(ph, Nindex, ndem, (char *)"Demand category name");
    BOOST_REQUIRE(error == 0);
    error = EN_saveinpfile(ph, "net1_dem_cat.inp");
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
	BOOST_REQUIRE(error == 0);
	error = EN_deleteproject(&ph);
	BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(test_categories_reopen, * boost::unit_test::depends_on("test_demand/test_categories_save"))
{
    int error = 0;
    int Nindex, ndem;

    EN_Project ph = NULL;

	BOOST_TEST_CHECKPOINT("Reopening saved input file");
    error = EN_createproject(&ph);
	BOOST_REQUIRE(error == 0);
	error = EN_open(ph, "net1_dem_cat.inp", DATA_PATH_RPT, DATA_PATH_OUT);
	BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

	char demname[80];
	error = EN_getdemandname(ph, Nindex, ndem, demname);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(check_string(demname, "Demand category name"));

	error = EN_close(ph);
	BOOST_REQUIRE(error == 0);
	error = EN_deleteproject(&ph);
	BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()
