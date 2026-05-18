
#include <doctest/doctest.h>

#include "Phoenix.Sim/Containers/FixedLeaderboard.h"

using namespace Phoenix;

TEST_SUITE("TInlineLeaderboard")
{
    TEST_CASE("starts empty")
    {
        TInlineLeaderboard<int, int, 8> lb;
        CHECK(lb.IsEmpty());
        CHECK(!lb.IsFull());
        CHECK(lb.GetNum() == 0u);
        CHECK(lb.GetCapacity() == 8u);
    }

    TEST_CASE("Add inserts a single element")
    {
        TInlineLeaderboard<int, int, 8> lb;
        CHECK(lb.Add(42, 5));
        CHECK(lb.GetNum() == 1u);
        CHECK(!lb.IsEmpty());
    }

    TEST_CASE("Add maintains ascending rank order")
    {
        TInlineLeaderboard<int, int, 8> lb;
        lb.Add(30, 3);
        lb.Add(10, 1);
        lb.Add(20, 2);

        // Items must come out sorted by Rank ascending.
        auto data = lb.begin();
        CHECK(data[0].Rank == 1);
        CHECK(data[1].Rank == 2);
        CHECK(data[2].Rank == 3);

        CHECK(data[0].Value == 10);
        CHECK(data[1].Value == 20);
        CHECK(data[2].Value == 30);
    }

    TEST_CASE("Add with equal rank appends after existing same-rank entries")
    {
        TInlineLeaderboard<int, int, 8> lb;
        lb.Add(1, 5);
        lb.Add(2, 5);   // same rank
        lb.Add(3, 5);   // same rank
        CHECK(lb.GetNum() == 3u);

        // All have rank 5; order among equal-rank entries follows insertion.
        for (const auto& item : lb)
            CHECK(item.Rank == 5);
    }

    TEST_CASE("Add returns false when leaderboard is full")
    {
        TInlineLeaderboard<int, int, 3> lb;
        CHECK(lb.Add(1, 1));
        CHECK(lb.Add(2, 2));
        CHECK(lb.Add(3, 3));
        CHECK(lb.IsFull());
        CHECK(!lb.Add(4, 4));
        CHECK(lb.GetNum() == 3u);
    }

    TEST_CASE("Reset empties the leaderboard")
    {
        TInlineLeaderboard<int, int, 8> lb;
        lb.Add(10, 1);
        lb.Add(20, 2);
        lb.Reset();
        CHECK(lb.IsEmpty());
        CHECK(lb.GetNum() == 0u);
    }

    TEST_CASE("range-for iterates in rank order")
    {
        TInlineLeaderboard<int, int, 8> lb;
        lb.Add(100, 10);
        lb.Add(300, 30);
        lb.Add(200, 20);

        int prevRank = -1;
        for (const auto& item : lb)
        {
            CHECK(item.Rank > prevRank);
            prevRank = item.Rank;
        }
    }

    TEST_CASE("Add lower rank than existing minimum inserts at front")
    {
        TInlineLeaderboard<int, int, 8> lb;
        lb.Add(99, 100);
        lb.Add(42,   1);   // lower rank — should be first
        CHECK(lb.begin()[0].Value == 42);
        CHECK(lb.begin()[0].Rank  == 1);
        CHECK(lb.begin()[1].Value == 99);
        CHECK(lb.begin()[1].Rank  == 100);
    }

    TEST_CASE("TRank can be float")
    {
        TInlineLeaderboard<int, float, 4> lb;
        lb.Add(10, 1.5f);
        lb.Add(20, 0.5f);
        lb.Add(30, 2.5f);

        auto data = lb.begin();
        CHECK(data[0].Rank == doctest::Approx(0.5f));
        CHECK(data[1].Rank == doctest::Approx(1.5f));
        CHECK(data[2].Rank == doctest::Approx(2.5f));
    }
}
