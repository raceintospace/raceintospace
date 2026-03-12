/**
 * Regression tests for MaxFail() and the Mev[] step-chain iteration
 * idiom used in the mission resolution code.
 *
 * Bug #1052: MaxFail() could loop infinitely if the trace chain formed
 *   a cycle.  The fix added a guard at count==53 that returns 1 early.
 */
#include <boost/test/unit_test.hpp>

#include <cstring>

#include "mc.h"
#include "prest.h"


BOOST_AUTO_TEST_SUITE(mission_step_chain_suite)

// Zero the global Mev array before and after each test to prevent
// state from leaking between cases.
struct MevFixture {
    MevFixture()  { std::memset(Mev, 0, sizeof(Mev)); }
    ~MevFixture() { std::memset(Mev, 0, sizeof(Mev)); }
};

// A single-step chain terminated by trace=0x7f returns that step's StepInfo.
BOOST_FIXTURE_TEST_CASE(MaxFail_single_step, MevFixture)
{
    Mev[0].StepInfo = 500;
    Mev[0].trace    = 0x7f;

    BOOST_CHECK_EQUAL(MaxFail(), 500);
}

// StepInfo==0 is treated as a missing value and replaced with 1003.
BOOST_FIXTURE_TEST_CASE(MaxFail_zero_StepInfo_becomes_1003, MevFixture)
{
    Mev[0].StepInfo = 0;
    Mev[0].trace    = 0x7f;

    BOOST_CHECK_EQUAL(MaxFail(), 1003);
}

// MaxFail returns the maximum StepInfo across all steps in the chain.
BOOST_FIXTURE_TEST_CASE(MaxFail_returns_max_over_chain, MevFixture)
{
    Mev[0].StepInfo = 100; Mev[0].trace = 1;
    Mev[1].StepInfo = 500; Mev[1].trace = 2;
    Mev[2].StepInfo = 200; Mev[2].trace = 0x7f;

    BOOST_CHECK_EQUAL(MaxFail(), 500);
}

// Safety valve: a cyclic trace chain must not loop forever.  The
// count==53 guard must fire and return 1 before the cycle repeats.
// Regression for bug #1052.
BOOST_FIXTURE_TEST_CASE(MaxFail_safety_valve_at_count_53, MevFixture)
{
    // Build a 54-step cycle: Mev[0] → [1] → … → [53] → [0].
    // Every step carries a high StepInfo to prove the valve fires before
    // the final tally; without the guard this test would loop forever.
    for (int i = 0; i < 54; ++i) {
        Mev[i].StepInfo = 2000;
        Mev[i].trace    = static_cast<int8_t>((i + 1) % 54);
    }

    // The safety valve fires at count==53 and returns 1, not 2000.
    BOOST_CHECK_EQUAL(MaxFail(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
