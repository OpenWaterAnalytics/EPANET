/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_demand.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/31/2019
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

    // Get the number of demand categories for Node 12
    error = EN_getnodeindex(ph, (char *)"12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

    // Name the current category and add a new one
    error = EN_setdemandname(ph, Nindex, ndem, (char *)"Demand category one");
    BOOST_REQUIRE(error == 0);
    error = EN_adddemand(ph, Nindex, 120.0, 0, (char *)"Demand category two");
    BOOST_REQUIRE(error == 0);

    // Check that EN_getnodevalue & EN_getbasedemand return same values
    double v1, v2;
    EN_getnodevalue(ph, Nindex, EN_BASEDEMAND, &v1);
    EN_getbasedemand(ph, Nindex, 1, &v2);
    BOOST_REQUIRE(v1 == v2);

    // Add some additional categories for testing
    error = EN_adddemand(ph, Nindex, 120.0, 0, (char *)"Cat3");
    BOOST_REQUIRE(error == 0);
    error = EN_adddemand(ph, Nindex, 120.0, 0, (char *)"Cat4");
    BOOST_REQUIRE(error == 0);

    // Delete the last category
    error = EN_deletedemand(ph, Nindex, 4);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 3);

    // Delete the middle category
    error = EN_deletedemand(ph, Nindex, 2);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 2);

    // Delete the first category
    error = EN_deletedemand(ph, Nindex, 1);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

    // Rename the remaining category and add a new one
    error = EN_setdemandname(ph, Nindex, ndem, (char *)"Demand category one");
    BOOST_REQUIRE(error == 0);
    error = EN_adddemand(ph, Nindex, 120.0, 0, (char *)"Demand category two");
    BOOST_REQUIRE(error == 0);

    // Save the project and delete it
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

    // Re-open the previously saved project
    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, "net1_dem_cat.inp", DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);

    // Check that Node 12 now has 2 demand categories
    error = EN_getnodeindex(ph, (char *)"12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 2);

    // Check that both demand categories have the correct names
    char demname[80];
    error = EN_getdemandname(ph, Nindex, 1, demname);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(demname, "Demand category one"));

    error = EN_getdemandname(ph, Nindex, 2, demname);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(demname, "Demand category two"));

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(&ph);
    BOOST_REQUIRE(error == 0);
    remove("net1_dem_cat.inp");
}

BOOST_AUTO_TEST_SUITE_END()
