/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_units.cpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/30/2023
 ******************************************************************************
*/

/*
This is a test for the API functions that change the units of a project.
*/

#include <boost/test/unit_test.hpp>

#include "test_toolkit.hpp"

/*
----------------------------------------------
   Flow units conversion factors
----------------------------------------------
*/
double  GPMperCFS =  448.831;
double  LPSperCFS =  28.317;
double  MperFT    =  0.3048;
double  PSIperFT  =  0.4333;
double  KPAperPSI =  6.895;

char unitrules[] = "RULE 1\n IF NODE 10 DEMAND > 10 \n"
            "AND NODE 10 HEAD > 20 \n"
            "AND NODE 10 PRESSURE > 30 \n"
            "AND NODE 10 LEVEL > 40 \n"
            "AND LINK 10 FLOW > 50 \n"
            "AND LINK PRV1 SETTING > 60 \n"
            "AND LINK FCV1 SETTING > 70 \n"
            "THEN LINK PRV1 SETTING = 80\n ELSE LINK FCV1 SETTING = 90";


BOOST_AUTO_TEST_SUITE (test_units)

BOOST_FIXTURE_TEST_CASE(test_pressure_units, FixtureInitClose)
{
    int index;
    long t;
    double p, units;

    // Create basic network
    error = EN_addnode(ph, "R1", EN_RESERVOIR, &index);
    BOOST_REQUIRE(error == 0);
    error = EN_setnodevalue(ph, index, EN_ELEVATION, 100);
    BOOST_REQUIRE(error == 0);
    error = EN_addnode(ph, "J1", EN_JUNCTION, &index);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, "P1", EN_PIPE, "R1", "J1", &index);
    BOOST_REQUIRE(error == 0);

    // Run simulation and get junction pressure
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &t);
    BOOST_REQUIRE(error == 0);
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 43.33) < 1.e-5);

    // Get pressure unit and check that it is PSI
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_PSI);

    // Change to pressure from PSI to meters and check it's still PSI
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_METERS);
    BOOST_REQUIRE(error == 0);

    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_PSI);

    // Change flow units to LPS to change to metric units and rerun simulation
    error = EN_setflowunits(ph, EN_LPS);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, EN_NOSAVE);
    BOOST_REQUIRE(error == 0);
    error = EN_runH(ph, &t);
    BOOST_REQUIRE(error == 0);

    // Confirm that pressure is now in meters
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 30.48) < 1.e-5);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_METERS);

    // Set and check that pressure units are in kPa
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_KPA);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_KPA);
    error = EN_getnodevalue(ph, 1, EN_PRESSURE, &p);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(p - 298.76035) < 1.e-5);

    // Set pressure to PSI and check that it remains in kPa
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_PSI);
    BOOST_REQUIRE(error == 0);
    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_KPA);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);
 
}

BOOST_FIXTURE_TEST_CASE(test_pda_unit_change,  FixtureOpenClose)
{
    int type;
    double pmin, preq, pexp;

    // Switch to PDA with pressure limits of 20 - 100 psi
    error = EN_setdemandmodel(ph, EN_PDA, 20, 100, 0.5);
    BOOST_REQUIRE(error == 0);

    error = EN_setflowunits(ph, EN_LPS);
    BOOST_REQUIRE(error == 0);

    error = EN_getdemandmodel(ph, &type, &pmin, &preq, &pexp);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(pmin - (20/PSIperFT*MperFT)) < 1.e-5); 
    BOOST_CHECK(abs(preq - (100/PSIperFT*MperFT)) < 1.e-5); 

}

