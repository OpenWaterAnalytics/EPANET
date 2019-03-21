/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_node.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_node)

BOOST_FIXTURE_TEST_CASE(test_junc_props, FixtureOpenClose)
{
    int index;
    const auto props = {
        EN_ELEVATION,
        EN_BASEDEMAND,
        EN_PATTERN,
        EN_EMITTER,
        EN_INITQUAL,
    };
    const size_t num_props = 5;

    std::vector<double> test (num_props);
    double *value = test.data();

    error = EN_getnodeindex(ph, (char *)"11", &index);
    std::vector<double> ref = {710.0, 150.0, 1.0, 0.0, 0.5};


    // Ranged for loop iterates over property set
    for (EN_NodeProperty p : props) {
         error = EN_getnodevalue(ph, index, p, value++);
         BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK(check_cdd_double(test, ref, 3));
}

BOOST_FIXTURE_TEST_CASE(test_tank_props, FixtureOpenClose)
{
    int index;
    const auto props = {
        EN_ELEVATION,
        EN_TANKLEVEL,
        EN_MINLEVEL,
        EN_MAXLEVEL,
        EN_TANKDIAM,
        EN_MINVOLUME
    };
    const size_t num_props = 6;

    std::vector<double> test (num_props);
    double *value = test.data();

    error = EN_getnodeindex(ph, (char *)"2", &index);
    std::vector<double> ref = {850.0, 120.0, 100.0, 150.0, 50.5, 200296.167};

    // Ranged for loop iterates over property set
    for (EN_NodeProperty p : props) {
        error = EN_getnodevalue(ph, index, p, value++);
        BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK(check_cdd_double(test, ref, 3));
}

BOOST_AUTO_TEST_SUITE_END()




BOOST_AUTO_TEST_SUITE(node_props_after_step)

BOOST_FIXTURE_TEST_CASE(test_junc_props, FixtureAfterStep)
{
    int index;
    const auto props = {
        EN_DEMAND,
        EN_HEAD,
        EN_PRESSURE,
        EN_QUALITY
    };
    const size_t num_props = 4;

    std::vector<double> test (num_props);
    double *value = test.data();

    error = EN_getnodeindex(ph, (char *)"11", &index);
    std::vector<double> ref = {179.999, 991.574, 122.006, 0.857};


    // Ranged for loop iterates over property set
    for (EN_NodeProperty p : props) {
         error = EN_getnodevalue(ph, index, p, value++);
         BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK(check_cdd_double(test, ref, 3));
}

BOOST_FIXTURE_TEST_CASE(test_tank_props, FixtureAfterStep)
{
    int index;
    const auto props = {
        EN_DEMAND,
        EN_HEAD,
        EN_PRESSURE,
        EN_QUALITY
    };
    const size_t num_props = 4;

    std::vector<double> test (num_props);
    double *value = test.data();

    error = EN_getnodeindex(ph, (char *)"2", &index);
    std::vector<double> ref = {505.383, 978.138, 55.522, 0.911};

    // Ranged for loop iterates over property set
    for (EN_NodeProperty p : props) {
        error = EN_getnodevalue(ph, index, p, value++);
        BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK(check_cdd_double(test, ref, 3));
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(setid_save_reopen)

BOOST_AUTO_TEST_CASE(test_setid_save)
{
    int error = 0;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Test of illegal node name change
    char newid_1[] = "Illegal; node name";
    error = EN_setnodeid(ph, 3, newid_1);
    BOOST_REQUIRE(error > 0);

    // Test of legal node name change
    char newid_2[] = "Node3";
    error = EN_setnodeid(ph, 3, newid_2);
    BOOST_REQUIRE(error == 0);

    // Save the project
    error = EN_saveinpfile(ph, "net1_setid.inp");
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);

}

BOOST_AUTO_TEST_CASE(test_setid_reopen, * boost::unit_test::depends_on("setid_save_reopen/test_setid_save"))
{
    int error = 0;
    int index;

    EN_Project ph = NULL;

    // Re-open the saved project
    EN_createproject(&ph);
    error = EN_open(ph, "net1_setid.inp", DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Check that 3rd node has its new name
    error = EN_getnodeindex(ph, (char *)"Node3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(index == 3);


    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()
