
#include <doctest/doctest.h>

#include <thread>
#include <vector>
#include <atomic>
#include <numeric>

#include "PhoenixSim/Containers/MPMCQueue.h"

using namespace Phoenix;

TEST_SUITE("TMPMCQueue")
{
    // -------------------------------------------------------------------------
    // Single-threaded behaviour
    // -------------------------------------------------------------------------

    TEST_CASE("starts empty")
    {
        TMPMCQueue<int> q(8);
        CHECK(q.IsEmpty());
        CHECK(q.GetCapacity() == 8u);
    }

    TEST_CASE("TryEnqueue / TryDequeue maintains FIFO order")
    {
        TMPMCQueue<int> q(8);
        CHECK(q.TryEnqueue(1));
        CHECK(q.TryEnqueue(2));
        CHECK(q.TryEnqueue(3));
        CHECK(!q.IsEmpty());

        int v = 0;
        CHECK(q.TryDequeue(v)); CHECK(v == 1);
        CHECK(q.TryDequeue(v)); CHECK(v == 2);
        CHECK(q.TryDequeue(v)); CHECK(v == 3);
        CHECK(q.IsEmpty());
    }

    TEST_CASE("TryDequeue returns false when empty")
    {
        TMPMCQueue<int> q(4);
        int v = 0;
        CHECK(!q.TryDequeue(v));
    }

    TEST_CASE("TryEnqueue returns false when full (all capacity slots used)")
    {
        // Unlike a single-writer ring buffer, TMPMCQueue uses all N slots.
        TMPMCQueue<int> q(4);
        CHECK(q.TryEnqueue(1));
        CHECK(q.TryEnqueue(2));
        CHECK(q.TryEnqueue(3));
        CHECK(q.TryEnqueue(4));   // 4th item fills all 4 slots
        CHECK(!q.TryEnqueue(5));  // now truly full
    }

    TEST_CASE("wraps around correctly after partial drain")
    {
        TMPMCQueue<int> q(4);
        q.TryEnqueue(1);
        q.TryEnqueue(2);
        int v = 0;
        q.TryDequeue(v);   // consume 1 → slots recycled
        q.TryDequeue(v);   // consume 2
        q.TryEnqueue(3);
        q.TryEnqueue(4);
        q.TryEnqueue(5);
        q.TryEnqueue(6);
        CHECK(!q.TryEnqueue(7));  // full again after 4 items
        CHECK(q.TryDequeue(v)); CHECK(v == 3);
        CHECK(q.TryDequeue(v)); CHECK(v == 4);
        CHECK(q.TryDequeue(v)); CHECK(v == 5);
        CHECK(q.TryDequeue(v)); CHECK(v == 6);
        CHECK(q.IsEmpty());
    }

    // Note: TryEnqueue takes 'const T&', so move-only types (unique_ptr) are not
    // supported — the implementation copies the item into the cell.

    // -------------------------------------------------------------------------
    // Multi-threaded: stress test
    // -------------------------------------------------------------------------

    TEST_CASE("multi-producer multi-consumer all items transferred exactly once")
    {
        constexpr int kProducers = 4;
        constexpr int kConsumers = 4;
        constexpr int kItemsPerProducer = 500;
        constexpr int kTotal = kProducers * kItemsPerProducer;

        TMPMCQueue<int> q(512);
        std::atomic<int> consumed{ 0 };
        std::atomic<int> sum{ 0 };

        std::vector<std::thread> producers, consumers;

        for (int p = 0; p < kProducers; ++p)
        {
            producers.emplace_back([&, p]()
            {
                for (int i = 0; i < kItemsPerProducer; ++i)
                {
                    while (!q.TryEnqueue(1)) { /* busy-wait */ }
                }
            });
        }

        for (int c = 0; c < kConsumers; ++c)
        {
            consumers.emplace_back([&]()
            {
                while (consumed.load(std::memory_order_relaxed) < kTotal)
                {
                    int v = 0;
                    if (q.TryDequeue(v))
                    {
                        sum.fetch_add(v, std::memory_order_relaxed);
                        consumed.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }

        for (auto& t : producers) t.join();
        for (auto& t : consumers) t.join();

        CHECK(consumed.load() == kTotal);
        CHECK(sum.load()      == kTotal);   // every item had value 1
        CHECK(q.IsEmpty());
    }
}
