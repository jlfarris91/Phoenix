
#include <doctest/doctest.h>

#include "Phoenix.Sim/Containers/FixedSortedList.h"

using namespace Phoenix;

// ---------------------------------------------------------------------------
// Minimal item type that satisfies TSortedList's requirements:
//   - IsValid() / Invalidate()
//   - a TGetKey functor that returns the sortable key
// ---------------------------------------------------------------------------

struct TestOrder
{
    uint32 UnitId = 0;   // sort key; 0 = invalid sentinel
    int32  Data   = 0;

    bool IsValid() const { return UnitId != 0; }
    void Invalidate()    { UnitId = 0; }

    // Required by PushBackUnique / Contains (std::equal_to<TestOrder> default).
    bool operator==(const TestOrder& o) const = default;
};

struct GetOrderUnitId
{
    uint32 operator()(const TestOrder& o) const { return o.UnitId; }
};

// Convenience alias.
template <uint32 N>
using OrderList = TInlineSortedList<TestOrder, GetOrderUnitId, N>;

TEST_SUITE("TInlineSortedList")
{
    TEST_CASE("starts empty")
    {
        OrderList<16> list;
        CHECK(list.IsEmpty());
        CHECK(!list.IsFull());
        CHECK(list.GetNum() == 0u);
        CHECK(list.GetNumValidItems() == 0u);
        CHECK(list.GetCapacity() == 16u);
    }

    TEST_CASE("PushBack adds an item and increments NumValidItems")
    {
        OrderList<16> list;
        CHECK(list.PushBack({ 1u, 100 }));
        CHECK(list.GetNum() == 1u);
        CHECK(list.GetNumValidItems() == 1u);
        CHECK(!list.IsEmpty());
    }

    TEST_CASE("PushBackUnique rejects exact-duplicate items")
    {
        // PushBackUnique uses std::equal_to<T> by default, which compares the
        // whole item (all fields). A different Data value with the same key is
        // NOT a duplicate and WILL be inserted.
        OrderList<16> list;
        CHECK(list.PushBack({ 1u, 10 }));
        CHECK(!list.PushBackUnique({ 1u, 10 }));   // identical item → rejected
        CHECK(list.GetNum() == 1u);
    }

    TEST_CASE("PushBackUnique accepts a different item even with the same key")
    {
        OrderList<16> list;
        CHECK(list.PushBack({ 1u, 10 }));
        CHECK(list.PushBackUnique({ 1u, 99 }));    // same key, different data → accepted
        CHECK(list.GetNum() == 2u);
    }

    TEST_CASE("PushBackUnique accepts different keys")
    {
        OrderList<16> list;
        CHECK(list.PushBack({ 1u, 10 }));
        CHECK(list.PushBackUnique({ 2u, 20 }));
        CHECK(list.GetNum() == 2u);
        CHECK(list.GetNumValidItems() == 2u);
    }

    TEST_CASE("Contains finds a present item")
    {
        OrderList<16> list;
        list.PushBack({ 5u, 50 });
        CHECK( list.Contains({ 5u, 50 }));
        CHECK(!list.Contains({ 6u, 60 }));
    }

    TEST_CASE("Sort moves invalid items to back and establishes sorted range")
    {
        OrderList<16> list;
        list.PushBack({ 3u, 30 });
        list.PushBack({ 1u, 10 });
        list.PushBack({ 2u, 20 });
        list.Sort();

        CHECK(list.GetSortedNum() == 3u);
        // After Sort, elements in [0..GetSortedNum()) are valid and sorted by UnitId.
        const TestOrder* data = list.GetData();
        CHECK(data[0].UnitId == 1u);
        CHECK(data[1].UnitId == 2u);
        CHECK(data[2].UnitId == 3u);
    }

    TEST_CASE("Sort compacts invalid items to back and shrinks size")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 10 });
        list.PushBack({ 2u, 20 });
        list.PushBack({ 3u, 30 });
        list.RemoveAt(1);   // invalidate index 1 (UnitId=2)
        list.Sort();        // should compact to 2 valid items

        CHECK(list.GetSortedNum() == 2u);
        CHECK(list.GetNum()       == 2u);
        CHECK(list.GetNumValidItems() == 2u);
    }

    TEST_CASE("RemoveAt invalidates item and decrements NumValidItems")
    {
        OrderList<16> list;
        list.PushBack({ 10u, 100 });
        list.PushBack({ 20u, 200 });
        CHECK(list.RemoveAt(0));
        CHECK(list.GetNumValidItems() == 1u);
        CHECK(!list.RemoveAt(0));   // already invalid — returns false
    }

    TEST_CASE("RemoveAt out-of-range returns false")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 1 });
        CHECK(!list.RemoveAt(99));
    }

    TEST_CASE("GetFirstItem finds by key after Sort")
    {
        OrderList<16> list;
        list.PushBack({ 5u, 50 });
        list.PushBack({ 3u, 30 });
        list.PushBack({ 7u, 70 });
        list.Sort();

        uint32 outIndex = INDEX_NONE;
        const TestOrder* item = list.GetFirstItem(5u, outIndex);
        REQUIRE(item != nullptr);
        CHECK(item->UnitId == 5u);
        CHECK(item->Data   == 50);
        CHECK(outIndex != static_cast<uint32>(INDEX_NONE));
    }

    TEST_CASE("GetFirstItem returns nullptr for absent key")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 10 });
        list.Sort();
        uint32 outIndex = INDEX_NONE;
        CHECK(list.GetFirstItem(99u, outIndex) == nullptr);
    }

    TEST_CASE("GetNumItems counts all valid occurrences for a key")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 10 });
        list.PushBack({ 1u, 20 });   // duplicate key — two items with UnitId=1
        list.PushBack({ 2u, 30 });
        list.Sort();

        CHECK(list.GetNumItems(1u) == 2u);
        CHECK(list.GetNumItems(2u) == 1u);
        CHECK(list.GetNumItems(9u) == 0u);
    }

    TEST_CASE("RemoveAll by key removes every matching item")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 10 });
        list.PushBack({ 1u, 20 });
        list.PushBack({ 2u, 30 });
        list.Sort();

        CHECK(list.RemoveAll(1u) == 2u);
        CHECK(list.GetNumValidItems() == 1u);
    }

    TEST_CASE("Reset clears the list entirely")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 10 });
        list.PushBack({ 2u, 20 });
        list.Reset();
        CHECK(list.IsEmpty());
        CHECK(list.GetNum() == 0u);
        CHECK(list.GetNumValidItems() == 0u);
    }

    TEST_CASE("ForEachItem visits every valid item")
    {
        OrderList<16> list;
        list.PushBack({ 1u, 100 });
        list.PushBack({ 2u, 200 });
        list.PushBack({ 3u, 300 });
        list.RemoveAt(1);   // invalidate UnitId=2

        int sum = 0;
        int count = 0;
        list.ForEachItem([&](const TestOrder& o) {
            sum += o.Data;
            ++count;
            return false;   // return false to continue iteration
        });

        CHECK(count == 2);
        CHECK(sum   == 400);   // 100 + 300
    }
}
