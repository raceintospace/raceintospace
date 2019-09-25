#include <boost/test/unit_test.hpp>

#include "game/roster.h"
#include "game/roster_group.h"
#include "game/roster_entry.h"


BOOST_AUTO_TEST_SUITE(roster_suite)

BOOST_AUTO_TEST_CASE(roster_test)
{
    BOOST_CHECK(2 + 2 == 4);
}

// BOOST_AUTO_TEST_CASE(TestLoadRoster)
// {
//     // Roster roster = Roster::load("test_roster.json");
//     BOOST_CHECK( true );
//     // BOOST_CHECK(roster.getGroup(0, 1));
// }

BOOST_AUTO_TEST_SUITE_END()
