
#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "PhoenixSim/Containers/ChaseLevDeque.h"

using namespace Phoenix;
using namespace std::chrono_literals;

TEST_SUITE("TChaseLevDeque")
{
    // ─────────────────────────────────────────────────────────────────────
    // Single-threaded behaviour. The owner-only paths must agree with a
    // textbook LIFO when no thieves are racing.
    // ─────────────────────────────────────────────────────────────────────

    TEST_CASE("starts empty")
    {
        TChaseLevDeque<int*> q;
        CHECK(q.IsEmptyApprox());
        CHECK(q.SizeApprox() == 0);
        int* out = nullptr;
        CHECK_FALSE(q.PopOwner(out));
        CHECK_FALSE(q.TrySteal(out));
    }

    TEST_CASE("owner sees LIFO ordering across Push/PopOwner")
    {
        TChaseLevDeque<int*> q;
        int a = 1, b = 2, c = 3;
        CHECK(q.Push(&a));
        CHECK(q.Push(&b));
        CHECK(q.Push(&c));
        CHECK(q.SizeApprox() == 3);

        int* out = nullptr;
        CHECK(q.PopOwner(out)); CHECK(out == &c);
        CHECK(q.PopOwner(out)); CHECK(out == &b);
        CHECK(q.PopOwner(out)); CHECK(out == &a);
        CHECK_FALSE(q.PopOwner(out));
    }

    TEST_CASE("thief sees FIFO ordering across Push/TrySteal")
    {
        TChaseLevDeque<int*> q;
        int a = 1, b = 2, c = 3;
        q.Push(&a);
        q.Push(&b);
        q.Push(&c);

        int* out = nullptr;
        CHECK(q.TrySteal(out)); CHECK(out == &a);
        CHECK(q.TrySteal(out)); CHECK(out == &b);
        CHECK(q.TrySteal(out)); CHECK(out == &c);
        CHECK_FALSE(q.TrySteal(out));
    }

    TEST_CASE("Push returns false when capacity is reached")
    {
        TChaseLevDeque<int*, 4> q;
        int v = 0;
        CHECK(q.Push(&v));
        CHECK(q.Push(&v));
        CHECK(q.Push(&v));
        CHECK(q.Push(&v));
        CHECK_FALSE(q.Push(&v));
    }

    TEST_CASE("PopOwner on the last element succeeds without contention")
    {
        TChaseLevDeque<int*> q;
        int a = 1;
        q.Push(&a);
        int* out = nullptr;
        CHECK(q.PopOwner(out));
        CHECK(out == &a);
        CHECK(q.IsEmptyApprox());
    }

    TEST_CASE("ring buffer wraps cleanly after a drain")
    {
        TChaseLevDeque<int*, 4> q;
        int values[8] = {1, 2, 3, 4, 5, 6, 7, 8};

        // Fill, drain, fill, drain — exercises the modulo wrap of Top/Bottom.
        for (int round = 0; round < 4; ++round)
        {
            for (int i = 0; i < 4; ++i) CHECK(q.Push(&values[i]));
            for (int i = 3; i >= 0; --i)
            {
                int* out = nullptr;
                CHECK(q.PopOwner(out));
                CHECK(out == &values[i]);
            }
            CHECK(q.IsEmptyApprox());
        }
    }

    TEST_CASE("owner and thief on opposite ends do not collide on small N")
    {
        TChaseLevDeque<int*, 8> q;
        int a = 1, b = 2, c = 3, d = 4;
        q.Push(&a);
        q.Push(&b);
        q.Push(&c);
        q.Push(&d);

        int* stolen = nullptr;
        int* popped = nullptr;
        CHECK(q.TrySteal(stolen));   // takes 'a' from top
        CHECK(stolen == &a);
        CHECK(q.PopOwner(popped));   // takes 'd' from bottom
        CHECK(popped == &d);
        CHECK(q.SizeApprox() == 2);

        CHECK(q.TrySteal(stolen));   // 'b'
        CHECK(stolen == &b);
        CHECK(q.PopOwner(popped));   // 'c'
        CHECK(popped == &c);
        CHECK(q.IsEmptyApprox());
    }

    // ─────────────────────────────────────────────────────────────────────
    // Multi-threaded stress: one owner, several thieves. Every pushed item
    // must come out exactly once (no losses, no duplicates) regardless of
    // who takes it.
    // ─────────────────────────────────────────────────────────────────────

    TEST_CASE("owner-with-thieves: every item is consumed exactly once")
    {
        constexpr int kProducerItems = 50000;
        constexpr int kNumThieves    = 4;
        constexpr std::int64_t kCapacity = 1024;

        TChaseLevDeque<std::intptr_t, kCapacity> q;

        std::atomic<int>      stolen{0};
        std::atomic<int>      popped{0};
        std::atomic<long long> sumConsumed{0};
        std::atomic<bool>     producerDone{false};

        std::thread owner([&]
        {
            for (int i = 1; i <= kProducerItems; ++i)
            {
                // Push may transiently fail if the deque is full; the owner
                // helps drain by popping its own work.
                while (!q.Push(static_cast<std::intptr_t>(i)))
                {
                    std::intptr_t v;
                    if (q.PopOwner(v))
                    {
                        sumConsumed.fetch_add(static_cast<long long>(v),
                            std::memory_order_relaxed);
                        popped.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            }
            // Owner drains anything left in its own deque after pushing.
            std::intptr_t v;
            while (q.PopOwner(v))
            {
                sumConsumed.fetch_add(static_cast<long long>(v),
                    std::memory_order_relaxed);
                popped.fetch_add(1, std::memory_order_relaxed);
            }
            producerDone.store(true, std::memory_order_release);
        });

        std::vector<std::thread> thieves;
        thieves.reserve(kNumThieves);
        for (int t = 0; t < kNumThieves; ++t)
        {
            thieves.emplace_back([&]
            {
                while (!producerDone.load(std::memory_order_acquire) ||
                       !q.IsEmptyApprox())
                {
                    std::intptr_t v;
                    if (q.TrySteal(v))
                    {
                        sumConsumed.fetch_add(static_cast<long long>(v),
                            std::memory_order_relaxed);
                        stolen.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            });
        }

        owner.join();
        for (auto& th : thieves) th.join();

        // Every item must have been consumed exactly once. Sum check is
        // a cheap proxy for "no duplicates, no losses": sum(1..N) == N(N+1)/2.
        const long long expectedSum =
            static_cast<long long>(kProducerItems) *
            static_cast<long long>(kProducerItems + 1) / 2;
        CHECK(stolen.load() + popped.load() == kProducerItems);
        CHECK(sumConsumed.load() == expectedSum);
    }

    TEST_CASE("contested last element is taken by exactly one thread")
    {
        // Repeatedly: push exactly one item, then race owner PopOwner against
        // many thieves' TrySteal. Across many trials, the item must always
        // be taken by exactly one of them.
        constexpr int kTrials = 5000;
        constexpr int kNumThieves = 3;

        for (int trial = 0; trial < kTrials; ++trial)
        {
            TChaseLevDeque<std::intptr_t, 16> q;
            std::intptr_t sentinel = 0xDEADBEEF;
            q.Push(sentinel);

            std::atomic<int> winners{0};
            std::atomic<int> startGun{0};

            auto worker = [&](bool isOwner)
            {
                // Cheap barrier: spin until startGun ticks.
                ++startGun;
                while (startGun.load(std::memory_order_acquire) <= kNumThieves)
                {
                    /* spin */
                }

                std::intptr_t v = 0;
                bool got = isOwner ? q.PopOwner(v) : q.TrySteal(v);
                if (got)
                {
                    CHECK(v == sentinel);
                    winners.fetch_add(1, std::memory_order_relaxed);
                }
            };

            std::thread owner(worker, true);
            std::vector<std::thread> thieves;
            for (int i = 0; i < kNumThieves; ++i)
            {
                thieves.emplace_back(worker, false);
            }
            owner.join();
            for (auto& t : thieves) t.join();

            CHECK(winners.load() == 1);
        }
    }
}
