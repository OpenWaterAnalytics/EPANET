/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_quality.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_quality)

BOOST_FIXTURE_TEST_CASE(test_solveQ, FixtureOpenClose)
{
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_report(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_qual_step, FixtureOpenClose)
{
    int flag = 0;
    long t, tstep;

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_stepQ(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_progressive_step, FixtureOpenClose)
{
    int flag = EN_NOSAVE;
    long t, tstep_h, tstep_q;

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep_h);
        BOOST_REQUIRE(error == 0);

        error = EN_nextQ(ph, &tstep_q);
        BOOST_REQUIRE(error == 0);

    } while (tstep_h > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);

}

BOOST_FIXTURE_TEST_CASE(test_michaelis_menten_reaction_orders, FixtureOpenClose)
{
    double value = 0.0;

    error = EN_setoption(ph, EN_BULKORDER, -1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_BULKORDER, &value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(value == -1.0);

    error = EN_setoption(ph, EN_TANKORDER, -1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_TANKORDER, &value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(value == -1.0);

    // Wall reaction order remains restricted to zero or one.
    error = EN_setoption(ph, EN_WALLORDER, -1.0);
    BOOST_CHECK(error == 213);

    error = EN_setoption(ph, EN_CONCENLIMIT, 1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_quality_uses_published_hydraulic_state, FixtureOpenClose)
{
    int node21, node32;
    double quality21, quality32, massBalance;

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"21", &node21);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, (char *)"32", &node32);
    BOOST_REQUIRE(error == 0);

    error = EN_getnodevalue(ph, node21, EN_QUALITY, &quality21);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, node32, EN_QUALITY, &quality32);
    BOOST_REQUIRE(error == 0);
    error = EN_getstatistic(ph, EN_MASSBALANCE, &massBalance);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_SMALL(quality21 - 0.593764082790338, 1.e-9);
    BOOST_CHECK_SMALL(quality32 - 0.153379320274777, 1.e-9);
    BOOST_CHECK_SMALL(massBalance - 0.999999981951671, 1.e-9);
}

BOOST_AUTO_TEST_SUITE_END()
