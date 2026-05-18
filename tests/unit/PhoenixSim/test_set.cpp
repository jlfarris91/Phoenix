
#include <doctest/doctest.h>

#include "Phoenix.Sim/Containers/FixedSet.h"

using namespace Phoenix;

// TSet uses 0 as the "empty slot" sentinel — keys must be non-zero.
//
// TInlineStorage (std::array) does NOT zero-initialise. Reset() must be
// called after construction before any lookup/insert to clear the slot table.

// Helper: construct a zeroed inline set ready for use.
template <uint32 N>
static TInlineSet<uint32, N> MakeSet()
{
    TInlineSet<uint32, N> s;
    s.Reset();
    return s;
}

TEST_SUITE("TInlineSet")
{
    TEST_CASE("starts empty after Reset")
    {
        auto s = MakeSet<16>();
        CHECK(s.IsEmpty());
        CHECK(s.GetNum() == 0u);
        CHECK(s.GetCapacity() == 16u);
    }

    TEST_CASE("Insert adds elements and Contains finds them")
    {
        auto s = MakeSet<16>();
        CHECK(s.Insert(1u));
        CHECK(s.Insert(2u));
        CHECK(s.Insert(3u));
        CHECK(s.GetNum() == 3u);
        CHECK( s.Contains(1u));
        CHECK( s.Contains(2u));
        CHECK( s.Contains(3u));
        CHECK(!s.Contains(99u));
    }

    TEST_CASE("Insert duplicate returns true and does not grow the set")
    {
        auto s = MakeSet<16>();
        CHECK(s.Insert(5u));
        CHECK(s.Insert(5u));   // duplicate — returns true per implementation
        CHECK(s.GetNum() == 1u);
    }

    TEST_CASE("Contains returns false for elements never inserted")
    {
        auto s = MakeSet<16>();
        s.Insert(1u);
        s.Insert(2u);
        CHECK(!s.Contains(3u));
        CHECK(!s.Contains(100u));
    }

    TEST_CASE("Reset clears all elements")
    {
        auto s = MakeSet<16>();
        s.Insert(1u);
        s.Insert(2u);
        s.Insert(3u);
        s.Reset();
        CHECK(s.IsEmpty());
        CHECK(s.GetNum() == 0u);
        CHECK(!s.Contains(1u));
        CHECK(!s.Contains(2u));
    }

    TEST_CASE("many distinct keys can be inserted and retrieved")
    {
        auto s = MakeSet<64>();
        for (uint32 i = 1; i <= 20; ++i) s.Insert(i);
        CHECK(s.GetNum() == 20u);
        for (uint32 i = 1; i <= 20; ++i) CHECK(s.Contains(i));
        CHECK(!s.Contains(21u));
        CHECK(!s.Contains(50u));
    }
}
