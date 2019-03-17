// Test of EPANET's Comment Handling Functions
//
// This is a test of the API functions EN_getcomment and EN_setcomment
//
#define _CRT_SECURE_NO_DEPRECATE

//#define NO_BOOST

#ifndef NO_BOOST
#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>
#endif

#include <iostream>
#include <string>
#include "epanet2_2.h"

#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"
#define DATA_PATH_TMP "./tmp.inp"

#ifdef NO_BOOST
#define BOOST_REQUIRE(x) (((x)) ? cout << "\nPassed at line " << __LINE__ : cout << "\nFailed at line " << __LINE__ )
#endif

using namespace std;

int checkComments(EN_Project ph)
{
    int index;
    char comment[EN_MAXMSG + 1];
    EN_getnodeindex(ph, (char *)"11", &index);
    EN_getcomment(ph, EN_NODE, index, comment);
    if (strcmp(comment, (char *)"J11") != 0) return 0;

    EN_getnodeindex(ph, (char *)"23", &index);
    EN_getcomment(ph, EN_NODE, index, comment);
    if (strcmp(comment, (char *)"Junc23") != 0) return 0;

    EN_getlinkindex(ph, (char *)"11", &index);
    EN_getcomment(ph, EN_LINK, index, comment);
    if (strcmp(comment, (char *)"P11") != 0) return 0;

    EN_getlinkindex(ph, (char *)"9", &index);
    EN_getcomment(ph, EN_LINK, index, comment);
    if (strcmp(comment, (char *)"Pump9") != 0) return 0;

    EN_getpatternindex(ph, (char *)"1", &index);
    EN_getcomment(ph, EN_TIMEPAT, index, comment);
    if (strcmp(comment, (char *)"Time Pattern 1") != 0) return 0;

    EN_getcurveindex(ph, (char *)"1", &index);
    EN_getcomment(ph, EN_CURVE, index, comment);
    if (strcmp(comment, (char *)"Curve 1") != 0) return 0;
    return 1;
}

#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE (test_toolkit)
BOOST_AUTO_TEST_CASE(test_setlinktype)
{
#else
int main(int argc, char *argv[])
{
#endif

    int error = 0;
    int index;
    char comment[EN_MAXMSG+1];

    // Create & load a project
    EN_Project ph = NULL;
    EN_createproject(&ph);
    std::string path_inp = string(DATA_PATH_INP);
    std::string path_rpt = string(DATA_PATH_RPT);
    std::string path_out = string(DATA_PATH_OUT);
    error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Add comments to selected objects
    EN_getnodeindex(ph, (char *)"11", &index);
    EN_setcomment(ph, EN_NODE, index, (char *)"J11");
    EN_getnodeindex(ph, (char *)"23", &index);
    EN_setcomment(ph, EN_NODE, index, (char *)"Junc23");
    EN_getlinkindex(ph, (char *)"11", &index);

    EN_setcomment(ph, EN_LINK, index, (char *)"P11");
    EN_getlinkindex(ph, (char *)"9", &index);
    EN_setcomment(ph, EN_LINK, index, (char *)"Pump9");

    EN_getpatternindex(ph, (char *)"1", &index);
    EN_setcomment(ph, EN_TIMEPAT, index, (char *)"Time Pattern 1");

    EN_getcurveindex(ph, (char *)"1", &index);
    EN_setcomment(ph, EN_CURVE, index, (char *)"Curve 1");

    // Retrieve comments and test their values
    BOOST_REQUIRE(checkComments(ph) == 1);

    // Replace short comment with longer one and vice versa
    EN_getnodeindex(ph, (char *)"11", &index);
    EN_setcomment(ph, EN_NODE, index, (char *)"Junction11");
    EN_getcomment(ph, EN_NODE, index, comment);
    BOOST_REQUIRE(strcmp(comment, (char *)"Junction11") == 0);
    EN_setcomment(ph, EN_NODE, index, (char *)"J11");
    EN_getcomment(ph, EN_NODE, index, comment);
    BOOST_REQUIRE(strcmp(comment, (char *)"J11") == 0);
    
    // Save & re-open project
    string path_tmp = string(DATA_PATH_TMP);
    EN_saveinpfile(ph, path_tmp.c_str());
    EN_close(ph);
    error = EN_open(ph, path_tmp.c_str(), path_rpt.c_str(), "");
    BOOST_REQUIRE(error == 0);

    // Check that comments were saved & read correctly
    BOOST_REQUIRE(checkComments(ph) == 1);
    remove(path_tmp.c_str());

    // Close project
    EN_close(ph);
    EN_deleteproject(&ph);

#ifdef NO_BOOST
    return 0;
#endif
}
#ifndef NO_BOOST
BOOST_AUTO_TEST_SUITE_END()
#endif