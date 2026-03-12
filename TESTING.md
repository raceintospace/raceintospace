# Testing

Race into Space uses [Boost.Test](https://www.boost.org/doc/libs/release/libs/test/doc/html/index.html)
(single-header variant) for unit testing. All test sources live under `test/`.

## Running tests

Build and run:

    cmake --preset macos-vcpkg                             # or linux-vcpkg, windows-vcpkg, linux-release
    cmake --build --preset macos-vcpkg --target game_test
    ctest --test-dir build/debug --output-on-failure

Run the executable directly for more control:

    ./build/debug/game_test                        # all tests, default verbosity
    ./build/debug/game_test --log_level=all        # verbose output
    ./build/debug/game_test -t mission_predicates_suite   # one suite only

The build directory depends on the preset: `build/debug` for vcpkg/debug presets,
`build/release` for release presets.

## Test data

Tests that read game data (e.g. `mission.json`) rely on a global fixture in
`test/test_main.cpp` that sets `options.dir_gamedata` to the source tree's `data/`
directory via the `TEST_DATADIR` compile definition. No manual setup is required.

## Test suites

### mission\_predicates\_suite (`test/game/mission_predicates_test.cpp`)

Cross-checks every mission predicate (`IsDuration`, `IsEVA`, `IsDocking`,
`IsJoint`, `IsLM`) against the authoritative field in `mission.json` for all
62 mission definitions. A failure means a predicate has drifted from the data.
Also tests `GetDurationParens` label mapping and `MissionTimingOk` planetary
alignment windows for Mars and Jupiter flybys.

### mission\_step\_chain\_suite (`test/game/mission_step_chain_test.cpp`)

Tests `MaxFail()` and the `Mev[]` step-chain traversal used during mission
resolution. Includes a regression test for bug #1052, where a cyclic trace
chain caused an infinite loop — the safety valve at count 53 is verified.

### mission\_utils\_suite (`test/game/mission_test.cpp`)

Tests `MissionType` struct equality via `Equals()`, checking every field
for correct comparison behavior.

### downgrade\_suite (`test/game/downgrade_test.cpp`)

Tests `Downgrader` initialization and a basic `next()` call.
Two additional test cases are commented out — they exercised the full downgrade
chain but were disabled because `Downgrader::next()` calls `GetMissionPlan()`,
which requires filesystem access. The global test fixture now provides the data
path, so re-enabling these tests is a viable follow-up.

### roster\_suite (`test/game/roster_test.cpp`)

Placeholder suite for roster module tests.

## CI

Tests run automatically on every push and pull request via `.github/workflows/CI.yaml`
across four matrix entries: `windows-vcpkg`, `linux-vcpkg`, `linux-release`, and
`macos-vcpkg`. Results are published as GitHub check annotations on PRs.

## Gaps

Major areas without test coverage:

- **Hardware programs** — no tests for R&D, purchase, or reliability mechanics
- **Crew assignment & training** — no tests for roster management or skill logic
- **AI opponent** — no tests for computer-player decision making
- **Flight resolution** — the `MaxFail` chain is tested but the full
  `MissionSetup`/`MissionResolve` sequence is not
- **Prestige & budget** — no tests for scoring or funding calculations
- **Save / load** — no round-trip serialization tests
- **UI / rendering** — not applicable to unit tests but worth noting
