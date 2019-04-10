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


boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}


bool iterate_int(void *data)
{
    printf("Found value: %d\n", *(int *)data);
    return true;
}

bool iterate_string(void *data)
{
    char *string = *(char **)data;
    printf("Found string value: %s\n", string);
    return true;
}

void free_string(void *data)
{
    free(*(char **)data);
}

BOOST_AUTO_TEST_SUITE(test_list)


BOOST_AUTO_TEST_CASE(test_create_delete) {

    list_t *list;
    list = create_list(sizeof(int), NULL);

    delete_list(list);
}


BOOST_AUTO_TEST_CASE(test_int_list){

    int i, numbers = 10;
    list_t *list = NULL;

    list = create_list(sizeof(int), NULL);

    for(i = 1; i <= numbers; i++) {
        append_list(list, &i);
    }
    BOOST_CHECK(size_list(list) == 10);

    for_each_list(list, iterate_int);

    delete_list(list);
}

struct FixtureStrings{
    FixtureStrings() {
		list = NULL;

        int numNames = 5;
        const char *names[] = { "David", "Kevin", "Michael", "Craig", "Jimi" };

		list = create_list(sizeof(char *), free_string);
		
		char *name;
    	for (int i = 0; i < numNames; i++) {
    		name = _strdup(names[i]);
    		append_list(list, &name);
    	}
    }
    ~FixtureStrings() {
        delete_list(list);
  }
  list_t *list;
};

BOOST_FIXTURE_TEST_CASE(test_string_list, FixtureStrings) {

    BOOST_CHECK(size_list(list) == 5);

    for_each_list(list, iterate_string);
}


BOOST_FIXTURE_TEST_CASE(test_head_list, FixtureStrings) {

    void *temp = head_list(list, true);

    BOOST_CHECK(check_string(*(char **)temp, "David"));
    BOOST_CHECK(size_list(list) == 4);

	// To free a node, free both the data and reference to data 
	free_string(temp);
    free(temp);
}


BOOST_FIXTURE_TEST_CASE(test_tail_list, FixtureStrings) {

    void *temp = tail_list(list);

    BOOST_CHECK(check_string(*(char **)temp, "Jimi"));
    BOOST_CHECK(size_list(list) == 5);

    free(temp);
}


BOOST_AUTO_TEST_SUITE_END()
