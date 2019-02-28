//
// test_project.cpp
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

#define BOOST_TEST_MODULE hydqual
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "test_fixtures.hpp"

using namespace std;
namespace utf = boost::unit_test;


BOOST_AUTO_TEST_SUITE (test_hyd_qual)

BOOST_FIXTURE_TEST_CASE(test_solveH_solveQ, FixtureOpenClose)
{
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_report(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_hyd_step, FixtureOpenClose)
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

BOOST_FIXTURE_TEST_CASE(test_qual_step, FixtureOpenClose)
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

BOOST_FIXTURE_TEST_CASE(test_progressive_step, FixtureOpenClose)
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

BOOST_FIXTURE_TEST_CASE(test_hydr_save, FixtureOpenClose)
{
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_saveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_report(ph);
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(test_hydr_savefile, FixtureOpenClose)
{
    string hyd_file("test_savefile.hyd");

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_savehydfile(ph, hyd_file.c_str());
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(filesystem::exists(hyd_file) == true);
}

BOOST_FIXTURE_TEST_CASE(test_hydr_usefile, FixtureOpenClose, * utf::depends_on("test_hyd_qual/test_hydr_savefile"))
{
    string hyd_file("test_savefile.hyd");

    error = EN_usehydfile(ph, hyd_file.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);
}



BOOST_AUTO_TEST_SUITE_END()
