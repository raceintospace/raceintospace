#define BOOST_TEST_MODULE Dummy
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(dummy_test)
{
    BOOST_CHECK(2 + 2 == 4);
}