/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_valve.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 07/28/2022
 ******************************************************************************
*/

/*
   Tests PCV valve with position curve
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

BOOST_AUTO_TEST_SUITE (test_valve)

BOOST_FIXTURE_TEST_CASE(test_PCV_valve, FixtureOpenClose)

{
    int npts = 5;
    double x[] = { 0.0, 25., 50., 75., 100. };
    double y[] = {0.0, 8.9, 18.4, 40.6, 100.0};
    double v;
    int linkIndex, curveIndex, curveType;

    // Make steady state run
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);

    // Convert pipe 22 to a PCV
    error = EN_getlinkindex(ph, (char*)"22", &linkIndex);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinktype(ph, &linkIndex, EN_PCV, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, linkIndex, EN_DIAMETER, 12);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, linkIndex, EN_MINORLOSS, 0.19);

    // Create the PCV's position-loss curve
    error = EN_addcurve(ph, (char*)"ValveCurve");
    BOOST_REQUIRE(error == 0);
    error = EN_getcurveindex(ph, (char*)"ValveCurve", &curveIndex);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurve(ph, curveIndex, x, y, npts);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurvetype(ph, curveIndex, EN_VALVE_CURVE);
    BOOST_REQUIRE(error == 0);
    error = EN_getcurvetype(ph, curveIndex, &curveType);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(curveType == EN_VALVE_CURVE);

    // Assign curve & initial setting to PCV
    error = EN_setlinkvalue(ph, linkIndex, EN_PCV_CURVE, curveIndex);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, linkIndex, EN_INITSETTING, 35.);
    BOOST_REQUIRE(error == 0);

    // Solve for hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // The PCV interpolated relative flow coeff. at 35% open is 0.127.
    // This translates to a minor loss coeff. of 0.19 / 0.127^2 = 11.78.
    // If the PCV were replaced with a TCV at that setting the resulting
    // head loss would be 0.0255 ft which should equal the PCV result. 
    error = EN_getlinkvalue(ph, linkIndex, EN_HEADLOSS, &v);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(v - 0.0255) < 0.001);
}

BOOST_FIXTURE_TEST_CASE(test_GPV_accepts_toolkit_generic_curve, FixtureInitClose)
{
    int reservoir = 0;
    int junction = 0;
    int link = 0;
    int curve = 0;
    int replacementCurve = 0;
    int curveType = -1;
    long time = 0;
    double flow = 0.0;
    double headloss = 0.0;
    double x[] = {0.0, 200.0, 400.0};
    double y[] = {0.0, 20.0, 40.0};
    double replacementY[] = {0.0, 40.0, 80.0};

    error = EN_addnode(ph, "R1", EN_RESERVOIR, &reservoir);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, reservoir, EN_ELEVATION, 100.0);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, "J1", EN_JUNCTION, &junction);
    BOOST_REQUIRE(error == 0);
    error = EN_setjuncdata(ph, junction, 0.0, 200.0, "");
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, "V1", EN_GPV, "R1", "J1", &link);
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, "GPV-CURVE");
    BOOST_REQUIRE(error == 0);
    error = EN_getcurveindex(ph, "GPV-CURVE", &curve);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurve(ph, curve, x, y, 3);
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, "GPV-REPLACEMENT");
    BOOST_REQUIRE(error == 0);
    error = EN_getcurveindex(ph, "GPV-REPLACEMENT", &replacementCurve);
    BOOST_REQUIRE(error == 0);
    error = EN_setcurve(ph, replacementCurve, x, replacementY, 3);
    BOOST_REQUIRE(error == 0);

    // EN_addcurve deliberately creates a generic curve. A GPV must compile
    // the curve because it references it, not because parser metadata says it
    // is a head-loss curve.
    error = EN_getcurvetype(ph, curve, &curveType);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE_EQUAL(curveType, EN_GENERIC_CURVE);
    error = EN_setlinkvalue(ph, link, EN_GPV_CURVE, curve);
    BOOST_REQUIRE(error == 0);

    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_INITFLOW);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &time);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, link, EN_FLOW, &flow);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, link, EN_HEADLOSS, &headloss);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_CLOSE_FRACTION(flow, 200.0, 1.e-6);
    BOOST_CHECK_CLOSE_FRACTION(headloss, 20.0, 1.e-6);

    // The replacement curve was not referenced when the full solver model was
    // compiled. Assigning it while hydraulics are open must compile it on
    // demand and update the active GPV setting.
    error = EN_setlinkvalue(ph, link, EN_GPV_CURVE, replacementCurve);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &time);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, link, EN_HEADLOSS, &headloss);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_CLOSE_FRACTION(headloss, 40.0, 1.e-6);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()
