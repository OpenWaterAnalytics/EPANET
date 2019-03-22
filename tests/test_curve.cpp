/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_curve.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (curve)

BOOST_FIXTURE_TEST_CASE(test_curve_comments, FixtureOpenClose)
{
    int index;
    char comment[EN_MAXMSG + 1];

    // Set curve comments
    error = EN_getcurveindex(ph, (char *)"1", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_setcomment(ph, EN_CURVE, index, (char *)"Curve 1");
    BOOST_REQUIRE(error == 0);

    // Check curve comments
    error = EN_getcurveindex(ph, (char *)"1", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getcomment(ph, EN_CURVE, index, comment);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(comment, (char *)"Curve 1"));
}

BOOST_AUTO_TEST_SUITE_END()
