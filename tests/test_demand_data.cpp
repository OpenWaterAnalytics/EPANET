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


#define DATA_PATH_NET1 "./example1_mdc.inp"
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


BOOST_AUTO_TEST_CASE(test_create_destroy_demand_list)
{
    int key;
    list_t *dlist;

    dlist = create_demand_list(100.0, 1, "CUB_SCOUT_DAY_CAMP", &key);
    BOOST_CHECK(dlist != NULL);
    BOOST_CHECK(key != NULL);

    delete_list(dlist);
}


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
    BOOST_CHECK(size == sizeof(demand_data_t *));
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
    char *name = NULL;

    name = get_category_name(lnode);
    BOOST_CHECK(check_string(name, (char *)"CUB_SCOUT_BASE_CAMP"));

    free(name);
    name = NULL;

    set_category_name(lnode, (char *)"CUB_SCOUT_COMMAND");

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

BOOST_AUTO_TEST_CASE(test_initclose)
{
    int error;

    EN_Project ph = NULL;

    EN_createproject(&ph);

    error = EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}


struct FixtureSingleNode {
    FixtureSingleNode() {
        error = 0;
        ph = NULL;

        EN_createproject(&ph);
        EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);

        EN_addnode(ph, (char *)"CUB_SCOUT_QUONSET_HUT", EN_JUNCTION, &node_qhut);
        //EN_getnodeindex(ph, (char *)"CUB_SCOUT_QUONSET_HUT", &node_qhut);
    }

    ~FixtureSingleNode() {
        EN_close(ph);
        EN_deleteproject(&ph);
    }
    int error, index, node_qhut;
    EN_Project ph;
};


BOOST_FIXTURE_TEST_CASE(test_replace_demand, FixtureSingleNode)
{
    error = EN_addpattern(ph, (char *)"Pat5");
    BOOST_REQUIRE(error == 0);

    error = EN_setjuncdata(ph, node_qhut, 555.5, 55.5, (char *)"Pat5");
    BOOST_CHECK(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_single_node, FixtureSingleNode)
{
    int demand_idx, pattern_idx, n;
    double demand;

    error = EN_getnumdemands(ph, node_qhut, &n);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n == 0);

    demand_idx = 1;
    error = EN_getbasedemand(ph, node_qhut, demand_idx, &demand);
    BOOST_REQUIRE(error == 253);

    error = EN_getdemandpattern(ph, node_qhut, demand_idx, &pattern_idx);
    BOOST_REQUIRE(error == 253);

    char demname[31];
    error = EN_getdemandname(ph, node_qhut, demand_idx, demname);
    BOOST_REQUIRE(error == 253);
    BOOST_CHECK(check_string(demname, "\0"));

    error = EN_setbasedemand(ph, node_qhut, demand_idx, 100.0);
    BOOST_REQUIRE(error == 0);

    // only one demand category
    pattern_idx = 1;
    error = EN_setdemandpattern(ph, node_qhut, demand_idx, pattern_idx);
    BOOST_REQUIRE(error == 205);

    // create pattern
    error = EN_addpattern(ph, (char *)"Pat2");
    BOOST_REQUIRE(error == 0);
    error = EN_getpatternindex(ph, (char *)"Pat2", &pattern_idx);
    BOOST_REQUIRE(error == 0);

    error = EN_setdemandpattern(ph, node_qhut, demand_idx, pattern_idx);
    BOOST_REQUIRE(error == 0);

    error = EN_setdemandname(ph, node_qhut, demand_idx, (char *)"CUB_SCOUT_MESS_HALL");
    BOOST_REQUIRE(error == 0);

}


BOOST_FIXTURE_TEST_CASE(test_pattern_edits, FixtureSingleNode)
{
    int n, node_cpoint, pat2_idx, pat3_idx;

    EN_addnode(ph, (char *)"CUB_SCOUT_CHECKPOINT", EN_JUNCTION, &node_cpoint);
    //EN_getnodeindex(ph, (char *)"CUB_SCOUT_CHECKPOINT", &node_cpoint);

    // Add 2 new patterns
    error = EN_addpattern(ph, (char *)"DefPat");
    BOOST_REQUIRE(error == 0);
    error = EN_addpattern(ph, (char *)"Pat2");
    BOOST_REQUIRE(error == 0);
    error = EN_getpatternindex(ph, (char *)"Pat2", &pat2_idx);
    BOOST_REQUIRE(error == 0);

    error = EN_addpattern(ph, (char *)"Pat3");
    BOOST_REQUIRE(error == 0);
    error = EN_getpatternindex(ph, (char *)"Pat3", &pat3_idx);
    BOOST_REQUIRE(error == 0);

    double f2[] = { 2.1, 2.2 };
    double f3[] = { 3.1, 3.2, 3.3, 3.4 };
    error = EN_setpattern(ph, pat2_idx, f2, 2);
    BOOST_REQUIRE(error == 0);
    error = EN_setpattern(ph, pat3_idx, f3, 4);
    BOOST_REQUIRE(error == 0);

    // Assign Pat3 to 3rd junction
    error = EN_setdemandpattern(ph, node_cpoint, 1, pat3_idx);
    BOOST_REQUIRE(error == 0);

    // Delete Pat2
    error = EN_deletepattern(ph, pat2_idx);
    BOOST_REQUIRE(error == 0);

    //Check that there are now 2 patterns
    error = EN_getcount(ph, EN_PATCOUNT, &n);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n == 2);

    // Check that Pat3 with 4 factors is still assigned to 3rd junction
    error = EN_getdemandpattern(ph, node_cpoint, 1, &pat3_idx);
    BOOST_REQUIRE(error == 0);
    error = EN_getpatternlen(ph, pat3_idx, &n);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n == 4);
}

struct FixtureOpenClose{
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


BOOST_FIXTURE_TEST_CASE(test_demand_parse, FixtureOpenClose)
{
    int n, node_idx;

    error = EN_getnodeindex(ph, "22", &node_idx);
    BOOST_CHECK(error == 0);

    error = EN_getnumdemands(ph, node_idx, &n);
    BOOST_CHECK(error == 0);
    BOOST_CHECK(n == 3);
}


BOOST_AUTO_TEST_SUITE_END()
