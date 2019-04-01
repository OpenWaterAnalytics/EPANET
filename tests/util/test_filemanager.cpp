/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/test_filemanager.cpp
 Description:  Tests filemanager
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/01/2019
 ******************************************************************************
*/

#define BOOST_TEST_MODULE filemanager

#include <boost/test/unit_test.hpp>

#include "util/filemanager.h"


#define DATA_PATH_OUTPUT "./example1.out"


BOOST_AUTO_TEST_SUITE(test_filemanager)


BOOST_AUTO_TEST_CASE (test_create_destroy)
{
    file_handle_t *file_handle = NULL;

    file_handle = create_file_manager();
    BOOST_CHECK(file_handle != NULL);

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

    error = close_file(file_handle);
    BOOST_REQUIRE(error == 0);

    delete_file_manager(file_handle);
}

struct Fixture{
    Fixture() {
		error = 0;
        file_handle = NULL;

		file_handle = create_file_manager();
        open_file(file_handle, DATA_PATH_OUTPUT, "rb");
    }
    ~Fixture() {
        close_file(file_handle);
        delete_file_manager(file_handle);
  }
  int error;
  file_handle_t *file_handle;
};


BOOST_AUTO_TEST_SUITE_END()
