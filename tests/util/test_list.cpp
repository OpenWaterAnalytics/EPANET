/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/list.h
 Description:  Generic list
               https://gist.github.com/pseudomuto/6334796#file-sample_app-c
               Accessed: April 9, 2019
 Authors:      David Muto, Modified by Michael E. Tryby
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/09/2019
 ******************************************************************************
*/

#include <string.h>

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

int *get_int_data(list_node_t *lnode) {
    return (int *)get_data(lnode);
}

bool iterate_int(list_node_t *lnode)
{
    printf("Found value: %d\n", *get_int_data(lnode));
    return true;
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


inline char *get_string_data(list_node_t *lnode)
{
    return *(char **)get_data(lnode);
}

bool iterate_string(list_node_t *lnode)
{
    printf("Found string value: %s\n", get_string_data(lnode));
    return true;
}

void free_string(void *data)
{
    free(*(char **)data);
}

struct FixtureStrings{
    FixtureStrings() {
        list = NULL;

        int numNames = 5;
        const char *names[] = { "David", "Kevin", "Michael", "Craig", "Jimi" };

        list = create_list(sizeof(char *), free_string);

        char *name;
        for (int i = 0; i < numNames; i++) {
            name = strdup(names[i]);
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

    BOOST_CHECK(check_string(get_string_data(head_list(list, false)), "David"));
    BOOST_CHECK(size_list(list) == 5);

    list_node_t *lnode = head_list(list, true);
    BOOST_CHECK(check_string(get_string_data(lnode), "David"));
    delete_node(list, lnode);

    BOOST_CHECK(check_string(get_string_data(head_list(list, false)), "Kevin"));
    BOOST_CHECK(size_list(list) == 4);
}


BOOST_FIXTURE_TEST_CASE(test_tail_list, FixtureStrings) {

    BOOST_CHECK(check_string(get_string_data(tail_list(list)), "Jimi"));
    BOOST_CHECK(size_list(list) == 5);
}


typedef struct test_data_s {
    int    num;
    char   *name;
} test_data_t;

test_data_t *create_test_data(int number, const char *name){

    test_data_t *data = (test_data_t *)malloc(sizeof(test_data_t));
    data->num = number;
    if (name)
        data->name = _strdup(name);
    else
        data->name = NULL;

    return data;
}

void delete_test_data(void *data) {

    test_data_t *test_data = *(test_data_t **)data;

    if (test_data->name)
        free(test_data->name);

	free(test_data);
}

inline test_data_t *get_test_data(list_node_t *lnode)
{
    return *(test_data_t **)get_data(lnode);
}

bool iterate_test_data(list_node_t *lnode)
{
    test_data_t *test_data = get_test_data(lnode);

    printf("Found number: %i name: %s\n", test_data->num, test_data->name);
    return true;
}



BOOST_AUTO_TEST_CASE(test_struct_list){

    list_t *list = NULL;
    list = create_list(sizeof(test_data_t *), delete_test_data);


    test_data_t *data = create_test_data(1, "David");
    append_list(list, &data);

    data = create_test_data(2, "Kevin");
    append_list(list, &data);

    data = create_test_data(3, "Michael");
    append_list(list, &data);


    BOOST_CHECK(size_list(list) == 3);

    for_each_list(list, iterate_test_data);


    list_node_t *lnode;
    // Iterate over list while maintaining containment of list abstraction
    for (lnode = first_list(list); done_list(lnode); lnode = next_list(lnode)) {
        test_data_t *test_data = get_test_data(lnode);

        if (test_data->num == 2)
            printf("Found %s!\n", test_data->name);
    }

    lnode = head_list(list, true);
    delete_node(list, lnode);

    delete_list(list);
}

// TODO: search for an index and return data

BOOST_AUTO_TEST_SUITE_END()
