
#include <doctest/doctest.h>

#include "Phoenix.Sim/Containers/FixedWeightedSet.h"

using namespace Phoenix;

// Use float weights for simplicity so comparisons are straightforward.
// Random is not needed for deterministic SampleIndex tests.
template <uint32 N>
using WSet = TInlineWeightedSet<int, N, float>;

TEST_SUITE("TInlineWeightedSet")
{
    TEST_CASE("starts empty with zero total weight")
    {
        WSet<8> ws;
        CHECK(ws.IsEmpty());
        CHECK(!ws.IsFull());
        CHECK(ws.GetNum() == 0u);
        CHECK(ws.GetCapacity() == 8u);
        CHECK(ws.GetTotalWeight() == doctest::Approx(0.0f));
    }

    TEST_CASE("PushBack adds items and accumulates total weight")
    {
        WSet<8> ws;
        CHECK(ws.PushBack({ 1, 1.0f }));
        CHECK(ws.PushBack({ 2, 2.0f }));
        CHECK(ws.PushBack({ 3, 3.0f }));
        CHECK(ws.GetNum() == 3u);
        CHECK(ws.GetTotalWeight() == doctest::Approx(6.0f));
    }

    TEST_CASE("IsFull is true after pushing capacity items")
    {
        WSet<2> ws;
        CHECK(ws.PushBack({ 1, 1.0f }));
        CHECK(ws.PushBack({ 2, 2.0f }));
        CHECK(ws.IsFull());
        CHECK(ws.GetNum() == 2u);
    }

    TEST_CASE("Remove subtracts weight and shrinks the set")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        ws.PushBack({ 2, 2.0f });
        ws.PushBack({ 3, 3.0f });
        CHECK(ws.Remove(2));
        CHECK(ws.GetNum() == 2u);
        CHECK(ws.GetTotalWeight() == doctest::Approx(4.0f));
    }

    TEST_CASE("Remove returns false for absent value")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        CHECK(!ws.Remove(99));
        CHECK(ws.GetNum() == 1u);
    }

    TEST_CASE("RemoveAt removes by index and updates total weight")
    {
        WSet<8> ws;
        ws.PushBack({ 10, 1.0f });
        ws.PushBack({ 20, 4.0f });
        CHECK(ws.RemoveAt(0));   // removes weight 1.0
        CHECK(ws.GetNum() == 1u);
        CHECK(ws.GetTotalWeight() == doctest::Approx(4.0f));
    }

    TEST_CASE("RemoveAt out-of-range returns false")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        CHECK(!ws.RemoveAt(99));
    }

    TEST_CASE("SetWeight updates an item's weight and total")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        ws.PushBack({ 2, 2.0f });
        CHECK(ws.SetWeight(0, 10.0f));
        CHECK(ws.GetWeight(0) == doctest::Approx(10.0f));
        CHECK(ws.GetTotalWeight() == doctest::Approx(12.0f));
    }

    TEST_CASE("GetItemChance returns weight fraction")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        ws.PushBack({ 2, 3.0f });
        // total = 4; item[0] chance = 1/4 = 0.25; item[1] chance = 3/4 = 0.75
        // GetItemChance returns Value (fixed-point); cast to float for comparison.
        CHECK((float)ws.GetItemChance(0) == doctest::Approx(0.25f).epsilon(0.001f));
        CHECK((float)ws.GetItemChance(1) == doctest::Approx(0.75f).epsilon(0.001f));
    }

    TEST_CASE("GetItemChance returns 1 for a single-item set (trivially)")
    {
        WSet<4> ws;
        ws.PushBack({ 42, 5.0f });
        CHECK((float)ws.GetItemChance(0) == doctest::Approx(1.0f).epsilon(0.001f));
    }

    TEST_CASE("SampleIndex selects the correct bucket for a given weight")
    {
        WSet<8> ws;
        ws.PushBack({ 10, 1.0f });   // cumulative [0, 1]
        ws.PushBack({ 20, 2.0f });   // cumulative [1, 3]
        ws.PushBack({ 30, 3.0f });   // cumulative [3, 6]

        CHECK(ws.SampleIndex(0.0f) == 0u);   // at start → bucket 0
        CHECK(ws.SampleIndex(1.0f) == 0u);   // at 1.0 → bucket 0
        CHECK(ws.SampleIndex(1.5f) == 1u);   // in [1,3] → bucket 1
        CHECK(ws.SampleIndex(3.5f) == 2u);   // in [3,6] → bucket 2
        CHECK(ws.SampleIndex(9.9f) == 2u);   // clamped → last bucket
    }

    TEST_CASE("SampleIndex on empty set returns INDEX_NONE")
    {
        WSet<8> ws;
        CHECK(ws.SampleIndex(0.0f) == Index<uint32>::None);
    }

    TEST_CASE("Sample fills outValue for a valid weight")
    {
        WSet<8> ws;
        ws.PushBack({ 42, 1.0f });
        int out = 0;
        CHECK(ws.Sample(0.5f, out));
        CHECK(out == 42);
    }

    TEST_CASE("Sample returns false for empty set")
    {
        WSet<8> ws;
        int out = 0;
        CHECK(!ws.Sample(0.0f, out));
    }

    TEST_CASE("Reset clears items and total weight")
    {
        WSet<8> ws;
        ws.PushBack({ 1, 1.0f });
        ws.PushBack({ 2, 2.0f });
        ws.Reset();
        CHECK(ws.IsEmpty());
        CHECK(ws.GetTotalWeight() == doctest::Approx(0.0f));
    }

    TEST_CASE("range-for (const) iterates all items")
    {
        WSet<8> ws;
        ws.PushBack({ 10, 1.0f });
        ws.PushBack({ 20, 2.0f });
        ws.PushBack({ 30, 3.0f });
        float weightSum = 0.0f;
        int count = 0;
        for (const auto& item : ws)
        {
            weightSum += item.Weight;
            ++count;
        }
        CHECK(count == 3);
        CHECK(weightSum == doctest::Approx(6.0f));
    }
}
