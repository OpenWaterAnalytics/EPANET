//
// test_demand_categories.cpp
//

/*
This is a test for the demand categories names get\set APIs
A demand category name is set, the network is saved, reopened and the new demand category name is checked.
*/

#define BOOST_TEST_MODULE "demands"

#include "shared_test.hpp"


BOOST_AUTO_TEST_SUITE (test_demands)

BOOST_AUTO_TEST_CASE(test_categories_save)
{
	//std::string path_inp(DATA_PATH_NET1);
	//std::string inp_save = "net1_dem_cat.inp";
	//std::string path_rpt(DATA_PATH_RPT);
	//std::string path_out(DATA_PATH_OUT);


	int error = 0;
    int Nindex, ndem;

    EN_Project ph = NULL;

    error = EN_createproject(&ph);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);

    error = EN_getnodeindex(ph, "12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

	error = EN_setdemandname(ph, Nindex, ndem, "Demand category name");
    BOOST_REQUIRE(error == 0);
    error = EN_saveinpfile(ph, "net1_dem_cat.inp");
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
	BOOST_REQUIRE(error == 0);
	error = EN_deleteproject(&ph);
	BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(test_categories_reopen, * boost::unit_test::depends_on("test_demands/test_categories_save"))
{
    //std::string inp_save = "net1_dem_cat.inp";
    //std::string path_rpt(DATA_PATH_RPT);
    //std::string path_out(DATA_PATH_OUT);
   
    char demname[80];

    int error = 0;
    int Nindex, ndem;

    EN_Project ph = NULL;

	BOOST_TEST_CHECKPOINT("Reopening saved input file");
    error = EN_createproject(&ph);
	BOOST_REQUIRE(error == 0);
	error = EN_open(ph, "net1_dem_cat.inp", DATA_PATH_RPT, DATA_PATH_OUT);
	BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, "12", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnumdemands(ph, Nindex, &ndem);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ndem == 1);

	error = EN_getdemandname(ph, Nindex, ndem, demname);
    BOOST_REQUIRE(error == 0);
	
    BOOST_CHECK(check_string(demname, "Demand category name"));

	error = EN_close(ph);
	BOOST_REQUIRE(error == 0);
	error = EN_deleteproject(&ph);
	BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()
