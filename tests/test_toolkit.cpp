//
// test_epanet_toolkit.cpp
//
// Date Created: January 24, 2018
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//

//#define BOOST_TEST_DYN_LINK

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <stdlib.h>
#endif

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

BOOST_AUTO_TEST_CASE (test_alloc_free)
{
    int error = 0;
    EN_Project ph = NULL;

    error = EN_createproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph != NULL);

    error = EN_deleteproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph == NULL);
}

BOOST_AUTO_TEST_CASE (test_open_close)
{
	string path_inp(DATA_PATH_INP);
	string path_rpt(DATA_PATH_RPT);
    string path_out(DATA_PATH_OUT);

	EN_Project ph = NULL;
    EN_createproject(&ph);

    int error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_CASE(test_save_reopen)
{
	string path_inp(DATA_PATH_INP);
	string inp_save("test_reopen.inp");
	string path_rpt(DATA_PATH_RPT);
	string path_out(DATA_PATH_OUT);

	EN_Project ph_save;
    EN_Project ph_reopen;

	EN_createproject(&ph_save);

	EN_open(ph_save, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
	EN_saveinpfile(ph_save, inp_save.c_str());
	EN_close(ph_save);

	EN_deleteproject(&ph_save);
	BOOST_TEST_CHECKPOINT("Saved input file");

	EN_createproject(&ph_reopen);

	EN_open(ph_reopen, inp_save.c_str(), path_rpt.c_str(), path_out.c_str());
	EN_close(ph_reopen);

	EN_deleteproject(&ph_reopen);
}

BOOST_AUTO_TEST_CASE(test_epanet)
{
    string path_inp(DATA_PATH_INP);
    string path_rpt(DATA_PATH_RPT);
    string path_out(DATA_PATH_OUT);

    EN_Project ph;

    EN_createproject(&ph);

    int error = EN_runproject(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str(), NULL);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()


struct Fixture{
    Fixture() {
        path_inp = string(DATA_PATH_INP);
        path_rpt = string(DATA_PATH_RPT);
        path_out = string(DATA_PATH_OUT);

        EN_createproject(&ph);
        error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    }

    ~Fixture() {
      error = EN_close(ph);
      EN_deleteproject(&ph);
  }

  string path_inp;
  string path_rpt;
  string path_out;

  int error;
  EN_Project ph;
};

BOOST_AUTO_TEST_SUITE(test_epanet_fixture)

BOOST_FIXTURE_TEST_CASE(test_epanet, Fixture)
{
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_report(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_hyd_step, Fixture)
{
    int flag = 00;
    long t, tstep;

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_qual_step, Fixture)
{
    int flag = 0;
    long t, tstep;

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextQ(ph, &tstep);
        BOOST_REQUIRE(error == 0);

    } while (tstep > 0);

    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_progressive_stepping, Fixture)
{
    int flag = EN_NOSAVE;
    long t, tstep_h, tstep_q;

    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);

    error = EN_openQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_initQ(ph, flag);
    BOOST_REQUIRE(error == 0);

    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_runQ(ph, &t);
        BOOST_REQUIRE(error == 0);

        error = EN_nextH(ph, &tstep_h);
        BOOST_REQUIRE(error == 0);

        error = EN_nextQ(ph, &tstep_q);
        BOOST_REQUIRE(error == 0);

    } while (tstep_h > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_closeQ(ph);
    BOOST_REQUIRE(error == 0);

}

BOOST_FIXTURE_TEST_CASE(test_setdemandpattern, Fixture)
{
    int i, j, pat_index, pat_index_2, numDemands, nnodes;
	char newpat[] = "new_pattern";

	// get the number of nodes
    error = EN_getcount(ph, EN_NODECOUNT, &nnodes);
    BOOST_REQUIRE(error == 0);

	// add a new pattern
    error = EN_addpattern(ph, newpat);
	BOOST_REQUIRE(error == 0);

	// get the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
	BOOST_REQUIRE(error == 0);

	for (i = 1; i <= nnodes; i++) {
		// get the number of demand categories
		error = EN_getnumdemands(ph, i, &numDemands);
		BOOST_REQUIRE(error == 0);

		for (j = 1; j <= numDemands; j++) {
			// set demand patterns
			error = EN_setdemandpattern(ph, i, j, pat_index);
			BOOST_REQUIRE(error == 0);
			// get demand patterns should be the same with set
			error = EN_getdemandpattern(ph, i, j, &pat_index_2);
			BOOST_REQUIRE(error == 0);
			BOOST_CHECK(pat_index == pat_index_2);
		}
	}
}
BOOST_FIXTURE_TEST_CASE(test_addpattern, Fixture)
{
    int pat_index, n_patterns_1, n_patterns_2;
    char newpat[] = "new_pattern";

    // get the number of current patterns
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_1);
    BOOST_REQUIRE(error == 0);

    // add a new pattern
    error = EN_addpattern(ph, newpat);
    BOOST_REQUIRE(error == 0);

    // get the new patterns count, shoul dbe the old one + 1
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_2);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n_patterns_1 + 1 == n_patterns_2);

    // gwt the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
    BOOST_CHECK(pat_index == n_patterns_2);
}

BOOST_FIXTURE_TEST_CASE(test_add_control, Fixture)
{
    int flag = 00;
    long t, tstep;
    double h1, h2;
    int Cindex;

    // run with original controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h1); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    // disable current controls
    error = EN_setcontrol(ph, 1, 0, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setcontrol(ph, 2, 1, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);

    // add new controls
    error = EN_addcontrol(ph, 0, 13, 1, 11, 110, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 3);
    error = EN_addcontrol(ph, 1, 13, 0, 11, 140, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 4);

    // run with new controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h2); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(abs(h1 - h2) < 1.e-5); // end head should be the same with new controls
}

BOOST_AUTO_TEST_SUITE_END()
