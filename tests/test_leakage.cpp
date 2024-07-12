/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       test_leakage.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 06/26/2024
 ******************************************************************************
*/

/*
   Tests Pipe Leakage Feature
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

BOOST_AUTO_TEST_SUITE (test_leakage)

BOOST_AUTO_TEST_CASE(test_leakage_model)

//#include <stdio.h>
//#include <math.h>
//#include "epanet2_2.h"

//int main()
{
    int error = 0;
    int Pipe21, Junc21, Junc22;
	double pipe21Leak, junc21Leak, junc22Leak;
    EN_Project ph = NULL;

    error = EN_createproject(&ph);
    BOOST_REQUIRE(error == 0);
    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
//    error = EN_open(ph, "Net1.inp", "Net1.rpt", "");
    BOOST_REQUIRE(error == 0);
    
    //  single period analysis
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);

    // Get index of Pipe 21
    error = EN_getlinkindex(ph, "21", &Pipe21);
    BOOST_REQUIRE(error == 0);

    // Set Pipe21 leak area to 1.0 sq mm per 100 ft of pipe
    // and its expansion rate to 0.1 sq mm per ft of head
    error = EN_setlinkvalue(ph, Pipe21, EN_LEAK_AREA, 1.0);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, Pipe21, EN_LEAK_EXPAN, 0.1);
    BOOST_REQUIRE(error == 0);

    // Solve for hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Compute Pipe 21 leakage flow using the FAVAD formula
    // Note: we can't just sum the leak rates at both end nodes
    //       together since in general the nodes can have leakage
    //       contributed by other connecting pipes.
    error = EN_getlinkvalue(ph, Pipe21, EN_LINK_LEAKAGE, &pipe21Leak);
    BOOST_REQUIRE(error == 0);
//    printf("\n Pipe leakage flow: %.4f", pipe21Leak);

	// Retrieve leakage flow at end nodes
    // Note: In this case all of the leakage at these nodes is from Pipe 21.
    error = EN_getnodeindex(ph, "21", &Junc21);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, "22", &Junc22);
    BOOST_REQUIRE(error == 0);
	error = EN_getnodevalue(ph, Junc21, EN_LEAKAGEFLOW, &junc21Leak);
    BOOST_REQUIRE(error == 0);
	error = EN_getnodevalue(ph, Junc22, EN_LEAKAGEFLOW, &junc22Leak);
    BOOST_REQUIRE(error == 0);

	// Check that the sum of the node leakages equals the pipe leakage
	//printf("\n Node leakage flow: %.4f\n", junc21Leak + junc22Leak);
    BOOST_REQUIRE(abs(pipe21Leak - (junc21Leak+junc22Leak)) < 0.01);

// Clean up
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(ph);
    BOOST_REQUIRE(error == 0);
//    return 0;
}

BOOST_AUTO_TEST_SUITE_END()