BOOST_FIXTURE_TEST_CASE(test_rule_unit_change,  FixtureOpenClose)
{
    int index, node22, link12;
    double units;

    // Rule variables
    int r_logop, r_object, r_objIndex, r_variable, r_relop, r_status;
    double r_value;
    
    // Control variables
    int c_index, c_type, c_linkIndex, c_nodeIndex;
    double c_setting, c_level;

    // Add new PRV and FCV to test rules
    error = EN_addlink(ph, (char *)"PRV1", EN_PRV, (char *)"10", (char *)"11", &index);
    BOOST_REQUIRE(error == 0);
    error = EN_addlink(ph, (char *)"FCV1", EN_FCV, (char *)"12", (char *)"13", &index);
    BOOST_REQUIRE(error == 0);

    // Add the rule to the project
    error = EN_addrule(ph, unitrules);
    BOOST_REQUIRE(error == 0);

    // Add control that checks junction pressure
    EN_getnodeindex(ph, (char *)"22", &node22);
    EN_getlinkindex(ph, (char *)"12", &link12);
    error = EN_addcontrol(ph, EN_HILEVEL, link12, 0, node22, 250, &c_index);
    BOOST_REQUIRE(error == 0);

    // Check that rules and controls are in US units
    error = EN_getpremise(ph, 1, 3, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(r_value == 30);

    error = EN_getcontrol(ph, c_index, &c_type, &c_linkIndex, &c_setting, &c_nodeIndex, &c_level);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(c_level == 250);

    // Change flow units to lps and pressure to meters
    error = EN_setflowunits(ph, EN_LPS);
    BOOST_REQUIRE(error == 0);

    error = EN_getoption(ph, EN_PRESS_UNITS, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(units == EN_METERS);

    // Check that rules and controls are in meters

    // Simple Control - 250 psi to meters
    error = EN_getcontrol(ph, c_index, &c_type, &c_linkIndex, &c_setting, &c_nodeIndex, &c_level);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(c_level - (250/PSIperFT*MperFT)) < 1.e-5); // 250 PSI to M

    // Premise 1 - Demand GPM to LPS
    error = EN_getpremise(ph, 1, 1, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (10/GPMperCFS*LPSperCFS)) < 1.e-5); //10 GPM to LPS

    // Premise 2 - Head FT to Meters
    error = EN_getpremise(ph, 1, 2, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (20*MperFT)) < 1.e-5); //20 FT to M

    // Premise 3 - Pressure PSI to Meters
    error = EN_getpremise(ph, 1, 3, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (30/PSIperFT*MperFT)) < 1.e-5); //30 PSI to M

    // Premise 4 - Level FT to Meters
    error = EN_getpremise(ph, 1, 4, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (40*MperFT)) < 1.e-5); //40 FT to M

    // Premise 5 - Flow GPM to LPS
    error = EN_getpremise(ph, 1, 5, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (50/GPMperCFS*LPSperCFS)) < 1.e-5); //50 GPM to LPS

    // Premise 6 - Setting PSI to Meters
    error = EN_getpremise(ph, 1, 6, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (60/PSIperFT*MperFT)) < 1.e-5); //60 PSI to M

    // Premise 7 - Setting GPM to LPS
    error = EN_getpremise(ph, 1, 7, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (70/GPMperCFS*LPSperCFS)) < 1.e-5); //70 GPM to LPS

    // ThenAction - Setting PSI to Meters
    error = EN_getthenaction(ph, 1, 1, &r_objIndex, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (80/PSIperFT*MperFT)) < 1.e-5); //80 PSI to M

    // ElseAction - Setting GPM to LPS
    error = EN_getelseaction(ph, 1, 1, &r_objIndex, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (90/GPMperCFS*LPSperCFS)) < 1.e-5); //90  GPM to LPS

    // Change pressure units to kPa
    error = EN_setoption(ph, EN_PRESS_UNITS, EN_KPA);
    BOOST_REQUIRE(error == 0);

    // Simple Control - 250 psi to kPa
    error = EN_getcontrol(ph, c_index, &c_type, &c_linkIndex, &c_setting, &c_nodeIndex, &c_level);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(c_level - (250*KPAperPSI)) < 1.e-5); //250 PSI to kPa

    // Premise 3 - Pressure PSI to kPa
    error = EN_getpremise(ph, 1, 3, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (30*KPAperPSI)) < 1.e-5); //30 PSI to kPa

    // Premise 6 - Setting PSI to kPa
    error = EN_getpremise(ph, 1, 6, &r_logop, &r_object, &r_objIndex, &r_variable, &r_relop, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (60*KPAperPSI)) < 1.e-5); //60 PSI to kPa

    // ThenAction - Setting PSI to kPa
    error = EN_getthenaction(ph, 1, 1, &r_objIndex, &r_status, &r_value);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(abs(r_value - (80*KPAperPSI)) < 1.e-5); //80 PSI to kPa


}

BOOST_AUTO_TEST_SUITE_END()
