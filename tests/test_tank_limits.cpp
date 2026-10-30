/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_tank_limits.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 10/30/2026
 ******************************************************************************
*/

/*
   Tests that a full tank that cannot overflow stops receiving inflow and
   that an empty tank stops delivering outflow, so that tank water is
   neither lost nor created (issue #936).
*/

#include <cmath>
#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

// Runs a single tank network fed or drained through pipe "P" and checks
// that the change in tank volume equals the volume carried by pipe "P"
// (summed with the same Euler step EPANET uses to update tank levels).
// sign = +1 when pipe P flows into the tank, -1 when it flows out.
static void check_tank_mass_balance(const char *inpFile, double sign,
                                    double finalLevel)
{
    int error = 0;
    int Tindex, Pindex;
    long t = 0, tstep = 1;
    double flow = 0.0, v0 = 0.0, v = 0.0, head = 0.0, elev = 0.0;
    double carried = 0.0;

    EN_Project ph = NULL;

    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, inpFile, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"T", &Tindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"P", &Pindex);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    do
    {
        // Warnings are allowed: junction J is cut off once the empty tank closes
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error < 100);

        error = EN_getnodevalue(ph, Tindex, EN_TANKVOLUME, &v);
        BOOST_REQUIRE(error == 0);
        if (t == 0) v0 = v;

        error = EN_getlinkvalue(ph, Pindex, EN_FLOW, &flow);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

        // Flow units are LPS and volume units are cubic meters
        carried += sign * flow * (double)tstep / 1000.0;
    } while (tstep > 0);

    // The tank ends at its limit level with pipe P closed
    // (EN_TANKLEVEL is the initial level, so use head minus elevation)
    error = EN_getnodevalue(ph, Tindex, EN_HEAD, &head);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, Tindex, EN_ELEVATION, &elev);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(std::abs(head - elev - finalLevel) < 0.001);
    BOOST_CHECK(std::abs(flow) < 0.0001);

    // No tank water is lost or created (2.3.5 was off by about 4,485 m3
    // for the full tank and 59 m3 for the empty tank)
    BOOST_CHECK_SMALL((v - v0) - carried, 0.5);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE (test_tank_limits)

BOOST_AUTO_TEST_CASE(test_full_tank_stops_filling)
{
    // Tank T fills to its maximum level of 7 m at about 0:50
    check_tank_mass_balance("./tank_full.inp", 1.0, 7.0);
}

BOOST_AUTO_TEST_CASE(test_empty_tank_stops_draining)
{
    // Tank T drains to its minimum level of 2 m at about 5:40
    check_tank_mass_balance("./tank_empty.inp", -1.0, 2.0);
}

BOOST_AUTO_TEST_SUITE_END()
