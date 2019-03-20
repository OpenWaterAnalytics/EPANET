

#define BOOST_TEST_MODULE errormanager
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "util/errormanager.h"


#define MESSAGE_STRING "This is unit testing!"


void mock_lookup(int errcode, char *errmsg, int len)
{
    char *msg = NULL;

    if (errcode == 100) {
        msg = MESSAGE_STRING;
    }
    else {
        msg = "";
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
    error_handle = error_new_manager(&mock_lookup);

    error_dst_manager(error_handle);
}


struct Fixture{
    Fixture() {
		error_message = NULL;
        error_handle = error_new_manager(&mock_lookup);
    }
    ~Fixture() {
        error_dst_manager(error_handle);
        free(error_message);
  }
  int error;
  error_handle_t *error_handle;
  char *error_message;
};


BOOST_FIXTURE_TEST_CASE (test_set_clear, Fixture)
{
    error = error_set(error_handle, 100);
    BOOST_CHECK(error == 100);

    error_clear(error_handle);
    error = error_check(error_handle, &error_message);
    BOOST_CHECK(error == 0);
    BOOST_CHECK(error_message == NULL);
}

BOOST_FIXTURE_TEST_CASE(test_set_check, Fixture)
{
    error = error_set(error_handle, 100);
    BOOST_CHECK(error == 100);

    error = error_check(error_handle, &error_message);
    BOOST_CHECK(check_string(error_message, MESSAGE_STRING));
}

BOOST_AUTO_TEST_SUITE_END()
