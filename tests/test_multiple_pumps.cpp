//
// test_multiple_pumps.cpp
//

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <string>
#include "epanet2.h"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE(test_multiple_pumps)
{
    int error = 0;
    int flag = 00;
    long t, tstep;
    int i, ind;
    float q_build_1, q_build_2, q_load_1, q_load_2;
    
    // first we build a network with 2 pumps, run it and record the flow in each pump
    EN_ProjectHandle ph = NULL;
    EN_createproject(&ph);
    
    error = EN_init(ph, "net.rpt", "net.out", EN_CMH, EN_HW);
    BOOST_REQUIRE(error == 0);

    error = EN_addnode(ph, (char *)"R1", EN_RESERVOIR);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"J1", EN_JUNCTION);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"J2", EN_JUNCTION);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"J3", EN_JUNCTION);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"J4", EN_JUNCTION);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, (char *)"R2", EN_RESERVOIR);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"P1", EN_PIPE, (char *)"R1", (char *)"J1");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"P2", EN_PIPE, (char *)"J2", (char *)"J4");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"P3", EN_PIPE, (char *)"J3", (char *)"J4");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"P4", EN_PIPE, (char *)"J4", (char *)"R2");
    BOOST_REQUIRE(error == 0);



    for (i = 0; i < 4; i++)
    {
      error = EN_setlinkvalue(ph, i + 1, EN_LENGTH, 100);
      BOOST_REQUIRE(error == 0);
      error = EN_setlinkvalue(ph, i + 1, EN_DIAMETER, 200);
      BOOST_REQUIRE(error == 0);
    }

    error = EN_addlink(ph, (char *)"PU1", EN_PUMP, (char *)"J1", (char *)"J2");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"PU2", EN_PUMP, (char *)"J1", (char *)"J3");
    BOOST_REQUIRE(error == 0);

    error = EN_addcurve(ph, (char *)"1");
    BOOST_REQUIRE(error == 0);

    EN_API_FLOAT_TYPE c1[3] = {100,200,300};
    EN_API_FLOAT_TYPE c2[3] = {30,20,10};
    
    error = EN_setcurve(ph, 1, c1, c2, 3);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"PU1", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setheadcurveindex(ph, ind, 1);
    BOOST_REQUIRE(error == 0);

    error = EN_addcurve(ph, (char *)"2");
    BOOST_REQUIRE(error == 0);
    EN_API_FLOAT_TYPE c3[3] = {200,250,400};
    EN_API_FLOAT_TYPE c4[3] = {40,25,10};
    error = EN_setcurve(ph, 2, c3, c4, 3);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"PU2", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setheadcurveindex(ph, ind, 2);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, 0);
    BOOST_REQUIRE(error == 0);

    error = EN_runH(ph, &t);
    cout << "\nGenerated Network has Error " << error;
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"PU1", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, ind, EN_FLOW, &q_build_1);
    BOOST_REQUIRE(error == 0);
    cout << "\nFlow from pump1: " << q_build_1;

    error = EN_getlinkindex(ph, (char *)"PU2", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, ind, EN_FLOW, &q_build_2);
    BOOST_REQUIRE(error == 0);
    cout << "   Flow from pump2: " << q_build_2;
    

    error = EN_saveinpfile(ph, "net_builder.inp");
    BOOST_REQUIRE(error == 0);
    
    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(&ph);
    BOOST_REQUIRE(error == 0);

    // ------------------------------------------------------------------------
    // now we load the netwok we just built and saved
    EN_createproject(&ph);
    error = EN_open(ph, "net_builder.inp", "net_builder.rpt", "net_builder.out");
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    error = EN_runH(ph, &t);
    cout << "\nSaved Network has Error " << error;
    BOOST_REQUIRE(error == 0);
    
    error = EN_getlinkindex(ph, (char *)"PU1", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, ind, EN_FLOW, &q_load_1);
    BOOST_REQUIRE(error == 0);
    cout << "\nFlow from pump1: " << q_load_1;

    error = EN_getlinkindex(ph, (char *)"PU2", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, ind, EN_FLOW, &q_load_2);
    BOOST_REQUIRE(error == 0);
    cout << "   Flow from pump2: " << q_load_2;

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
    
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph); 

    // compare the original to the build & saved network
    BOOST_REQUIRE(q_build_1 == q_load_1);
    BOOST_REQUIRE(q_build_2 == q_load_2);
}

BOOST_AUTO_TEST_SUITE_END()
