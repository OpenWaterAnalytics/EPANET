/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_pda.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/02/2023
 ******************************************************************************
*/

/*
   Tests the Pressure Driven Analysis option
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

BOOST_AUTO_TEST_SUITE (test_pda)

BOOST_AUTO_TEST_CASE(test_pda_model)

{
    int error = 0;
    int index;
    double count, reduction;

    EN_Project ph = NULL;
    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Set Demand Multiplier to 10 to cause negative pressures
    error = EN_setoption(ph, EN_DEMANDMULT, 10);
    BOOST_REQUIRE(error == 0);
    
    // Run single period analysis
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);

    // Solve hydraulics with default DDA option
    // which will return with neg. pressure warning code
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 6);

    // Check that 4 demand nodes have negative pressures
    error = EN_getstatistic(ph, EN_DEFICIENTNODES, &count);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(count == 4);

    // Switch to PDA with pressure limits of 20 - 100 psi
    error = EN_setdemandmodel(ph, EN_PDA, 20, 100, 0.5);
    BOOST_REQUIRE(error == 0);
    
    // Solve hydraulics again
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Check that 6 nodes had demand reductions totaling 32.66%
    error = EN_getstatistic(ph, EN_DEFICIENTNODES, &count);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(count == 6);
    error = EN_getstatistic(ph, EN_DEMANDREDUCTION, &reduction);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(reduction - 32.66) < 0.01);

    // Check that Junction 12 had full demand
    error = EN_getnodeindex(ph, (char *)"12", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, index, EN_DEMANDDEFICIT, &reduction);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(reduction) < 0.01);

    // Check that Junction 21 had a demand deficit of 413.67    
    error = EN_getnodeindex(ph, (char *)"21", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, index, EN_DEMANDDEFICIT, &reduction);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(abs(reduction - 413.67) < 0.01);

    // Clean up
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()
