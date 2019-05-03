/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_link.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#include <math.h>

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"


BOOST_AUTO_TEST_SUITE (test_link)


BOOST_FIXTURE_TEST_CASE(test_adddelete_link, FixtureInitClose)
{
    int index;

    // Build a network
    EN_addnode(ph, (char *)"N1", EN_JUNCTION, &index);
    EN_addnode(ph, (char *)"N2", EN_JUNCTION, &index);
    EN_addnode(ph, (char *)"N3", EN_RESERVOIR, &index);

    error = EN_addlink(ph, (char *)"L1", EN_PUMP, (char *)"N3", (char *)"N1", &index);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"L2", EN_PIPE, (char *)"N1", (char *)"N3", &index);
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"L2", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_deletelink(ph, index, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"L3", EN_PIPE, (char *)"N1", (char *)"N2", &index);
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"L1", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_deletelink(ph, index, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkindex(ph, (char *)"L3", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_deletelink(ph, index, EN_UNCONDITIONAL);
    BOOST_REQUIRE(error == 0);

}

BOOST_FIXTURE_TEST_CASE(test_link_id_isvalid, FixtureInitClose)
{
    int index;

    // Build a network
    EN_addnode(ph, (char *)"N1", EN_JUNCTION, &index);
    EN_addnode(ph, (char *)"N2", EN_JUNCTION, &index);
    EN_addnode(ph, (char *)"N3", EN_RESERVOIR, &index);

    error = EN_addlink(ph, (char *)"L1", EN_PUMP, (char *)"N1", (char *)"N2", &index);
    BOOST_REQUIRE(error == 0);

    error = EN_addlink(ph, (char *)"L 2", EN_PIPE, (char *)"N1", (char *)"N2", &index);
    BOOST_REQUIRE(error == 252);

    error = EN_addlink(ph, (char *)"\"L2", EN_PIPE, (char *)"N1", (char *)"N2", &index);
    BOOST_REQUIRE(error == 252);

    error = EN_addlink(ph, (char *)"L;2", EN_PIPE, (char *)"N1", (char *)"N2", &index);
    BOOST_REQUIRE(error == 252);

    EN_getlinkindex(ph, (char *)"L1", &index);
    error = EN_setlinkid(ph, index, (char *)"L;1");
    BOOST_REQUIRE(error == 252);
}

BOOST_AUTO_TEST_CASE(test_setlinktype)
{
    int error = 0;
    int p113, n31, p121, n113_1, n113_2;
    double q113 = 0.0, p31 = 0.0, diam;

    EN_Project ph = NULL;
    EN_createproject(&ph);

    error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Change duration to 0
    error = EN_settimeparam(ph, EN_DURATION, 0);
    BOOST_REQUIRE(error == 0);

    // Get indexes of pipe 113 and node 31
    error = EN_getlinkindex(ph, (char *)"113", &p113);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodeindex(ph, (char *)"31", &n31);
    BOOST_REQUIRE(error == 0);

    // Reverse pipe 113 and give it a check valve
    error = EN_getlinknodes(ph, p113, &n113_1, &n113_2);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinknodes(ph, p113, n113_2, n113_1);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinktype(ph, &p113, EN_CVPIPE, 0);
    BOOST_REQUIRE(error == 0);

    // Get index & diameter of pipe 121 connected to node 31
    error = EN_getlinkindex(ph, (char *)"121", &p121);
    BOOST_REQUIRE(error == 0);
    error = EN_getlinkvalue(ph, p121, EN_DIAMETER, &diam);
    BOOST_REQUIRE(error == 0);

    // Replace it with a PRV
    error = EN_setlinktype(ph, &p121, EN_PRV, 0);
    BOOST_REQUIRE(error == 0);

    // Set diameter & setting of new PRV
    error = EN_setlinkvalue(ph, p121, EN_INITSETTING, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_setlinkvalue(ph, p121, EN_DIAMETER, diam);
    BOOST_REQUIRE(error == 0);

    // Solve for hydraulics
    error = EN_solveH(ph);
    BOOST_REQUIRE(error == 0);

    // Get flow in link 113 and pressure at node 31
    error = EN_getlinkvalue(ph, p113, EN_FLOW, &q113);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, n31, EN_PRESSURE, &p31);
    BOOST_REQUIRE(error == 0);

    // Require that link 113 flow be 0
    q113 = fabs(q113);
    BOOST_REQUIRE(q113 < 0.001);

    // Require that node 31 pressure be 100
    p31 = fabs(p31 - 100.0f);
    BOOST_REQUIRE(p31 < 0.001);

    // Close and delete project
    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}


BOOST_AUTO_TEST_CASE(test_link_setid_save)
{
	int error = 0;

	EN_Project ph = NULL;
	EN_createproject(&ph);

	error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, "");
	BOOST_REQUIRE(error == 0);

	// Test of illegal link name change
	char newid_3[] = "Illegal; link name";
	error = EN_setlinkid(ph, 3, newid_3);
	BOOST_REQUIRE(error > 0);

	// Test of legal link name change
	char newid_4[] = "Link3";
	error = EN_setlinkid(ph, 3, newid_4);
	BOOST_REQUIRE(error == 0);

	// Save the project
	error = EN_saveinpfile(ph, "net1_setid.inp");
	BOOST_REQUIRE(error == 0);

	error = EN_close(ph);
	BOOST_REQUIRE(error == 0);
	EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_CASE(test_link_setid_reopen, * boost::unit_test::depends_on("test_link/test_link_setid_save"))
{
	int error = 0;
	int index;

	EN_Project ph = NULL;

    // Re-open the saved project
    EN_createproject(&ph);
    error = EN_open(ph, "net1_setid.inp", DATA_PATH_RPT, "");
    BOOST_REQUIRE(error == 0);

    // Check that 3rd link has its new name
    error = EN_getlinkindex(ph, (char *)"Link3", &index);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(index == 3);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph);
}

BOOST_FIXTURE_TEST_CASE(test_link_comments, FixtureOpenClose)
{
    int index;
    char comment[EN_MAXMSG + 1];

    // Set link comments
    error = EN_getlinkindex(ph, (char *)"11", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_setcomment(ph, EN_LINK, index, (char *)"P11");
    BOOST_REQUIRE(error == 0);

    error = EN_getlinkindex(ph, (char *)"9", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_setcomment(ph, EN_LINK, index, (char *)"Pump9");
    BOOST_REQUIRE(error == 0);

    // Check link comments
    error = EN_getlinkindex(ph, (char *)"11", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getcomment(ph, EN_LINK, index, comment);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(comment, (char *)"P11"));

    error = EN_getlinkindex(ph, (char *)"9", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_getcomment(ph, EN_LINK, index, comment);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(check_string(comment, (char *)"Pump9"));
}

BOOST_AUTO_TEST_SUITE_END()
