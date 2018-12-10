



#define BOOST_TEST_MODULE "toolkit"
#include <boost/test/included/unit_test.hpp>

#include <string>
#include "epanet_py.h"

// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"

using namespace std;


BOOST_AUTO_TEST_SUITE (test_toolkit)

BOOST_AUTO_TEST_CASE (test_alloc_free)
{
    int error = 0;
    Handle ph = NULL;

    error = create_project(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph != NULL);

    error = delete_project(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph == NULL);
}

BOOST_AUTO_TEST_CASE(test_epanet)
{
	string path_inp(DATA_PATH_INP);
	string path_rpt(DATA_PATH_RPT);
	string path_out(DATA_PATH_OUT);

	char *msg = nullptr;

	Handle ph = NULL;

	create_project(&ph);
	clear_error(ph);
	
	int error = run_project(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
	BOOST_CHECK(error == 0);

	check_error(ph, &msg);
	toolkit_free((void **)&msg);

	delete_project(&ph);
}

BOOST_AUTO_TEST_SUITE_END()
