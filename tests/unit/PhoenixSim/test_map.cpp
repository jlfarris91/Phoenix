
#include <doctest/doctest.h>

#include "PhoenixSim/Containers/FixedMap.h"

using namespace Phoenix;

// TMap uses TKey{} (zero-initialised) as the "empty slot" sentinel.
// All test keys must therefore be non-zero.
//
// TInlineStorage (std::array) does NOT zero-initialise. Reset() must be
// called after construction before any lookup/insert to clear the slot table.

// Helper: construct a zeroed inline map ready for use.
static TInlineMap<int, int, 16> MakeMap()
{
    TInlineMap<int, int, 16> m;
    m.Reset();
    return m;
}

TEST_SUITE("TInlineMap")
{
    TEST_CASE("starts empty after Reset")
    {
        auto m = MakeMap();
        CHECK(m.IsEmpty());
        CHECK(m.GetNum() == 0u);
        CHECK(m.GetCapacity() == 16u);
    }

    TEST_CASE("Insert adds entries and Contains finds them")
    {
        auto m = MakeMap();
        m.Insert(1, 100);
        m.Insert(2, 200);
        m.Insert(3, 300);
        CHECK(m.GetNum() == 3u);
        CHECK( m.Contains(1));
        CHECK( m.Contains(2));
        CHECK( m.Contains(3));
        CHECK(!m.Contains(4));
    }

    TEST_CASE("GetPtr returns pointer to value for present key")
    {
        auto m = MakeMap();
        m.Insert(5, 500);
        int* ptr = m.GetPtr(5);
        REQUIRE(ptr != nullptr);
        CHECK(*ptr == 500);
    }

    TEST_CASE("GetPtr returns nullptr for absent key")
    {
        auto m = MakeMap();
        m.Insert(1, 10);
        CHECK(m.GetPtr(99) == nullptr);
    }

    TEST_CASE("Insert with existing key updates the value")
    {
        auto m = MakeMap();
        m.Insert(1, 10);
        m.Insert(1, 99);   // update
        CHECK(m.GetNum() == 1u);
        CHECK(m.Get(1) == 99);
    }

    TEST_CASE("Remove deletes an entry")
    {
        auto m = MakeMap();
        m.Insert(1, 10);
        m.Insert(2, 20);
        m.Insert(3, 30);
        CHECK(m.Remove(2));
        CHECK(m.GetNum() == 2u);
        CHECK(!m.Contains(2));
        CHECK( m.Contains(1));
        CHECK( m.Contains(3));
    }

    TEST_CASE("Remove returns false for absent key")
    {
        auto m = MakeMap();
        CHECK(!m.Remove(99));
    }

    TEST_CASE("Remove then re-insert works correctly")
    {
        auto m = MakeMap();
        m.Insert(1, 10);
        m.Insert(2, 20);
        m.Remove(1);
        m.Insert(1, 11);
        CHECK(m.GetNum() == 2u);
        CHECK(m.Get(1) == 11);
        CHECK(m.Get(2) == 20);
    }

    TEST_CASE("FindOrAdd inserts a new key with supplied value")
    {
        auto m = MakeMap();
        int* v = m.FindOrAdd(1, 10);
        REQUIRE(v != nullptr);
        CHECK(*v == 10);
        CHECK(m.GetNum() == 1u);
    }

    TEST_CASE("FindOrAdd returns existing value without overwriting")
    {
        auto m = MakeMap();
        m.Insert(1, 10);
        int* v = m.FindOrAdd(1, 999);   // key already present
        REQUIRE(v != nullptr);
        CHECK(*v == 10);                // original value unchanged
    }

    TEST_CASE("FindOrAddDefaulted inserts default value for new key")
    {
        auto m = MakeMap();
        int* v = m.FindOrAddDefaulted(7);
        REQUIRE(v != nullptr);
        CHECK(*v == 0);   // default int
        CHECK(m.GetNum() == 1u);
    }

    TEST_CASE("Emplace constructs value in-place for new key")
    {
        auto m = MakeMap();
        CHECK(m.Emplace(3, 333));
        CHECK(m.GetNum() == 1u);
        CHECK(m.Get(3) == 333);
    }

    TEST_CASE("Reset clears all entries")
    {
        auto m = MakeMap();
        m.Insert(1, 1);
        m.Insert(2, 2);
        m.Insert(3, 3);
        m.Reset();
        CHECK(m.IsEmpty());
        CHECK(!m.Contains(1));
        CHECK(!m.Contains(2));
    }

    TEST_CASE("const iteration visits every entry exactly once")
    {
        auto m = MakeMap();
        m.Insert(1, 100);
        m.Insert(2, 200);
        m.Insert(3, 300);
        int keySum = 0, valSum = 0, count = 0;
        const auto& cm = m;   // use ConstIter (returns const TItem&)
        for (const auto& item : cm)
        {
            keySum += item.Key;
            valSum += item.Value;
            ++count;
        }
        CHECK(count  == 3);
        CHECK(keySum == 6);
        CHECK(valSum == 600);
    }
}
