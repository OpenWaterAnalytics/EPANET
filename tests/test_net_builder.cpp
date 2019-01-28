//
// test_net_builder.cpp
//

/*
This is a test for the network builder functions. It has three parts:
First Net1.inp is loaded, run and the head for Tank 2 at the end of the simulation is recorded (h_orig).
Then, Net1 is built from scratch using the net builder functions (EN_init, EN_addnode, EN_addlink....).
The built network is then run and the the final head of Tank 2 is recorded again (h_build).
In the last stage, the built network is saved to an INP file which is reloaded and runs. Again the final
head is recoded (h_build_loaded).

The test ends with a check that the three head values are equal.
*/

#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet2_2.h"

// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;

BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE(test_net_builder)
{
    int error = 0;
    int flag = 00;
    long t, tstep;
    int i, ind, Lindex, Nindex, Cindex;
    double h_orig, h_build, h_build_loaded;

    // first we load Net1.inp, run it and record the head in Tank 2 at the end of the simulation (h_orig)
    EN_Project ph = NULL;
    EN_createproject(&ph);

    std::string path_inp = std::string(DATA_PATH_INP);
    std::string path_rpt = std::string(DATA_PATH_RPT);
    std::string path_out = std::string(DATA_PATH_OUT);

    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_getnodeindex(ph, (char *)"2", &Nindex);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        // this is the head at the end of the simulation after loading the original Net1.inp
        error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_orig);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);

    // ------------------------------------------------------------------------
    // now we build Net1 from scratch...
    char juncs[9][10] = { "10", "11", "12", "13", "21", "22", "23", "31", "32" };
    double e[9] = {710, 710, 700, 695, 700, 695, 690, 700, 710};
    double d[9] = {0, 150, 150, 100, 150, 200, 150, 100, 100 };
    double X[9] = {20, 30, 50, 70, 30, 50, 70, 30, 50};
    double Y[9] = {70, 70, 70, 70, 40, 40, 40, 10, 10 };
    double L[12] = {10530, 5280, 5280, 5280, 5280, 5280, 200, 5280, 5280, 5280, 5280, 5280};
    double dia[12] = { 18, 14, 10, 10, 12, 6, 18, 10, 12, 8, 8, 6 };
    double P[12] = { 1.0f, 1.2f, 1.4f, 1.6f, 1.4f, 1.2f, 1.0f, 0.8f, 0.6f, 0.4f, 0.6f, 0.8f };

    error = EN_createproject(&ph);
    error = EN_init(ph, "net.rpt", "net.out", EN_GPM, EN_HW);
    error = EN_addpattern(ph, (char *)"pat1");
    BOOST_REQUIRE(error == 0);
    error = EN_setpattern(ph, 1, P, 12);
    BOOST_REQUIRE(error == 0);
    error = EN_setoption(ph, EN_DEFDEMANDPAT, 1);
    BOOST_REQUIRE(error == 0);
    for (i = 0; i < 9; i++)
    {
      error = EN_addnode(ph, juncs[i], EN_JUNCTION);
      BOOST_REQUIRE(error == 0);
      error = EN_setnodevalue(ph, i + 1,EN_ELEVATION, e[i]);
      BOOST_REQUIRE(error == 0);
      error = EN_setnodevalue(ph, i + 1, EN_BASEDEMAND, d[i]);
      BOOST_REQUIRE(error == 0);
      error = EN_setcoord(ph, i + 1, X[i], Y[i]);
      BOOST_REQUIRE(error == 0);
      //error = EN_setdemandpattern(ph, i + 1, 1, 1);
      //BOOST_REQUIRE(error == 0);
    }
    error = EN_addnode(ph, (char *)"9", EN_RESERVOIR);
    BOOST_REQUIRE(error == 0);
    error = EN_setcoord(ph, 10, 10, 70);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 10, EN_ELEVATION, 800);
    BOOST_REQUIRE(error == 0);

    error = EN_addnode(ph, (char *)"2", EN_TANK);
    BOOST_REQUIRE(error == 0);
    error = EN_setcoord(ph, 11, 50, 90);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_TANKDIAM, 50.5);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_ELEVATION, 850);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MAXLEVEL, 150);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_TANKLEVEL, 120);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MINLEVEL, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, 11, EN_MIXFRACTION, 1);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"10", EN_PIPE, (char *)"10", (char *)"11");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"11", EN_PIPE, (char *)"11", (char *)"12");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"12", EN_PIPE, (char *)"12", (char *)"13");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"21", EN_PIPE, (char *)"21", (char *)"22");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"22", EN_PIPE, (char *)"22", (char *)"23");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"31", EN_PIPE, (char *)"31", (char *)"32");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"110", EN_PIPE, (char *)"2", (char *)"12");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"111", EN_PIPE, (char *)"11", (char *)"21");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"112", EN_PIPE, (char *)"12", (char *)"22");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"113", EN_PIPE, (char *)"13", (char *)"23");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"121", EN_PIPE, (char *)"21", (char *)"31");
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"122", EN_PIPE, (char *)"22", (char *)"32");
    BOOST_REQUIRE(error == 0);
    for (i = 0; i < 12; i++)
    {
      error = EN_setlinkvalue(ph, i + 1, EN_LENGTH, L[i]);
      BOOST_REQUIRE(error == 0);
      error = EN_setlinkvalue(ph, i + 1, EN_DIAMETER, dia[i]);
      BOOST_REQUIRE(error == 0);
    }

    error = EN_addlink(ph, (char *)"9", EN_PUMP, (char *)"9", (char *)"10");
    BOOST_REQUIRE(error == 0);
    error = EN_addcurve(ph, (char *)"1");
    BOOST_REQUIRE(error == 0);
    error = EN_setcurvevalue(ph, 1, 1, 1500, 250);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"9", &ind);
    BOOST_REQUIRE(error == 0);
    error = EN_setheadcurveindex(ph, ind, 1);
    BOOST_REQUIRE(error == 0);

    error = EN_settimeparam(ph, EN_DURATION, 24*3600);
    BOOST_REQUIRE(error == 0);
    error = EN_settimeparam(ph, EN_PATTERNSTEP, 2*3600);
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"9", &Lindex);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, (char *)"2", &Nindex);
    BOOST_REQUIRE(error == 0);

    // Add controls
    error = EN_addcontrol(ph, EN_LOWLEVEL, Lindex, 1, Nindex, 110, &Cindex);
    BOOST_REQUIRE(error == 0);
    error = EN_addcontrol(ph, EN_HILEVEL, Lindex, 0, Nindex, 140, &Cindex);
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, 0);
    BOOST_REQUIRE(error == 0);
    do {
      error = EN_runH(ph, &t);
      BOOST_REQUIRE(error == 0);
      // this is the head at the end of the simulation after building the network *without* saving it to file
      error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_build);
      BOOST_REQUIRE(error == 0);
      error = EN_nextH(ph, &tstep);
      BOOST_REQUIRE(error == 0);
    } while (tstep > 0);
    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_saveinpfile(ph, "net_builder.inp");
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_deleteproject(&ph);
    BOOST_REQUIRE(error == 0);

    // ------------------------------------------------------------------------
    // now we load the netwok we just built and saved
    EN_createproject(&ph);
    error = EN_open(ph, "net_builder.inp", path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        // this is the head at the end of the simulation after building the network and saving it to file
        error = EN_getnodevalue(ph, Nindex, EN_HEAD, &h_build_loaded);
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
    //---------------------------------------------------------------------
    // if we got this far we can compare results

    // compare the original to the build & saved network
    BOOST_REQUIRE(abs(h_orig - h_build_loaded) < 0.0001);

    // compare the original to the build without saving
    BOOST_REQUIRE(abs(h_orig - h_build) < 0.0001); 

}

BOOST_AUTO_TEST_SUITE_END()
