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

#define BOOST_TEST_MODULE cstr_helper
#include <boost/test/unit_test.hpp>

#include "util/cstr_helper.h"


BOOST_AUTO_TEST_SUITE(test_cstrhelper)


BOOST_AUTO_TEST_CASE(test_validate_id){

    BOOST_CHECK(cstr_validate_id("big tank") == false);
    BOOST_CHECK(cstr_validate_id("big\"tank") == false);
    BOOST_CHECK(cstr_validate_id("big;tank") == false);

    BOOST_CHECK(cstr_validate_id("big-tank") == true);
}


BOOST_AUTO_TEST_SUITE_END()
