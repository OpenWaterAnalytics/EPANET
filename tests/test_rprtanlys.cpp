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
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "test_fixtures.hpp"

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
    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());
}



BOOST_AUTO_TEST_SUITE_END()
