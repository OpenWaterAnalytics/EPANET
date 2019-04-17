/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_cstrhelper.cpp
 Description:  tests for C string helper functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/16/2019
 ******************************************************************************
*/

#include <string>

#define BOOST_TEST_MODULE cstr_helper
#include <boost/test/unit_test.hpp>

#include "util/cstr_helper.h"


boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}


BOOST_AUTO_TEST_SUITE(test_cstrhelper)


BOOST_AUTO_TEST_CASE(test_duplicate) {

    std::string source = "I will be rewarded for good behavior.";
    char *dest = NULL;

    cstr_duplicate(&dest, source.c_str());
    BOOST_CHECK(check_string(dest, source));
    BOOST_CHECK(cstr_isnullterm(dest) == true);

    free(dest);
}


BOOST_AUTO_TEST_CASE(test_isvalid) {

    BOOST_CHECK(cstr_isvalid("big tank") == false);
    BOOST_CHECK(cstr_isvalid("big\"tank") == false);
    BOOST_CHECK(cstr_isvalid("big;tank") == false);

    BOOST_CHECK(cstr_isvalid("big-tank") == true);
}


BOOST_AUTO_TEST_SUITE_END()
