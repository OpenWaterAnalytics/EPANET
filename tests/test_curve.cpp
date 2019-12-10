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

    // Test of EN_setcurve and EN_getcurve
    int i;
    char id1[] = "NewCurve";
    int n1 = 5;
    double X1[] = {16.88889, 19.5, 22.13889, 25.94445, 33.33334};
    double Y1[] = {156.7, 146.5, 136.2, 117.9, 50.0};
    int n2;
    double X2[5], Y2[5];
    char id2[EN_MAXID+1];

    // Add data to a new curve
    error = EN_addcurve(ph, id1);
    BOOST_REQUIRE(error == 0);
    error = EN_getcurveindex(ph, id1, &i);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurve(ph, i, X1, Y1, n1);
    BOOST_REQUIRE(error == 0);

    // Retrieve data from curve
    error = EN_getcurve(ph, i, id2, &n2, X2, Y2);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(id2, id1));
    BOOST_REQUIRE(n2 == n1);
    for (i = 0; i < n1; i++)
    {
        BOOST_CHECK(X1[i] == X2[i]);
        BOOST_CHECK(Y1[i] == Y2[i]);
    }
}


BOOST_FIXTURE_TEST_CASE(test_curve_id_isvalid, FixtureInitClose)
{
    int index;

    error = EN_addcurve(ph, (char *)"C1");
    BOOST_REQUIRE(error == 0);

    error = EN_addcurve(ph, (char *)"C 2");
    BOOST_REQUIRE(error == 252);

    error = EN_addcurve(ph, (char *)"\"C2");
    BOOST_REQUIRE(error == 252);

    error = EN_addcurve(ph, (char *)"C;2");
    BOOST_REQUIRE(error == 252);

    EN_getcurveindex(ph, (char *)"C1", &index);
    error = EN_setcurveid(ph, index, (char *)"C;2");
    BOOST_REQUIRE(error == 252);
}


BOOST_AUTO_TEST_SUITE_END()
