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

//#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_analysis)

BOOST_FIXTURE_TEST_CASE(test_anlys_getoption, FixtureOpenClose)
{
    int i;

    std::vector<double> test(23);
    double  *array = test.data();

	std::vector<double> ref = {40.0, 0.001, 0.01, 0.5, 1.0, 0.0, 0.0, 0.0, 75.0, 0.0, 0.0, 0.0,
                               1.0, 1.0, 10.0, 2.0, 10.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_TRIALS; i<=EN_CONCENLIMIT; i++) {
        error = EN_getoption(ph, i, array++);
        BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    double temp;
    error = EN_getoption(ph, 25, &temp);
    BOOST_CHECK(error == 251);
}

BOOST_FIXTURE_TEST_CASE(test_anlys_gettimeparam, FixtureOpenClose)
{
    int i;

    std::vector<long> test(16);
    long *array = test.data();

	std::vector<long> ref = {86400, 3600, 300, 7200, 0, 3600, 0, 360, 0, 25, 0, 86400, 86400, 0, 3600, 0};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_DURATION; i<=EN_NEXTEVENTTANK; i++) {
        error = EN_gettimeparam(ph, i, array++);
        BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    long temp;
    error = EN_gettimeparam(ph, 18, &temp);
    BOOST_CHECK(error == 251);
}

BOOST_AUTO_TEST_SUITE_END()
