/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_report.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_report)

BOOST_FIXTURE_TEST_CASE(test_rprt_anlysstats, FixtureOpenClose)
{
    int i;

    std::vector<double> test(5);
    double *array = test.data();

	std::vector<double> ref = {3.0, 7.0799498320679432e-06, 1.6680242187483429e-08,
        0.0089173150106518495, 0.99999998187144024};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_ITERATIONS; i<=EN_MASSBALANCE; i++) {
        error = EN_getstatistic(ph, i, array++);
        BOOST_REQUIRE(error == 0);
    }
    BOOST_CHECK(check_cdd_double(test, ref, 3));

    double temp;
    error = EN_getstatistic(ph, 8, &temp);
    BOOST_CHECK(error == 251);
}

BOOST_AUTO_TEST_SUITE_END()
