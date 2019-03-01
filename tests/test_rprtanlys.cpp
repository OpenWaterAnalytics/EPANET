//
// test_rprtanlys.cpp
//
// Date Created: February 28, 2019
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

#include "test_shared.hpp"


using namespace std;
using namespace boost;


BOOST_AUTO_TEST_SUITE (test_rprtanlys)

BOOST_FIXTURE_TEST_CASE(test_rprt_anlysstats, FixtureOpenClose)
{
    int i;
    double array[5];

    std::vector<double> test;
	vector<double> ref = {3.0, 7.0799498320679432e-06, 1.6680242187483429e-08,
        0.0089173150106518495, 0.99999998187144024};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_ITERATIONS; i<=EN_MASSBALANCE; i++) {
        error = EN_getstatistic(ph, i, &array[i]);
        BOOST_REQUIRE(error == 0);
    }

    test.assign(array, array + 5);
//    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());
    BOOST_CHECK(check_cdd_double(test, ref, 3));

    error = EN_getstatistic(ph, 8, &array[0]);
    BOOST_CHECK(error == 251);
}

BOOST_FIXTURE_TEST_CASE(test_anlys_getoption, FixtureOpenClose)
{
    int i;
    double array[13];

    std::vector<double> test;
	vector<double> ref = {40.0, 0.001, 0.01, 0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 75.0, 0.0, 0.0, 0.0};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_TRIALS; i<=EN_DEMANDCHARGE; i++) {
        error = EN_getoption(ph, i, &array[i]);
        BOOST_REQUIRE(error == 0);
    }

    test.assign(array, array + 13);
    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    error = EN_getoption(ph, 18, &array[0]);
    BOOST_CHECK(error == 251);
}

BOOST_FIXTURE_TEST_CASE(test_anlys_gettimeparam, FixtureOpenClose)
{
    int i;
    long array[16];

    std::vector<long> test;
	vector<long> ref = {86400, 3600, 300, 7200, 0, 3600, 0, 360, 0, 25, 0, 86400, 86400, 0, 3600, 0};

    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    error = EN_solveQ(ph);
    BOOST_REQUIRE(error == 0);


    for (i=EN_DURATION; i<=EN_NEXTEVENTTANK; i++) {
        error = EN_gettimeparam(ph, i, &array[i]);
        BOOST_REQUIRE(error == 0);
    }

    test.assign(array, array + 16);
    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    error = EN_gettimeparam(ph, 18, &array[0]);
    BOOST_CHECK(error == 251);
}
BOOST_AUTO_TEST_SUITE_END()
