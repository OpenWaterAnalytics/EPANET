/*
******************************************************************************
Project:      OWA EPANET
Version:      2.4
Module:       test_lua.cpp
Description:  Tests the Lua scripting engine
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
******************************************************************************
*/

#define BOOST_TEST_MODULE lua

#include <cmath>

#include <boost/test/included/unit_test.hpp>

#include "epanet2_2.h"


#define DATA_PATH_LUA_PRV "./lua-prv.inp"
#define DATA_PATH_RPT "./test_lua.rpt"
#define DATA_PATH_OUT "./test_lua.out"

#define PRV_TARGET_PRESSURE 30.0

struct FixtureOpenLuaPrv {
    FixtureOpenLuaPrv() {
        error = 0;
        ph = NULL;

        EN_createproject(&ph);
        error = EN_open(ph, DATA_PATH_LUA_PRV, DATA_PATH_RPT, DATA_PATH_OUT);
    }

    ~FixtureOpenLuaPrv() {
        EN_close(ph);
        EN_deleteproject(ph);
    }

    int error;
    EN_Project ph;
};


BOOST_AUTO_TEST_SUITE(test_lua)

BOOST_FIXTURE_TEST_CASE(script_section_parses, FixtureOpenLuaPrv)
{
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(script_steers_prv_toward_target, FixtureOpenLuaPrv)
{
    int nodeIndex, linkIndex;
    long t, tstep;
    double pressure, initialSetting, lastSetting = 0.0, maxDeviation = 0.0;

    BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"J126", &nodeIndex);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"V1", &linkIndex);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, linkIndex, EN_SETTING, &initialSetting);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);

    do
    {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_getnodevalue(ph, nodeIndex, EN_PRESSURE, &pressure);
        BOOST_REQUIRE(error == 0);
        error = EN_getlinkvalue(ph, linkIndex, EN_SETTING, &lastSetting);
        BOOST_REQUIRE(error == 0);

        if (std::abs(pressure - PRV_TARGET_PRESSURE) > maxDeviation)
        {
            maxDeviation = std::abs(pressure - PRV_TARGET_PRESSURE);
        }

        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    // A setting change re-triggers the solver's status checks, so the
    // script converges on the target within each time step: J126 must
    // stay at the target pressure for the whole run, which is only
    // possible if the script pulled the valve away from its fixed setting
    BOOST_CHECK_SMALL(maxDeviation, 0.5);
    BOOST_CHECK(std::abs(lastSetting - initialSetting) > 1.0);
}

BOOST_AUTO_TEST_SUITE_END()
