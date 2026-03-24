
#include <doctest/doctest.h>

#include "PhoenixSim/Containers/FixedQueue.h"

using namespace Phoenix;

TEST_SUITE("TInlineQueue")
{
    TEST_CASE("starts empty")
    {
        TInlineQueue<int, 8> q;
        CHECK(q.IsEmpty());
        CHECK(!q.IsFull());
        CHECK(q.GetNum() == 0u);
        CHECK(q.GetCapacity() == 8u);
    }

    TEST_CASE("Enqueue / Dequeue maintains FIFO order")
    {
        TInlineQueue<int, 8> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Enqueue(3);
        CHECK(q.GetNum() == 3u);
        CHECK(q.Dequeue() == 1);
        CHECK(q.Dequeue() == 2);
        CHECK(q.Dequeue() == 3);
        CHECK(q.IsEmpty());
    }

    TEST_CASE("IsFull when capacity - 1 elements are enqueued")
    {
        // Ring buffer reserves one slot, so capacity N holds N-1 items.
        TInlineQueue<int, 4> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Enqueue(3);
        CHECK(q.IsFull());
        CHECK(q.GetNum() == 3u);
    }

    TEST_CASE("wraps around the ring buffer correctly")
    {
        TInlineQueue<int, 4> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Dequeue();         // Start advances
        q.Dequeue();
        q.Enqueue(3);        // these slots wrap around
        q.Enqueue(4);
        q.Enqueue(5);
        CHECK(q.GetNum() == 3u);
        CHECK(q.Dequeue() == 3);
        CHECK(q.Dequeue() == 4);
        CHECK(q.Dequeue() == 5);
        CHECK(q.IsEmpty());
    }

    TEST_CASE("Reset clears the queue")
    {
        TInlineQueue<int, 8> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Reset();
        CHECK(q.IsEmpty());
        CHECK(q.GetNum() == 0u);
    }

    TEST_CASE("Contains finds present elements")
    {
        TInlineQueue<int, 8> q;
        q.Enqueue(10);
        q.Enqueue(20);
        q.Enqueue(30);
        CHECK( q.Contains(10));
        CHECK( q.Contains(20));
        CHECK(!q.Contains(99));
    }

    TEST_CASE("operator[] accesses by logical index from front")
    {
        TInlineQueue<int, 8> q;
        q.Enqueue(100);
        q.Enqueue(200);
        q.Enqueue(300);
        CHECK(q[0] == 100);
        CHECK(q[1] == 200);
        CHECK(q[2] == 300);
    }

    TEST_CASE("operator[] works correctly after wrap-around")
    {
        TInlineQueue<int, 4> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Dequeue();
        q.Dequeue();
        q.Enqueue(3);
        q.Enqueue(4);
        CHECK(q[0] == 3);
        CHECK(q[1] == 4);
    }

    TEST_CASE("EnqueueUnique skips duplicate values")
    {
        TInlineQueue<int, 8> q;
        q.Enqueue(1);
        q.EnqueueUnique(1);   // duplicate — should not enqueue
        q.EnqueueUnique(2);   // new value
        CHECK(q.GetNum() == 2u);
        CHECK(q.Dequeue() == 1);
        CHECK(q.Dequeue() == 2);
    }

    TEST_CASE("AllowOverwrite mode overwrites oldest when full")
    {
        TInlineQueue<int, 4, true> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Enqueue(3);
        CHECK(q.IsFull());
        q.Enqueue(4);           // overwrites 1
        CHECK(q.GetNum() == 3u);
        CHECK(q.Dequeue() == 2);
        CHECK(q.Dequeue() == 3);
        CHECK(q.Dequeue() == 4);
    }

    TEST_CASE("MoveIndex static helper wraps correctly")
    {
        // Validate the compile-time asserts are consistent at runtime too.
        CHECK(TInlineQueue<int, 32>::MoveIndex(0,  32,  1)  ==  1u);
        CHECK(TInlineQueue<int, 32>::MoveIndex(0,  32, -1)  == 31u);
        CHECK(TInlineQueue<int, 32>::MoveIndex(31, 32,  1)  ==  0u);
        CHECK(TInlineQueue<int, 32>::MoveIndex(0,  32, 32)  ==  0u);
        CHECK(TInlineQueue<int, 32>::MoveIndex(0,  32, -32) ==  0u);
    }
}
