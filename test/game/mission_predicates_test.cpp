/**
 * Cross-checks every mission predicate against the authoritative JSON
 * data file.  A failure here means a hardcoded list has drifted from
 * the data.
 */
#include <boost/test/unit_test.hpp>

#include <string>

#include "game/mission_util.h"
#include "game/data.h"


BOOST_AUTO_TEST_SUITE(mission_predicates_suite)

// Each predicate should agree with the corresponding mStr field for
// every mission loaded from mission.json.

BOOST_AUTO_TEST_CASE(IsDuration_agrees_with_json)
{
    for (const struct mStr &m : GetMissionData()) {
        bool expected = m.Dur >= 1;
        bool actual   = IsDuration(m.Index);
        BOOST_CHECK_MESSAGE(
            actual == expected,
            "IsDuration(" << m.Index << " \"" << m.Name << "\")"
            " returned " << actual << ", expected " << expected
            << " (Dur=" << static_cast<int>(m.Dur) << ")");
    }
}

BOOST_AUTO_TEST_CASE(IsEVA_agrees_with_json)
{
    for (const struct mStr &m : GetMissionData()) {
        bool expected = m.EVA >= 1;
        bool actual   = IsEVA(m.Index);
        BOOST_CHECK_MESSAGE(
            actual == expected,
            "IsEVA(" << m.Index << " \"" << m.Name << "\")"
            " returned " << actual << ", expected " << expected);
    }
}

BOOST_AUTO_TEST_CASE(IsDocking_agrees_with_json)
{
    for (const struct mStr &m : GetMissionData()) {
        bool expected = m.Doc >= 1;
        bool actual   = IsDocking(m.Index);
        BOOST_CHECK_MESSAGE(
            actual == expected,
            "IsDocking(" << m.Index << " \"" << m.Name << "\")"
            " returned " << actual << ", expected " << expected);
    }
}

BOOST_AUTO_TEST_CASE(IsJoint_agrees_with_json)
{
    for (const struct mStr &m : GetMissionData()) {
        bool expected = m.Jt >= 1;
        bool actual   = IsJoint(m.Index);
        BOOST_CHECK_MESSAGE(
            actual == expected,
            "IsJoint(" << m.Index << " \"" << m.Name << "\")"
            " returned " << actual << ", expected " << expected);
    }
}

BOOST_AUTO_TEST_CASE(IsLM_agrees_with_json)
{
    for (const struct mStr &m : GetMissionData()) {
        bool expected = m.LM >= 1;
        bool actual   = IsLM(m.Index);
        BOOST_CHECK_MESSAGE(
            actual == expected,
            "IsLM(" << m.Index << " \"" << m.Name << "\")"
            " returned " << actual << ", expected " << expected);
    }
}

// GetDurationParens maps integer duration codes to label strings.
BOOST_AUTO_TEST_CASE(GetDurationParens_known_values)
{
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(0)), "");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(1)), "");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(2)), " (B)");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(3)), " (C)");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(4)), " (D)");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(5)), " (E)");
    BOOST_CHECK_EQUAL(std::string(GetDurationParens(6)), " (F)");
}

// MissionTimingOk is always true for ordinary missions and enforces
// planetary alignment windows for Mars/Jupiter/Saturn flybys.

BOOST_AUTO_TEST_CASE(MissionTimingOk_regular_missions_always_ok)
{
    BOOST_CHECK(MissionTimingOk(Mission_Earth_Orbital, 60, 0));
    BOOST_CHECK(MissionTimingOk(Mission_Earth_Orbital, 77, 1));
    BOOST_CHECK(MissionTimingOk(Mission_HistoricalLanding, 68, 0));
}

BOOST_AUTO_TEST_CASE(MissionTimingOk_Mars_windows)
{
    BOOST_CHECK( MissionTimingOk(Mission_MarsFlyby, 60, 0));
    BOOST_CHECK(!MissionTimingOk(Mission_MarsFlyby, 60, 1)); // wrong season
    BOOST_CHECK( MissionTimingOk(Mission_MarsFlyby, 62, 0));
    BOOST_CHECK(!MissionTimingOk(Mission_MarsFlyby, 61, 0)); // wrong year
    BOOST_CHECK( MissionTimingOk(Mission_MarsFlyby, 69, 1));
    BOOST_CHECK(!MissionTimingOk(Mission_MarsFlyby, 69, 0)); // wrong season
}

BOOST_AUTO_TEST_CASE(MissionTimingOk_Jupiter_windows)
{
    BOOST_CHECK( MissionTimingOk(Mission_JupiterFlyby, 60, 0));
    BOOST_CHECK( MissionTimingOk(Mission_JupiterFlyby, 60, 1)); // either season
    BOOST_CHECK(!MissionTimingOk(Mission_JupiterFlyby, 61, 0)); // wrong year
    BOOST_CHECK( MissionTimingOk(Mission_JupiterFlyby, 77, 0));
}

BOOST_AUTO_TEST_SUITE_END()
