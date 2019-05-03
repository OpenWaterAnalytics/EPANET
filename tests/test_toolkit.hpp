/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       test_toolkit.hpp
 Description:  Tests EPANET toolkit api functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/21/2019
 ******************************************************************************
*/

#ifndef TEST_TOOLKIT_HPP
#define TEST_TOOLKIT_HPP

#include "epanet2_2.h"


#define DATA_PATH_NET1 "./net1.inp"
#define DATA_PATH_TMP "./tmp.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

struct FixtureOpenClose{
    FixtureOpenClose() {
        error = 0;
        ph = NULL;

        EN_createproject(&ph);
        error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);
    }

    ~FixtureOpenClose() {
      error = EN_close(ph);
      EN_deleteproject(&ph);
  }

  int error;
  EN_Project ph;
};


struct FixtureInitClose {
	FixtureInitClose() {
        error = 0;
        ph = NULL;

		EN_createproject(&ph);
		EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);
	}

	~FixtureInitClose() {
		EN_close(ph);
		EN_deleteproject(&ph);
	}
	int error;
	EN_Project ph;
};


struct FixtureAfterStep{
    FixtureAfterStep() {
        error = 0;
        ph = NULL;

        flag = 0;
        tstop = 10800;

        EN_createproject(&ph);
		error = EN_open(ph, DATA_PATH_NET1, DATA_PATH_RPT, DATA_PATH_OUT);

        error = EN_solveH(ph);
        BOOST_REQUIRE(error == 0);

        error = EN_openQ(ph);
        BOOST_REQUIRE(error == 0);

        error = EN_initQ(ph, flag);
        BOOST_REQUIRE(error == 0);

        do {
            error = EN_runQ(ph, &t);
            BOOST_REQUIRE(error == 0);

            error = EN_stepQ(ph, &tstep);
            BOOST_REQUIRE(error == 0);

        } while (tstep > 0 && t < tstop);
    }

    ~FixtureAfterStep() {
        error = EN_closeQ(ph);
        BOOST_REQUIRE(error == 0);

        error = EN_close(ph);
        EN_deleteproject(&ph);
    }

    int error, flag;
    long t, tstep, tstop;
    EN_Project ph;
};

struct FixtureSingleNode {
    FixtureSingleNode() {
        error = 0;
        ph = NULL;

        EN_createproject(&ph);
        EN_init(ph, DATA_PATH_RPT, DATA_PATH_OUT, EN_GPM, EN_HW);

        EN_addnode(ph, (char *)"CUB_SCOUT_QUONSET_HUT", EN_JUNCTION, &node_qhut);
    }

    ~FixtureSingleNode() {
        EN_close(ph);
        EN_deleteproject(&ph);
    }
    int error, index, node_qhut;
    EN_Project ph;
};


boost::test_tools::predicate_result check_cdd_double(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol);
boost::test_tools::predicate_result check_string(std::string test, std::string ref);


#endif //TEST_TOOLKIT_HPP
