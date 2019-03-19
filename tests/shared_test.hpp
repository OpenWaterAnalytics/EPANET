


#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <stdlib.h>
#endif


#include <math.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "epanet2_2.h"



// Custom test to check the minimum number of correct decimal digits between
// the test and the ref vectors.
boost::test_tools::predicate_result check_cdd_double(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol){
    double tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<double>::iterator test_it;
    std::vector<double>::iterator ref_it;

    for (test_it = test.begin(), ref_it = ref.begin();
        (test_it < test.end()) && (ref_it < ref.end());
        ++test_it, ++ref_it)
    {
        if (*test_it != *ref_it) {
            // Compute log absolute error
            tmp = abs(*test_it - *ref_it);
            if (tmp < 1.0e-7)
                tmp = 1.0e-7;

            else if (tmp > 2.0)
                tmp = 1.0;

            tmp = -log10(tmp);
            if (tmp < 0.0)
                tmp = 0.0;

            if (tmp < min_cdd)
                min_cdd = tmp;
        }
    }

    return floor(min_cdd) >= cdd_tol;
}

boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}


#define DATA_PATH_NET1 "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

struct FixtureOpenClose{
    FixtureOpenClose() {
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


struct FixtureAfterStep{
    FixtureAfterStep() {
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
