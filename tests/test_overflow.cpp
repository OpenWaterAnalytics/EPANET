/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_overflow.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/02/2023
 ******************************************************************************
*/

/*
   Tests the EN_CANOVERFLOW option for Tank nodes
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

BOOST_AUTO_TEST_SUITE (test_overflow)

BOOST_AUTO_TEST_CASE(test_tank_overflow)

{
    int error = 0;
    int Nindex, Lindex;
    double level, spillage, spillage2, inflow;
    char testFile[] = "test_overflow.inp";

    EN_Project ph = NULL;

    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Get index of the tank and its inlet/outlet pipe
    error = EN_getnodeindex(ph, (char *)"2", &Nindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"110", &Lindex);
    BOOST_REQUIRE(error == 0);

    // Set initial & maximum level to 130
    error = EN_setnodevalue(ph, Nindex, EN_TANKLEVEL, 130);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, Nindex, EN_MAXLEVEL, 130);
    BOOST_REQUIRE(error == 0);

    // Set duration to 1 hr
    error = EN_settimeparam(ph, EN_DURATION, 3600);
    BOOST_REQUIRE(error == 0);

    // Solve hydraulics with default of no tank spillage allowed
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Check that tank remains full
    error = EN_getnodevalue(ph, Nindex, EN_TANKLEVEL, &level);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(level - 130.0) < 0.0001);

    // Check that there is no spillage
    error = EN_getnodevalue(ph, Nindex, EN_DEMAND, &spillage);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(spillage) < 0.0001);

    // Check that inflow link is closed
    error = EN_getlinkvalue(ph, Lindex, EN_FLOW, &inflow);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(inflow) < 0.0001);

    // Turn tank overflow option on
    error = EN_setnodevalue(ph, Nindex, EN_CANOVERFLOW, 1);
    BOOST_REQUIRE(error == 0);

    // Solve hydraulics again
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Check that tank remains full
    error = EN_getnodevalue(ph, Nindex, EN_TANKLEVEL, &level);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(level - 130.0) < 0.0001);

    // Check that there is spillage equal to tank inflow
    // (inflow has neg. sign since tank is start node of inflow pipe)
    error = EN_getnodevalue(ph, Nindex, EN_DEMAND, &spillage);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(spillage > 0.0001);
    error = EN_getlinkvalue(ph, Lindex, EN_FLOW, &inflow);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(-inflow - spillage) < 0.0001);

    // Save project to file and then close it
    error = EN_saveinpfile(ph, testFile);
    BOOST_REQUIRE(error == 0);
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    // Re-open saved file & run it
    error = EN_open(ph, testFile, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Check that tank spillage has same value as before
    error = EN_getnodevalue(ph, Nindex, EN_DEMAND, &spillage2);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(spillage - spillage2) < 0.0001);

    // Clean up
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);

}

BOOST_AUTO_TEST_SUITE_END()
