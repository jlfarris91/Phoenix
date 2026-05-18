#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// Phase 0: verifies the test harness compiles and runs.
// Real tests are added starting in Phase 1 (Core framework).
TEST_CASE("Test harness sanity check")
{
    CHECK(1 + 1 == 2);
}
