#include <boost/test/unit_test.hpp>

#include <cstring>

#include "game/downgrader.h"
#include "game/mission_util.h"

struct DowngradeFixture {
    DowngradeFixture() 
    {
        dgs.add(Mission_HistoricalLanding, Mission_LunarOrbital_LM_Test);
        dgs.add(Mission_HistoricalLanding, Mission_Lunar_Orbital);
        dgs.add(Mission_HistoricalLanding, Mission_LunarOrbital);
        dgs.add(Mission_HistoricalLanding, Mission_LunarPass);

        memset(&mission, 0, sizeof(mission));
        strcpy(mission.Name, "APOLLO XI");
        mission.MissionCode = Mission_HistoricalLanding;
        mission.Patch = 29;
        mission.part = 1;
        strcpy(mission.Hard, "ABCDE");
        mission.Joint = 1;
        mission.Rushing = 0;
        mission.Month = 7;
        mission.Duration = 4;
        mission.Men = 3;
        mission.Prog = MANNED_HW_THREE_MAN_CAPSULE;
        mission.PCrew = 1;
        mission.BCrew = 2;
        mission.Crew = 1;
    }
    ~DowngradeFixture()
    {
    }

    Downgrader::Options dgs;
    MissionType mission;
};


BOOST_FIXTURE_TEST_SUITE(downgrade_suite, DowngradeFixture)

BOOST_AUTO_TEST_CASE(DowngradeInitTest)
{
    BOOST_REQUIRE_NO_THROW( Downgrader(mission, dgs) );
}


BOOST_AUTO_TEST_CASE(DowngradeNextThrowTest)
{
    Downgrader tester(mission, dgs);

    BOOST_REQUIRE_NO_THROW( tester.next() );
}


// This isn't working. I believe the problem is rooted in
// Downgrader::next() relying on GetMissionType(), which uses the
// filesystem. Trying to read files through the RIS code seems to
// always cause problems.
// BOOST_AUTO_TEST_CASE(DowngradeNextTest)
// {
//     Downgrader tester(mission, dgs);
//
//     MissionType nextMission = mission;
//     nextMission.MissionCode = Mission_LunarOrbital_LM_Test;
//
//     BOOST_CHECK( Equals(tester.next(), nextMission) );
//
//     nextMission.MissionCode = Mission_Lunar_Orbital;
//
//     BOOST_CHECK( Equals(tester.next(), nextMission) );
//
//     nextMission.MissionCode = Mission_LunarPass;
//     nextMission.Duration = 3;
//
//     BOOST_CHECK( Equals(tester.next(), nextMission) );
//
//     memset(&nextMission, 0, sizeof(nextMission));
//     nextMission.MissionCode = Mission_None;
//
//     BOOST_CHECK( Equals(tester.next(), nextMission) );
//
//     BOOST_CHECK( Equals(tester.next(), mission) );
// }
//
//
// BOOST_AUTO_TEST_CASE(DowngradeCurrentTest)
// {
//     Downgrader tester(mission, dgs);
//
//     BOOST_REQUIRE( Equals(tester.current(), mission) );
//
//     MissionType nextMission = mission;
//     nextMission.MissionCode = Mission_LunarOrbital_LM_Test;
//     tester.next();
//
//     BOOST_CHECK( Equals(tester.current(), nextMission) );
//
//     nextMission.MissionCode = Mission_Lunar_Orbital;
// }

BOOST_AUTO_TEST_SUITE_END()
