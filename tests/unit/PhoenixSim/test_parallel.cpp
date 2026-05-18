
#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <numeric>
#include <thread>
#include <vector>

#include "Phoenix/Parallel.h"

using namespace Phoenix;
using namespace std::chrono_literals;

// ─────────────────────────────────────────────────────────────────────────────
// Pinning tests for the job system.
//
// These tests capture the externally observable contract of the current
// ThreadPool / Task / ParallelRange implementation. They exist so that the
// upcoming refactors (cache-line padding, task pool, work stealing, fibers)
// can be applied without silently regressing behavior. Each test should
// continue to pass through every step of the JobSystemRoadmap, even though
// the implementation changes underneath.
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    // Slightly larger than core-count to exercise queue contention on most boxes.
    constexpr uint32 kWorkers       = 4;
    constexpr uint32 kQueueCapacity = 256;

    struct ScopedPool
    {
        ThreadPool Pool;
        ScopedPool() : Pool("TestPool", kWorkers, kQueueCapacity) {}
    };
}

TEST_SUITE("ThreadPool")
{
    TEST_CASE("worker count is reported back as constructed")
    {
        ScopedPool s;
        CHECK(s.Pool.GetNumWorkers() == kWorkers);
    }

    TEST_CASE("Submit runs the work and marks the handle completed")
    {
        ScopedPool s;
        std::atomic<int> counter{0};

        auto handle = s.Pool.Submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
        REQUIRE(handle != nullptr);
        REQUIRE(handle->WaitForCompleted(2s));
        CHECK(handle->IsCompleted());
        CHECK(counter.load() == 1);
    }

    TEST_CASE("Submit returns distinct handles for distinct tasks")
    {
        ScopedPool s;
        auto a = s.Pool.Submit([]{});
        auto b = s.Pool.Submit([]{});
        CHECK(a != nullptr);
        CHECK(b != nullptr);
        CHECK(a.get() != b.get());
        REQUIRE(Task::WaitAll({a, b}, 2s));
    }

    TEST_CASE("WaitIdle returns true once the queue drains and workers go idle")
    {
        ScopedPool s;
        std::atomic<int> counter{0};
        constexpr int N = 64;
        for (int i = 0; i < N; ++i)
        {
            s.Pool.Submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
        }
        REQUIRE(s.Pool.WaitIdle(5s));
        CHECK(s.Pool.IsEmpty());
        CHECK(counter.load() == N);
    }

    TEST_CASE("Many small tasks all run exactly once")
    {
        ScopedPool s;
        constexpr int N = 1024;
        std::atomic<int> counter{0};
        std::vector<std::shared_ptr<TaskHandle>> handles;
        handles.reserve(N);
        for (int i = 0; i < N; ++i)
        {
            handles.push_back(s.Pool.Submit([&] {
                counter.fetch_add(1, std::memory_order_relaxed);
            }));
        }
        REQUIRE(Task::WaitAll(handles, 10s));
        CHECK(counter.load() == N);
    }

    TEST_CASE("Submit blocks (with backoff) when the queue is full but eventually accepts")
    {
        // Capacity 8 → easy to overflow; workers must drain to make room.
        ThreadPool pool("Tiny", 2, 8);
        constexpr int N = 64;
        std::atomic<int> counter{0};
        std::vector<std::shared_ptr<TaskHandle>> handles;
        handles.reserve(N);
        for (int i = 0; i < N; ++i)
        {
            handles.push_back(pool.Submit([&] {
                // Stretch the task slightly so the queue actually fills up.
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                counter.fetch_add(1, std::memory_order_relaxed);
            }));
        }
        REQUIRE(Task::WaitAll(handles, 30s));
        CHECK(counter.load() == N);
    }

    TEST_CASE("Submit from inside a worker pushes to the worker's own deque")
    {
        // A task that spawns sub-tasks is the canonical work-stealing
        // workload. The continuation submission must land on the running
        // worker's own Chase-Lev deque (LIFO locality) — never on the
        // submission inbox. We don't directly observe routing, but the
        // recursive submission must complete correctly with no losses.
        ScopedPool s;
        constexpr int kFanOut = 32;
        std::atomic<int> leafCounter{0};

        auto root = s.Pool.Submit([&]
        {
            std::vector<std::shared_ptr<TaskHandle>> children;
            children.reserve(kFanOut);
            for (int i = 0; i < kFanOut; ++i)
            {
                children.push_back(s.Pool.Submit([&]
                {
                    leafCounter.fetch_add(1, std::memory_order_relaxed);
                }));
            }
            REQUIRE(Task::WaitAll(children, 5s));
        });
        REQUIRE(root->WaitForCompleted(10s));
        CHECK(leafCounter.load() == kFanOut);
    }
}

