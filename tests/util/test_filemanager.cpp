/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/test_filemanager.cpp
 Description:  Tests for util/filemanager.c
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/01/2019
 ******************************************************************************
*/

#define BOOST_TEST_MODULE filemanager

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "util/filemanager.h"


#define DATA_PATH_OUTPUT "./example1.out"


boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}


BOOST_AUTO_TEST_SUITE(test_filemanager)


BOOST_AUTO_TEST_CASE (test_create_destroy)
{
    file_handle_t *file_handle = NULL;

    file_handle = create_file_manager();
    BOOST_CHECK(file_handle != NULL);
	BOOST_CHECK(is_valid(file_handle) == true);

    delete_file_manager(file_handle);
}

BOOST_AUTO_TEST_CASE(test_open_close)
{
    int error = 0;
    file_handle_t *file_handle = NULL;

    file_handle = create_file_manager();
    BOOST_CHECK(file_handle != NULL);

    error = open_file(file_handle, DATA_PATH_OUTPUT, "rb");
    BOOST_REQUIRE(error == 0);
	BOOST_CHECK(is_valid(file_handle) == true);

    error = close_file(file_handle);
    BOOST_REQUIRE(error == 0);

    delete_file_manager(file_handle);
}

struct Fixture{
    Fixture() {
		error = 0;
        file_handle = NULL;

		file_handle = create_file_manager();
        open_file(file_handle, NULL, "wt");
    }
    ~Fixture() {
        close_file(file_handle);
        delete_file_manager(file_handle);
  }
  int error;
  file_handle_t *file_handle;
};

BOOST_FIXTURE_TEST_CASE(test_temp_file, Fixture)
{
    char *filename;

    printf_file(file_handle, "%s", "This is a test.");

    error = get_filename(file_handle, &filename);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(is_valid(file_handle) == true);

    BOOST_CHECK(boost::filesystem::exists(filename) == true);

    free(filename);
}

BOOST_AUTO_TEST_SUITE_END()
