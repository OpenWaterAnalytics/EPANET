

#define BOOST_TEST_MODULE errormanager

#include <boost/test/unit_test.hpp>

#include "util/errormanager.h"


#define MESSAGE_STRING "This is unit testing!"


void mock_lookup(int errcode, char *errmsg, int len)
{
    char *msg = NULL;

    if (errcode == 100) {
        msg = (char *)MESSAGE_STRING;
    }
    else {
        msg = (char *)"";
    }
    strncpy(errmsg, msg, len);
}

boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}


BOOST_AUTO_TEST_SUITE(test_errormanager)

BOOST_AUTO_TEST_CASE (test_create_destroy)
{
    error_handle_t *error_handle = NULL;
    error_handle = create_error_manager(&mock_lookup);

    delete_error_manager(error_handle);
}


struct Fixture{
    Fixture() {
        error_message = NULL;
        error_handle = create_error_manager(&mock_lookup);
    }
    ~Fixture() {
        delete_error_manager(error_handle);
        free(error_message);
    }
    int error;
    error_handle_t *error_handle;
    char *error_message;
};


BOOST_FIXTURE_TEST_CASE (test_set_clear, Fixture)
{
    error = set_error(error_handle, 100);
    BOOST_CHECK(error == 100);

    clear_error(error_handle);
    error = check_error(error_handle, &error_message);
    BOOST_CHECK(error == 0);
    BOOST_CHECK(error_message == NULL);
}

BOOST_FIXTURE_TEST_CASE(test_set_check, Fixture)
{
    error = set_error(error_handle, 100);
    BOOST_CHECK(error == 100);

    error = check_error(error_handle, &error_message);
    BOOST_CHECK(check_string(error_message, MESSAGE_STRING));
}

BOOST_AUTO_TEST_SUITE_END()
