/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/test_list.cpp
 Description:  Tests for util/list.c
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/09/2019
 ******************************************************************************
*/

#define BOOST_TEST_MODULE list

#include <boost/test/unit_test.hpp>

#include "util/list.h"


bool iterate_int(void *data)
{
	printf("Found value: %d\n", *(int *)data);
	return true;
}


BOOST_AUTO_TEST_SUITE(test_list)


BOOST_AUTO_TEST_CASE(test_create_delete) {

	list_t *list;
    list = create_list(sizeof(int), NULL);

    delete_list(list);
}


struct Fixture{
    Fixture() {
        list = NULL;

		list = create_list(sizeof(int), NULL);
    }
    ~Fixture() {
        delete_list(list);
  }
  list_t *list;
};


BOOST_FIXTURE_TEST_CASE(test_list_append, Fixture){

int i, numbers = 10;

    for(i = 1; i <= numbers; i++) {
        append_list(list, &i);
    }
	BOOST_CHECK(size_list(list) == 10);
}

BOOST_FIXTURE_TEST_CASE(test_list_foreach, Fixture) {

	int i, numbers = 10;

	for (i = 1; i <= numbers; i++) {
		append_list(list, &i);
	}

	for_each_list(list, iterate_int);
}

BOOST_AUTO_TEST_SUITE_END()