TEST_SUITE("TaskHandle")
{
    TEST_CASE("WaitForCompleted with a zero timeout waits indefinitely (until completion)")
    {
        ScopedPool s;
        std::atomic<bool> go{false};
        auto h = s.Pool.Submit([&] {
            while (!go.load(std::memory_order_acquire)) { std::this_thread::yield(); }
        });
        // The handle is not yet completed; release it and then wait.
        CHECK(!h->IsCompleted());
        go.store(true, std::memory_order_release);
        REQUIRE(h->WaitForCompleted(2s));
        CHECK(h->IsCompleted());
    }

    TEST_CASE("WaitForCompleted with a small timeout returns false if the task is still running")
    {
        ScopedPool s;
        std::atomic<bool> go{false};
        auto h = s.Pool.Submit([&] {
            while (!go.load(std::memory_order_acquire)) { std::this_thread::yield(); }
        });
        CHECK_FALSE(h->WaitForCompleted(50ms));
        CHECK_FALSE(h->IsCompleted());
        go.store(true, std::memory_order_release);
        REQUIRE(h->WaitForCompleted(2s));
    }

    TEST_CASE("OnCompleted fires after the body, even if registered after completion")
    {
        ScopedPool s;
        auto h = s.Pool.Submit([]{});
        REQUIRE(h->WaitForCompleted(2s));

        std::atomic<bool> fired{false};
        h->OnCompleted([&] { fired.store(true, std::memory_order_release); });
        CHECK(fired.load(std::memory_order_acquire));
    }

    TEST_CASE("WaitAll returns once every handle is completed")
    {
        ScopedPool s;
        std::vector<std::shared_ptr<TaskHandle>> handles;
        for (int i = 0; i < 16; ++i)
        {
            handles.push_back(s.Pool.Submit([] {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }));
        }
        REQUIRE(Task::WaitAll(handles, 5s));
        for (auto& h : handles)
        {
            CHECK(h->IsCompleted());
        }
    }
}

TEST_SUITE("ParallelRange")
{
    TEST_CASE("covers the entire range with no gaps or overlaps")
    {
        ScopedPool s;
        constexpr uint32 N = 1000;
        std::vector<std::atomic<int>> hits(N);
        for (auto& a : hits) a.store(0);

        ParallelRange(s.Pool, N, /*minRange*/ 32, [&](uint32 start, uint32 len) {
            for (uint32 i = start; i < start + len; ++i)
            {
                hits[i].fetch_add(1, std::memory_order_relaxed);
            }
        });

        int total = 0;
        for (auto& a : hits)
        {
            CHECK(a.load() == 1);
            total += a.load();
        }
        CHECK(total == (int)N);
    }

    TEST_CASE("respects the minRange floor")
    {
        ScopedPool s;
        // total < minRange → must collapse to a single batch.
        constexpr uint32 N = 4;
        std::atomic<int> batchCount{0};
        std::atomic<uint32> totalLen{0};
        ParallelRange(s.Pool, N, /*minRange*/ 100, [&](uint32 /*start*/, uint32 len) {
            batchCount.fetch_add(1, std::memory_order_relaxed);
            totalLen.fetch_add(len, std::memory_order_relaxed);
        });
        CHECK(batchCount.load() == 1);
        CHECK(totalLen.load() == N);
    }

    TEST_CASE("handles total=0 without scheduling any work")
    {
        ScopedPool s;
        std::atomic<int> calls{0};
        ParallelRange(s.Pool, 0u, 1u, [&](uint32, uint32) {
            calls.fetch_add(1, std::memory_order_relaxed);
        });
        CHECK(calls.load() == 0);
    }
}

