/*
 *   test_epanet_output.cpp
 *
 *   Created: 8/4/2017
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Unit testing for EPANET Output API using google test.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "gtest/gtest.h"

#include "../src/epanet_output.h"

#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/epanet/"
#define DATA_PATH "tools/epanet-output/test/data/net1.out"


namespace {

TEST(ENR_init, InitTest) {
    ENR_Handle p_handle;

    int error = ENR_init(&p_handle);
    ASSERT_EQ(0, error);
    ASSERT_TRUE(p_handle != NULL);
}

TEST(ENR_open, OpenTest) {
    std::string path = std::string(PROJECT_HOME) + std::string(DATA_PATH);
    ENR_Handle p_handle;
    ENR_init(&p_handle);

    int error = ENR_open(p_handle, path.c_str());
    ASSERT_EQ(0, error);
    ENR_close(&p_handle);
}

TEST(ENR_close, CloseTest) {
    ENR_Handle p_handle;
    int error = ENR_init(&p_handle);

    error = ENR_close(&p_handle);
    ASSERT_EQ(-1, error);
    ASSERT_TRUE(p_handle != NULL);
}



class OutputapiTest : public testing::Test {
protected:
    // SetUp for OutputapiTest fixture
    virtual void SetUp() {
        std::string path = std::string(PROJECT_HOME) + std::string(DATA_PATH);

        error = ENR_init(&p_handle);
        ENR_clearError(p_handle);
        error = ENR_open(p_handle, path.c_str());
    }

    // TearDown for OutputapiTest fixture
    virtual void TearDown() {
        ENR_free((void**)&array);
        error = ENR_close(&p_handle);
    }

    int error = 0;
    ENR_Handle p_handle = NULL;

    float* array = NULL;
    int array_dim = 0;
};

TEST_F(OutputapiTest, getNetSizeTest) {
    int* i_array = NULL;
    // nodes, tanks, links, pumps, valves
    int ref_array[5] = {11,2,13,1,0};

    error = ENR_getNetSize(p_handle, &i_array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_EQ(ref_array[i], i_array[i]);

    ENR_free((void**)&i_array);
}

TEST_F(OutputapiTest, getElementName) {
    char* name = new char[MAXID];
    int length, index = 1;

    error = ENR_getElementName(p_handle, ENR_node, index, &name, &length);
    ASSERT_EQ(0, error);

    EXPECT_STREQ("10", name);

    delete(name);
}

TEST_F(OutputapiTest, getNodeAttributeTest) {
    float ref_array[11] = { 1.0,
            0.44407997,
            0.43766347,
            0.42827705,
            0.41342604,
            0.42804748,
            0.44152543,
            0.40502965,
            0.38635802,
            1.0,
            0.96745253 };

    error = ENR_getNodeAttribute(p_handle, 1, ENR_quality, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getLinkAttributeTest) {
    float ref_array[13] = { 1848.5812,
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
            1848.5812};

    error = ENR_getLinkAttribute(p_handle, 1, ENR_flow, &array ,&array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getNodeResultTest) {
    float ref_array[4] = {0.041142918,
            150.0,
            987.98358,
            120.45029};

    error = ENR_getNodeResult(p_handle, 1, 2, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getLinkResultTest) {
    float ref_array[8] = {0.58586824,
            1892.2433,
            0.0,
            -200.71875,
            1.0,
            3.0,
            1.0,
            0.0};

    error = ENR_getLinkResult(p_handle, 24, 13, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getNodeSeriesTest){
    float ref_array[10] = {119.25731,
            120.45029,
            121.19854,
            122.00622,
            122.37414,
            122.8122,
            122.82034,
            122.90379,
            123.40434,
            123.81807};

    error = ENR_getNodeSeries(p_handle, 2, ENR_pressure, 0, 10, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getLinkSeriesTest) {
    float ref_array[10] = {1234.2072,
            1220.4274,
            1164.4,
            1154.8175,
            1100.0635,
            1094.759,
            1041.7854,
            1040.7617,
            1087.556,
            1082.5011};

    error = ENR_getLinkSeries(p_handle, 2, ENR_flow, 0, 10, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getNetReactsTest) {
    float ref_array[4] = {18806.59,
            85424.438,
            115174.05,
            238972.66};

    error = ENR_getNetReacts(p_handle, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(OutputapiTest, getEnergyUsageTest) {
    float ref_array[6] = {57.712959,
            75.0,
            880.41583,
            96.254318,
            96.707115,
            0.0};

    int linkIdx;

    error = ENR_getEnergyUsage(p_handle, 1, &linkIdx, &array, &array_dim);
    ASSERT_EQ(0, error);

    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

}

GTEST_API_ int main(int argc, char **argv) {

    printf("Running main() from gtest_main.cc\n");
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
