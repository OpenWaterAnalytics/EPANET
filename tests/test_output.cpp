/*
 *   test_epanet_output.cpp
 *
 *   Created: 8/4/2017
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Unit testing for EPANET Output API.
*/

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "output"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "epanet_output.h"

//#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/epanet/"
#define DATA_PATH "./net1.out"

using namespace std; 

// Custom test to check the minimum number of correct decimal digits between 
// the test and the ref vectors.  
boost::test_tools::predicate_result check_cdd(std::vector<float>& test, 
    std::vector<float>& ref, long cdd_tol)
{
    float tmp, min_cdd = 100.0;
    
    // TODO: What is the vectors aren't the same length? 

    std::vector<float>::iterator test_it;
    std::vector<float>::iterator ref_it;

    for (test_it = test.begin(); test_it < test.end(); ++test_it) {
        for (ref_it = ref.begin(); ref_it < ref.end(); ++ref_it) {
             
             if (*test_it != *ref_it) {
                tmp = - log10f(abs(*test_it - *ref_it));
                if (tmp < min_cdd) min_cdd = tmp;
            }
        }
    }

    if (min_cdd == 100.0)
        return true; 
    else
        return std::lround(min_cdd) <= cdd_tol;
}


BOOST_AUTO_TEST_SUITE (test_output_auto)

BOOST_AUTO_TEST_CASE(InitTest) {
    ENR_Handle p_handle = NULL;

    int error = ENR_init(&p_handle);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(p_handle != NULL);
}

BOOST_AUTO_TEST_CASE(OpenTest) {
    std::string path = std::string(DATA_PATH);
    ENR_Handle p_handle = NULL;
    ENR_init(&p_handle);

    int error = ENR_open(p_handle, path.c_str());
    BOOST_REQUIRE(error == 0);
    ENR_close(&p_handle);
}

BOOST_AUTO_TEST_CASE(CloseTest) {
    ENR_Handle p_handle = NULL;
    int error = ENR_init(&p_handle);

    error = ENR_close(&p_handle);
    BOOST_REQUIRE(error == -1);
    BOOST_CHECK(p_handle != NULL);
}

BOOST_AUTO_TEST_SUITE_END()


struct Fixture{
    Fixture() {
        path = std::string(DATA_PATH);

        error = ENR_init(&p_handle);
        ENR_clearError(p_handle);
        error = ENR_open(p_handle, path.c_str());
    }
    ~Fixture() {
        ENR_free((void**)&array);
        error = ENR_close(&p_handle);
    }
    
    std::string path;
    int error = 0;
    ENR_Handle p_handle = NULL;

    float* array = NULL;
    int array_dim = 0;

};

BOOST_AUTO_TEST_SUITE(test_output_fixture)

BOOST_FIXTURE_TEST_CASE(test_getNetSize, Fixture)
{
    int *i_array = NULL;
    
    error = ENR_getNetSize(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    // nodes, tanks, links, pumps, valves
    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);
    std::vector<int> ref({11,2,13,1,0});

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    ENR_free((void**)&i_array);
}

BOOST_FIXTURE_TEST_CASE(test_getElementName, Fixture) {
    char* name = new char[MAXID];
    int length, index = 1;

    error = ENR_getElementName(p_handle, ENR_node, index, &name, &length);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK("10" == name);

    delete(name);
}

BOOST_FIXTURE_TEST_CASE(test_getNodeAttribute, Fixture) {
    
    error = ENR_getNodeAttribute(p_handle, 1, ENR_quality, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({ 1.0,
                                 0.44407997,
                                 0.43766347,
                                 0.42827705,
                                 0.41342604,
                                 0.42804748,
                                 0.44152543,
                                 0.40502965,
                                 0.38635802,
                                 1.0,
                                 0.96745253 });

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkAttribute, Fixture) {

    error = ENR_getLinkAttribute(p_handle, 1, ENR_flow, &array ,&array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({ 1848.5812,
                                 1220.4274,
                                 130.11162,
                                 187.6893,
                                 119.8884,
                                 40.464489,
                                 -748.58112,
                                 478.15378,
                                 191.73459,
                                 30.111609,
                                 140.46449,
                                 59.535515,
                                 1848.5812});


    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNodeResult, Fixture) {
    
    error = ENR_getNodeResult(p_handle, 1, 2, &array, &array_dim);
    BOOST_REQUIRE(error == 0);
    
    std::vector<float> ref_vec({0.041142918,
                                150.0,
                                987.98358,
                                120.45029});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkResult, Fixture) {
    
    error = ENR_getLinkResult(p_handle, 24, 13, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({0.58586824,
                                1892.2433,
                                0.0,
                                -200.71875,
                                1.0,
                                3.0,
                                1.0,
                                0.0});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNodeSeries, Fixture){
    
    error = ENR_getNodeSeries(p_handle, 2, ENR_pressure, 0, 10, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({119.25731,
                                120.45029,
                                121.19854,
                                122.00622,
                                122.37414,
                                122.8122,
                                122.82034,
                                122.90379,
                                123.40434,
                                123.81807});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);
    
    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkSeries, Fixture) {
    
    error = ENR_getLinkSeries(p_handle, 2, ENR_flow, 0, 10, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({1234.2072,
                                1220.4274,
                                1164.4,
                                1154.8175,
                                1100.0635,
                                1094.759,
                                1041.7854,
                                1040.7617,
                                1087.556,
                                1082.5011});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNetReacts, Fixture) {
    
    error = ENR_getNetReacts(p_handle, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({18806.59,
                                85424.438,
                                115174.05,
                                238972.66});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_TEST(check_cdd(test_vec, ref_vec, 2));
}

BOOST_FIXTURE_TEST_CASE(test_getEnergyUsage, Fixture) {
    
    int linkIdx;

    error = ENR_getEnergyUsage(p_handle, 1, &linkIdx, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({57.712959,
                                75.0,
                                880.41583,
                                96.254318,
                                96.707115,
                                0.0});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_AUTO_TEST_SUITE_END()