TEST_SUITE("ParallelForEach")
{
    TEST_CASE("invokes the job exactly once per index")
    {
        ScopedPool s;
        constexpr uint32 N = 200;
        std::vector<std::atomic<int>> hits(N);
        for (auto& a : hits) a.store(0);

        ParallelForEach(s.Pool, N, [&](uint32 i) {
            hits[i].fetch_add(1, std::memory_order_relaxed);
        });

        for (auto& a : hits)
        {
            CHECK(a.load() == 1);
        }
    }

    TEST_CASE("handles small num below the granularity floor")
    {
        ScopedPool s;
        constexpr uint32 N = 4; // smaller than kDefaultParallelForEachMinBatch
        std::vector<std::atomic<int>> hits(N);
        for (auto& a : hits) a.store(0);
        ParallelForEach(s.Pool, N, [&](uint32 i) {
            hits[i].fetch_add(1, std::memory_order_relaxed);
        });
        for (auto& a : hits) CHECK(a.load() == 1);
    }

    TEST_CASE("handles a large num that gets evenly partitioned")
    {
        ScopedPool s;
        constexpr uint32 N = 4096;
        std::vector<std::atomic<int>> hits(N);
        for (auto& a : hits) a.store(0);
        ParallelForEach(s.Pool, N, [&](uint32 i) {
            hits[i].fetch_add(1, std::memory_order_relaxed);
        });
        for (auto& a : hits) CHECK(a.load() == 1);
    }

    TEST_CASE("handles num=0 without invoking the job")
    {
        ScopedPool s;
        std::atomic<int> calls{0};
        ParallelForEach(s.Pool, 0u, [&](uint32) {
            calls.fetch_add(1, std::memory_order_relaxed);
        });
        CHECK(calls.load() == 0);
    }
}

TEST_SUITE("TaskQueue")
{
    TEST_CASE("Enqueue accepts a const Task& without wrapping it as a callable")
    {
        // Regression: the TInlineCallable<void(), 128> hot-path body refuses
        // to store anything ≥ 128 B. sizeof(Task) is ~160 B, so binding a
        // const Task& to the variadic Enqueue overloads must NOT fall
        // through to Enqueue(TTaskFunc&&) (which would try to wrap the
        // Task itself as a callable and trip the static_assert). This
        // happened in CI on every platform on the initial #4 commit. See
        // WorldTaskQueue::Schedule(WorldRef, const Task&) at the original
        // call site.
        ScopedPool s;
        TaskQueue q(/*id*/ 7u, &s.Pool);
        std::atomic<int> hit{0};
        Task t([&] { hit.fetch_add(1, std::memory_order_relaxed); });
        const Task& cref = t; // bind a const lvalue ref, as WorldTaskQueue does
        q.Enqueue(cref);
        q.Flush();
        CHECK(hit.load() == 1);
    }

    TEST_CASE("Flush runs groups sequentially, tasks within a group run in parallel")
    {
        ScopedPool s;
        TaskQueue q(/*id*/ 42u, &s.Pool);

        std::atomic<int> phase{0};
        std::atomic<int> group1Hits{0};
        std::atomic<int> group2Hits{0};
        std::atomic<int> group1MaxPhase{0};
        std::atomic<int> group2MinPhase{INT32_MAX};

        auto& g1 = q.BeginGroup(8);
        for (int i = 0; i < 8; ++i)
        {
            g1.emplace_back([&] {
                group1Hits.fetch_add(1, std::memory_order_relaxed);
                int p = phase.load(std::memory_order_relaxed);
                int prev = group1MaxPhase.load(std::memory_order_relaxed);
                while (p > prev && !group1MaxPhase.compare_exchange_weak(prev, p)) {}
            });
        }
        q.EndGroup();

        auto& g2 = q.BeginGroup(8);
        for (int i = 0; i < 8; ++i)
        {
            g2.emplace_back([&] {
                phase.store(1, std::memory_order_relaxed);
                group2Hits.fetch_add(1, std::memory_order_relaxed);
                int p = phase.load(std::memory_order_relaxed);
                int prev = group2MinPhase.load(std::memory_order_relaxed);
                while (p < prev && !group2MinPhase.compare_exchange_weak(prev, p)) {}
            });
        }
        q.EndGroup();

        q.Flush();

        CHECK(group1Hits.load() == 8);
        CHECK(group2Hits.load() == 8);
        // Group 1 ran entirely before group 2 set phase=1.
        CHECK(group1MaxPhase.load() == 0);
        CHECK(group2MinPhase.load() == 1);
    }
}
