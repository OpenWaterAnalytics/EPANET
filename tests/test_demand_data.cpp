/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       /test_demand_data.cpp
 Description:  tests demand data list node struct
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/18/2019
 ******************************************************************************
*/

#define BOOST_TEST_MODULE demand_data
#include <boost/test/unit_test.hpp>

#include "demand.h"
#include "epanet2_2.h"


#define DATA_PATH_NET1 "./net1.inp"
#define DATA_PATH_TMP "./tmp.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"


boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
	if (ref.compare(test) == 0)
		return true;
	else
		return false;
}


BOOST_AUTO_TEST_SUITE(test_demand_data)


BOOST_AUTO_TEST_CASE (test_create_destroy)
{
    void *data = NULL;

    data = create_demand_data(100.0, 1, NULL);
    BOOST_CHECK(data != NULL);

    delete_demand_data(&data);

	data = NULL;

	data = create_demand_data(100.0, 1, "CUB_SCOUT_BASE_CAMP");
	BOOST_CHECK(data != NULL);

	delete_demand_data(&data);
}

BOOST_AUTO_TEST_CASE(test_get_size)
{
	size_t size = get_demand_data_size();
	BOOST_CHECK(size == 8);
}


struct Fixture {
	Fixture() {
		_data = NULL;
		dlist = NULL;

		dlist = create_list(get_demand_data_size(), delete_demand_data);
		_data = create_demand_data(100.0, 1, "CUB_SCOUT_BASE_CAMP");
		
		append_list(dlist, &_data);
	}
	~Fixture() {
		delete_list(dlist);
	}
	demand_data_t *_data;
	list_t *dlist;
};

BOOST_FIXTURE_TEST_CASE(test_demand_list, Fixture)
{
	list_node_t *lnode = head_list(dlist, false);
	BOOST_CHECK(lnode != NULL);
}

BOOST_FIXTURE_TEST_CASE(test_demand_getset, Fixture)
{
	list_node_t *lnode = head_list(dlist, false);
	double demand;

	demand = get_base_demand(lnode);
	BOOST_CHECK(demand == 100.0);

	set_base_demand(lnode, 200.0);

	demand = get_base_demand(lnode);
	BOOST_CHECK(demand == 200.0);
}

BOOST_FIXTURE_TEST_CASE(test_pattern_getset, Fixture)
{
	list_node_t *lnode = head_list(dlist, false);
	int index;

	index = get_pattern_index(lnode);
	BOOST_CHECK(index == 1);

	set_pattern_index(lnode, 2);

	index = get_pattern_index(lnode);
	BOOST_CHECK(index == 2);
}

BOOST_FIXTURE_TEST_CASE(test_category_getset, Fixture)
{
	list_node_t *lnode = head_list(dlist, false);
	char *name;

	name = get_category_name(lnode);
	BOOST_CHECK(check_string(name, "CUB_SCOUT_BASE_CAMP"));

	free(name);

	set_category_name(lnode, "CUB_SCOUT_COMMAND");

	name = get_category_name(lnode);
	BOOST_CHECK(check_string(name, "CUB_SCOUT_COMMAND"));

	free(name);
}

BOOST_FIXTURE_TEST_CASE(test_convert_demand, Fixture)
{
	list_node_t *lnode = head_list(dlist, false);
	BOOST_CHECK(lnode != NULL);
	
	// 100.0 GPM == 6.31 LPS
	convert_units(lnode, 15.850);
	double demand = get_base_demand(lnode);
	BOOST_TEST(demand == 6.31, boost::test_tools::tolerance(0.01));
}

BOOST_AUTO_TEST_CASE(test_openclose)
{
	int error;

	EN_Project ph = NULL;

	EN_createproject(&ph);

	error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);
	BOOST_REQUIRE(error == 0);

	error = EN_close(ph);
	BOOST_REQUIRE(error == 0);

	EN_deleteproject(&ph);
}


#define DATA_PATH_NET1 "./net1.inp"
#define DATA_PATH_TMP "./tmp.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

struct FixtureOpenClose {
	FixtureOpenClose() {
		error = 0;
		ph = NULL;

		EN_createproject(&ph);
		error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);
	}

	~FixtureOpenClose() {
		error = EN_close(ph);
		EN_deleteproject(&ph);
	}

	int error;
	EN_Project ph;
};

BOOST_FIXTURE_TEST_CASE(test_demandname_getset, FixtureOpenClose)
{
	int Nindex, ndem;

	error = EN_getnodeindex(ph, (char *)"12", &Nindex);
	BOOST_REQUIRE(error == 0);
	error = EN_getnumdemands(ph, Nindex, &ndem);
	BOOST_REQUIRE(error == 0);
	BOOST_CHECK(ndem == 1);

	char demname[31];

	error = EN_getdemandname(ph, Nindex, ndem, demname);
	BOOST_REQUIRE(error == 0);
	BOOST_CHECK(check_string(demname, "\0"));

	error = EN_setdemandname(ph, Nindex, ndem, (char *)"Demand category name");
	BOOST_REQUIRE(error == 0);

	error = EN_getdemandname(ph, Nindex, ndem, demname);
	BOOST_REQUIRE(error == 0);
	BOOST_CHECK(check_string(demname, "Demand category name"));
}

BOOST_FIXTURE_TEST_CASE(test_demandpattern_get, FixtureOpenClose)
{
	int n, patIdx;

	error = EN_getdemandpattern(ph, 3, 1, &patIdx);
	BOOST_REQUIRE(error == 0);

	error = EN_getpatternlen(ph, patIdx, &n);
	BOOST_REQUIRE(error == 0);
	
	BOOST_CHECK(n == 12);
}

BOOST_FIXTURE_TEST_CASE(test_demandpattern_getset, FixtureOpenClose)
{
	int n, patIdx;

	double f3[] = { 3.1, 3.2, 3.3, 3.4 };

	// Create pattern
	error = EN_addpattern(ph, (char *)"Pat3");
	BOOST_REQUIRE(error == 0);

	error = EN_setpattern(ph, 3, f3, 4);
	BOOST_REQUIRE(error == 0);
	
	// Assign Pat3 to 3rd junction
	error = EN_setdemandpattern(ph, 3, 1, 3);
	BOOST_REQUIRE(error == 0);


	error = EN_getdemandpattern(ph, 3, 1, &patIdx);
	BOOST_REQUIRE(error == 0);
	
	error = EN_getpatternlen(ph, patIdx, &n);
	BOOST_REQUIRE(error == 0);
	BOOST_CHECK(n == 4);
}

BOOST_AUTO_TEST_SUITE_END()
