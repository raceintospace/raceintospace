// This file generates a main() function for the game_test target
#define BOOST_TEST_MODULE RaceIntoSpaceTest
#include <boost/test/included/unit_test.hpp>

#include "game/options.h"

struct DataDirFixture {
    DataDirFixture() {
        options.dir_gamedata = const_cast<char *>(TEST_DATADIR);
    }
};

BOOST_GLOBAL_FIXTURE(DataDirFixture);
