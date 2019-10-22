#include <boost/test/unit_test.hpp>

#include <cstring>

#include "game/mission_util.h"
#include "game/data.h"


BOOST_AUTO_TEST_SUITE(mission_utils_suite)

BOOST_AUTO_TEST_CASE(missiontype_equals_test)
{
    MissionType m1 = {
        "APOLLO XI",
        Mission_HistoricalLanding,
        29, 1, "ABCDE", // Didn't code in the actual hardware. Sorry.
        1, 0, 7, 4, 3, MANNED_HW_THREE_MAN_CAPSULE, 1, 2, 1
    };

    BOOST_CHECK( Equals(m1, m1) );

    MissionType m2 = {
        "APOLLO XI",
        Mission_HistoricalLanding,
        29, 1, "ABCDE", // Didn't code in the actual hardware. Sorry.
        1, 0, 7, 4, 3, MANNED_HW_THREE_MAN_CAPSULE, 1, 2, 1
    };

    BOOST_CHECK( Equals(m1, m2) );

    MissionType m3 = {
        "GEMINI VIII",
        21, // JT ORBITAL-DE
        17, 1, "AACDE", // Didn't code in the actual hardware. Sorry.
        1, 0, 3, 1, 2, MANNED_HW_THREE_MAN_CAPSULE, 1, 2, 1
    };

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;

    BOOST_CHECK( Equals(m1, m3) );

    m3 = m2;
    strcpy(m3.Name, "APOLLO XII");

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.MissionCode = Mission_None;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Patch = 10;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.part = 0;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    strcpy(m3.Hard, "IJKLM");

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Joint = 0;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Rushing = 1;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Month = 8;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Duration = 5;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Men = 4;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Prog = MANNED_HW_FOUR_MAN_CAPSULE;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.PCrew = 6;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.BCrew = 7;

    BOOST_CHECK( Equals(m1, m3) == false );

    m3 = m2;
    m3.Crew = 4;

    BOOST_CHECK( Equals(m1, m3) == false );
}

BOOST_AUTO_TEST_SUITE_END()
