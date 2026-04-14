
#include <doctest/doctest.h>

#include "PhoenixSim/Containers/FixedArray.h"

using namespace Phoenix;

TEST_SUITE("TInlineArray")
{
    TEST_CASE("starts empty")
    {
        TInlineArray<int, 8> arr;
        CHECK(arr.IsEmpty());
        CHECK(arr.GetNum() == 0);
        CHECK(arr.GetCapacity() == 8u);
        CHECK(!arr.IsFull());
    }

    TEST_CASE("PushBack grows size")
    {
        TInlineArray<int, 4> arr;
        CHECK(arr.PushBack(10));
        CHECK(arr.PushBack(20));
        CHECK(arr.PushBack(30));
        CHECK(arr.GetNum() == 3u);
        CHECK(!arr.IsEmpty());
        CHECK(!arr.IsFull());
        CHECK(arr.PushBack(40));
        CHECK(arr.IsFull());
    }

    TEST_CASE("IsFull is true after pushing capacity elements")
    {
        TInlineArray<int, 2> arr;
        CHECK(arr.PushBack(1));
        CHECK(arr.PushBack(2));
        CHECK(arr.IsFull());
        CHECK(arr.GetNum() == 2u);
    }

    TEST_CASE("operator[] accesses elements in push order")
    {
        TInlineArray<int, 4> arr;
        arr.PushBack(100);
        arr.PushBack(200);
        arr.PushBack(300);
        CHECK(arr[0] == 100);
        CHECK(arr[1] == 200);
        CHECK(arr[2] == 300);
    }

    TEST_CASE("Front and Back return correct elements")
    {
        TInlineArray<int, 4> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);
        CHECK(arr.Front() == 1);
        CHECK(arr.Back()  == 3);
    }

    TEST_CASE("PopBack decrements size and updates Back")
    {
        TInlineArray<int, 4> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);
        arr.PopBack();
        CHECK(arr.GetNum() == 2u);
        CHECK(arr.Back() == 2);
    }

    TEST_CASE("Reset clears all elements")
    {
        TInlineArray<int, 4> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.Reset();
        CHECK(arr.IsEmpty());
        CHECK(arr.GetNum() == 0u);
    }

    TEST_CASE("Insert at index shifts subsequent elements right")
    {
        TInlineArray<int, 8> arr;
        arr.PushBack(1);
        arr.PushBack(3);
        arr.PushBack(4);
        arr.Insert(2, 1);   // insert value 2 at index 1
        CHECK(arr.GetNum() == 4u);
        CHECK(arr[0] == 1);
        CHECK(arr[1] == 2);
        CHECK(arr[2] == 3);
        CHECK(arr[3] == 4);
    }

    TEST_CASE("RemoveAt preserves order of remaining elements")
    {
        TInlineArray<int, 8> arr;
        for (int i = 1; i <= 5; ++i) arr.PushBack(i);
        CHECK(arr.RemoveAt(2));     // remove index 2 (value 3)
        CHECK(arr.GetNum() == 4u);
        CHECK(arr[0] == 1);
        CHECK(arr[1] == 2);
        CHECK(arr[2] == 4);
        CHECK(arr[3] == 5);
    }

    TEST_CASE("RemoveAt returns false for out-of-range index")
    {
        TInlineArray<int, 4> arr;
        arr.PushBack(1);
        CHECK(!arr.RemoveAt(5));
        CHECK(arr.GetNum() == 1u);
    }

    TEST_CASE("RemoveAtUnsorted replaces removed slot with last element")
    {
        TInlineArray<int, 8> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);
        arr.PushBack(4);
        CHECK(arr.RemoveAtUnsorted(1));  // remove index 1 (value 2), last (4) moves in
        CHECK(arr.GetNum() == 3u);
        CHECK(arr[0] == 1);
        CHECK(arr[1] == 4);
        CHECK(arr[2] == 3);
    }

    TEST_CASE("IndexOf finds first match")
    {
        TInlineArray<int, 8> arr;
        arr.PushBack(10);
        arr.PushBack(20);
        arr.PushBack(30);
        CHECK(arr.IndexOf(20)  == 1);
        CHECK(arr.IndexOf(99)  == INDEX_NONE);
    }

    TEST_CASE("Contains returns true iff element is present")
    {
        TInlineArray<int, 8> arr;
        arr.PushBack(10);
        arr.PushBack(20);
        CHECK( arr.Contains(10));
        CHECK( arr.Contains(20));
        CHECK(!arr.Contains(99));
    }

    TEST_CASE("SetNum fills up to requested size with default value")
    {
        TInlineArray<int, 8> arr;
        arr.SetNum(4, 7);
        CHECK(arr.GetNum() == 4u);
        for (uint32 i = 0; i < 4; ++i) CHECK(arr[i] == 7);
    }

    TEST_CASE("SetNum truncates when new size is smaller")
    {
        TInlineArray<int, 8> arr;
        arr.SetNum(5, 1);
        arr.SetNum(2);
        CHECK(arr.GetNum() == 2u);
    }

    TEST_CASE("Fill populates array to capacity")
    {
        TInlineArray<int, 4> arr;
        arr.Fill(42);
        CHECK(arr.IsFull());
        CHECK(arr.GetNum() == 4u);
        for (uint32 i = 0; i < 4; ++i) CHECK(arr[i] == 42);
    }

    TEST_CASE("range-for iterates all elements in order")
    {
        TInlineArray<int, 8> arr;
        for (int i = 0; i < 5; ++i) arr.PushBack(i * 10);
        int sum = 0;
        for (int v : arr) sum += v;
        CHECK(sum == 100);   // 0+10+20+30+40
    }

    TEST_CASE("const range-for iterates all elements in order")
    {
        TInlineArray<int, 8> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);
        const auto& constArr = arr;
        int sum = 0;
        for (int v : constArr) sum += v;
        CHECK(sum == 6);
    }
}
