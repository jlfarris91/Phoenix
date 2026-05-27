
// Phoenix Job System Benchmark
//
// Measures four properties of the ThreadPool + TaskQueue:
//   1. flush_overhead              — cost of an empty Flush (pure infrastructure)
//   2. barrier_latency             — gap between consecutive single-task groups
//   3. saturated_barrier_latency   — same gap when all workers are busy per group
//   4. idle_wake_latency           — time to pick up work after workers have yielded
//   5. parallel_efficiency         — actual throughput vs ideal for uniform parallel work
//
// Each benchmark is run for both TaskPoolExecutor and EnkiTSExecutor so the two
// backends can be compared side by side. Benchmark 4 (idle wake) is ThreadPool-only
// because it relies on TaskHandle/WaitForCompleted which are ThreadPool-specific.
//
// Each benchmark reports avg/p99/max and PASS/FAIL against a threshold.
// Writes Tracy stream + bench.db for visual inspection.
// Exit code: 0 = all passed, 1 = one or more failed.

#include <../../src/Phoenix/Profiling/CompositeProfiler.h>
#include <Phoenix.Parallel/ParallelExecutor.h>
#include <Phoenix/Platform.h>
#include <../../src/Phoenix/Profiling/Profiling.h>
#include <Phoenix.Parallel/TaskQueue.h>
#include <Phoenix.Parallel/TaskPoolExecutor.h>
#include <Phoenix.Parallel/NullExecutor.h>
#include <Phoenix.Parallel.enkiTS/EnkiTSExecutor.h>
#include <Phoenix.Profilers.Structured/StructuredProfiler.h>
#include <Phoenix.Profilers.Tracy/TracyProfiler.h>

#include <sqlite3.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdio>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#if _WIN32
#include <intrin.h>
#endif

using namespace Phoenix;
using Clock = std::chrono::steady_clock;

static int64_t NowNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now().time_since_epoch()).count();
}

// ── RDTSC busy-wait (avoids QPC contention with 10+ threads) ─────────────────

#if _WIN32
static uint64_t g_rdtscPerNs = 3;

static void CalibrateRDTSC()
{
    int64_t  t0 = NowNs();
    uint64_t r0 = __rdtsc();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int64_t  t1 = NowNs();
    uint64_t r1 = __rdtsc();
    int64_t  elapsed = t1 - t0;
    g_rdtscPerNs = elapsed > 0 ? (r1 - r0 + elapsed / 2) / elapsed : 3;
    printf("  RDTSC: %llu ticks/ns (%.2f GHz)\n",
           (unsigned long long)g_rdtscPerNs, (double)g_rdtscPerNs);
}

// Use NowNs() (QPC) for the loop condition — not RDTSC. Under full load all
// 12 threads spin simultaneously, causing CPU frequency to drop (P-state
// throttling). RDTSC calibration (done at idle boost) becomes stale and
// BusyWaitNs runs for 2× the intended duration. QPC is wall-clock accurate
// regardless of CPU frequency changes.
static void BusyWaitNs(int64_t ns)
{
    int64_t end = NowNs() + ns;
    while (NowNs() < end) { PHX_THREAD_PAUSE(); }
}
#else
static void CalibrateRDTSC() {}
static void BusyWaitNs(int64_t ns)
{
    int64_t end = NowNs() + ns;
    while (NowNs() < end) { PHX_THREAD_PAUSE(); }
}
#endif

// ── Timing sample + stats ─────────────────────────────────────────────────────

struct Sample { std::vector<int64_t> ns; };

static void Push(Sample& s, int64_t v) { s.ns.push_back(v); }

struct Stats { double avg_us = 0, p99_us = 0, max_us = 0; };

static Stats Compute(Sample s)
{
    if (s.ns.empty()) return {};
    std::sort(s.ns.begin(), s.ns.end());
    int64_t sum = std::accumulate(s.ns.begin(), s.ns.end(), int64_t(0));
    Stats r;
    r.avg_us = double(sum) / double(s.ns.size()) / 1e3;
    r.p99_us = double(s.ns[s.ns.size() * 99 / 100]) / 1e3;
    r.max_us = double(s.ns.back()) / 1e3;
    return r;
}

// ── Result reporting ──────────────────────────────────────────────────────────

static bool gAllPass = true;

static bool Report(const char* name, Stats s, double threshold_us)
{
    bool pass = s.p99_us <= threshold_us;
    gAllPass &= pass;
    printf("  [%s] %-38s  avg=%7.1f us  p99=%8.1f us  max=%8.1f us  (p99 < %.0f us)\n",
           pass ? "PASS" : "FAIL", name,
           s.avg_us, s.p99_us, s.max_us, threshold_us);
    return pass;
}

static void ReportEfficiency(const char* name, double pct, double threshold_pct)
{
    bool pass = pct >= threshold_pct;
    gAllPass &= pass;
    printf("  [%s] %-38s  efficiency=%5.1f%%  (threshold > %.0f%%)\n",
           pass ? "PASS" : "FAIL", name, pct, threshold_pct);
}

// ── Benchmark 1: empty Flush overhead ─────────────────────────────────────────
//
// Measures the fixed cost of TaskQueue::Flush() with no tasks enqueued.
// Baseline for infrastructure overhead at each physics group boundary.

static Sample BenchFlushOverhead(IParallelExecutor& executor, uint32 queueId)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.flush_overhead");
    constexpr int kWarmup = 500, kIter = 5000;
    TaskQueue q(queueId, &executor);
    Sample s; s.ns.reserve(kIter);
    for (int i = 0; i < kWarmup + kIter; ++i)
    {
        int64_t t0 = NowNs(); q.Flush(); int64_t t1 = NowNs();
        if (i >= kWarmup) Push(s, t1 - t0);
    }
    return s;
}

// ── Benchmark 2: single-task inter-group barrier latency ──────────────────────
//
// 80 sequential groups of 1 task each (mirrors physics NumSeparationSteps×2).
// Measures: task[g].end → task[g+1].start  (true end-to-end barrier gap).
// This is the main target of the SpinBackoff(200'000) fix.

static Sample BenchBarrierLatency(IParallelExecutor& executor, uint32 queueId)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.barrier_latency");
    constexpr int kGroups = 80, kRuns = 20, kWarmup = 3;
    TaskQueue q(queueId, &executor);
    Sample s; s.ns.reserve(kGroups * kRuns);
    std::vector<int64_t> t_start(kGroups), t_end(kGroups);

    auto runOnce = [&]
    {
        for (int g = 0; g < kGroups; ++g)
        {
            auto& grp = q.BeginGroup(1);
            grp.emplace_back([&, g] { t_start[g] = NowNs(); t_end[g] = NowNs(); });
            q.EndGroup();
        }
        q.Flush();
    };

    for (int r = 0; r < kWarmup + kRuns; ++r)
    {
        runOnce();
        if (r >= kWarmup)
            for (int g = 0; g + 1 < kGroups; ++g)
                Push(s, t_start[g + 1] - t_end[g]);
    }
    return s;
}

// ── Benchmark 3: saturated inter-group barrier latency ────────────────────────
//
// Same as above but each group fills all W workers with 50µs of real work.
// Barrier = MAX(end of group g across all workers) → MIN(start of group g+1).
// Tests that SpinBackoff keeps the sim thread hot even when tasks vary slightly.

static Sample BenchSaturatedBarrierLatency(IParallelExecutor& executor, uint32 queueId)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.saturated_barrier_latency");
    constexpr int kGroups = 40, kRuns = 20, kWarmup = 3;
    constexpr int64_t kTaskNs = 50'000; // 50 µs per task

    const int W = (int)executor.GetNumWorkers();
    TaskQueue q(queueId, &executor);
    Sample s; s.ns.reserve(kGroups * kRuns);

    // Store raw RDTSC ticks — no QPC serialization across workers.
    // RDTSC is per-core but invariant-TSC CPUs keep cores synchronized to <10ns.
    std::vector<std::atomic<uint64_t>> grp_last_end(kGroups);
    std::vector<std::atomic<uint64_t>> grp_first_start(kGroups);

    auto runOnce = [&]
    {
        for (int g = 0; g < kGroups; ++g)
        {
            grp_last_end[g].store(0, std::memory_order_relaxed);
            grp_first_start[g].store(UINT64_MAX, std::memory_order_relaxed);
        }

        for (int g = 0; g < kGroups; ++g)
        {
            auto& grp = q.BeginGroup(W);
            for (int w = 0; w < W; ++w)
            {
                grp.emplace_back([&, g]
                {
                    uint64_t ts = __rdtsc();
                    uint64_t prev = grp_first_start[g].load(std::memory_order_relaxed);
                    while (ts < prev &&
                           !grp_first_start[g].compare_exchange_weak(
                               prev, ts, std::memory_order_relaxed)) {}

                    BusyWaitNs(kTaskNs);

                    uint64_t te = __rdtsc();
                    prev = grp_last_end[g].load(std::memory_order_relaxed);
                    while (te > prev &&
                           !grp_last_end[g].compare_exchange_weak(
                               prev, te, std::memory_order_relaxed)) {}
                });
            }
            q.EndGroup();
        }
        q.Flush();
    };

    for (int r = 0; r < kWarmup + kRuns; ++r)
    {
        runOnce();
        if (r >= kWarmup)
            for (int g = 0; g + 1 < kGroups; ++g)
            {
                // Signed cast before subtracting: invariant TSC skew between cores
                // can make grp_first_start[g+1] appear slightly < grp_last_end[g].
                // Clamping negative readings to 0 avoids uint64 wraparound.
                int64_t gap_ticks =
                    (int64_t)grp_first_start[g + 1].load() -
                    (int64_t)grp_last_end[g].load();
                Push(s, gap_ticks > 0 ? gap_ticks / (int64_t)g_rdtscPerNs : 0);
            }
    }
    return s;
}

// ── Benchmark 4: idle-wake latency ────────────────────────────────────────────
//
// Sleeps 300ms (> SpinBackoff(200'000) ~128ms spin window) so workers yield,
// then submits 1 task and measures submit → task_start latency.
// Worst-case latency when the sim goes idle between frames.
// ThreadPool-only: uses TaskHandle/WaitForCompleted.

static Sample BenchIdleWakeLatency(ThreadPool& pool)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.idle_wake_latency");
    constexpr int kIter = 15, kSleepMs = 300;
    Sample s; s.ns.reserve(kIter);
    std::atomic<int64_t> taskStart{0};

    for (int i = 0; i < kIter; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMs));
        int64_t tSubmit = NowNs();
        auto h = pool.Submit([&] { taskStart.store(NowNs(), std::memory_order_relaxed); });
        h->WaitForCompleted(std::chrono::seconds(10));
        Push(s, taskStart.load() - tSubmit);
    }
    return s;
}

// ── Benchmark 5: parallel throughput efficiency ───────────────────────────────
//
// Each run: one group of exactly W tasks × 2ms each. If all workers run in
// true parallel the wall time equals task_time; any overhead degrades
// efficiency. Uses W tasks (not W×N) so Flush's completion scan is O(W),
// matching real ECS usage (one task per worker per physics sub-step).
//
// Efficiency = task_time / wall_per_run. Below ~90% = scheduling overhead,
// worker idle gap, or false sharing in the dispatch path.
//
// When pool != nullptr, an additional diagnostic submit+wait run is printed
// that reveals per-task submit and start-time offsets (ThreadPool-only).

static double BenchParallelEfficiency(IParallelExecutor& executor, uint32 queueId,
                                      ThreadPool* pool = nullptr)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.parallel_efficiency");
    const int W = (int)executor.GetNumWorkers();
    constexpr int kRuns = 20, kWarmup = 3;
    constexpr int64_t kTaskNs = 2'000'000; // 2 ms per task

    TaskQueue q(queueId, &executor);
    Sample wallSamples; wallSamples.ns.reserve(kRuns);

    auto runOnce = [&]
    {
        auto& grp = q.BeginGroup(W);
        for (int i = 0; i < W; ++i)
            grp.emplace_back([&] { BusyWaitNs(kTaskNs); });
        q.EndGroup();
        int64_t t0 = NowNs();
        q.Flush();
        return NowNs() - t0;
    };

    // Verify BusyWaitNs accuracy on the calling thread before measuring workers.
    {
        int64_t ta = NowNs();
        BusyWaitNs(kTaskNs);
        int64_t tb = NowNs();
        printf("  BusyWaitNs(%lld ms) single-thread: %.1f us\n", kTaskNs / 1'000'000, (tb - ta) / 1e3);
    }

    // Verify worker-thread BusyWaitNs by submitting one task and timing it.
    {
        std::atomic<int64_t> workerDuration{0};
        auto& grp0 = q.BeginGroup(1);
        grp0.emplace_back([&]
        {
            int64_t ta = NowNs();
            BusyWaitNs(kTaskNs);
            workerDuration.store(NowNs() - ta, std::memory_order_relaxed);
        });
        q.EndGroup();
        q.Flush();
        printf("  BusyWaitNs(%lld ms) single-worker: %.1f us\n", kTaskNs / 1'000'000, workerDuration.load() / 1e3);
    }

    // ThreadPool-only diagnostic: show per-task submit and start-time offsets.
    if (pool != nullptr)
    {
        std::vector<std::atomic<int64_t>> startTimes(W);
        std::vector<int64_t> submitTimes(W, 0);

        int64_t diagT0 = NowNs();
        std::vector<std::shared_ptr<TaskHandle>> diagHandles;
        diagHandles.reserve(W);
        for (int i = 0; i < W; ++i)
        {
            Task t([&startTimes, i, kTaskNs]
            {
                startTimes[i].store(NowNs(), std::memory_order_relaxed);
                BusyWaitNs(kTaskNs);
            });
            diagHandles.push_back(pool->Submit(t));
            submitTimes[i] = NowNs() - diagT0;
        }
        Task::WaitAll(diagHandles);
        int64_t diagWall = NowNs() - diagT0;

        printf("  Submit offsets (us):");
        for (int i = 0; i < W; ++i) printf(" %.0f", submitTimes[i] / 1e3);
        printf("\n");
        printf("  Start  offsets (us), wall=%.1fus:", diagWall / 1e3);
        for (int i = 0; i < W; ++i)
            printf(" %.0f", (startTimes[i].load() - diagT0) / 1e3);
        printf("\n");
    }

    for (int r = 0; r < kWarmup + kRuns; ++r)
    {
        int64_t wall = runOnce();
        if (r >= kWarmup)
        {
            Push(wallSamples, wall);
            printf("  run %2d: wall=%8.1f us  eff=%5.1f%%\n",
                   r - kWarmup, wall / 1e3,
                   double(kTaskNs / 1000) / (wall / 1e3) * 100.0);
        }
    }

    Stats ws = Compute(wallSamples);

    printf("  wall: avg=%7.1f us  p99=%8.1f us  max=%8.1f us\n",
           ws.avg_us, ws.p99_us, ws.max_us);

    double avg_eff = double(kTaskNs / 1000) / ws.avg_us * 100.0;
    return avg_eff;
}

// ── Benchmark 6: imbalanced range — undersubscribed vs oversubscribed ─────────
//
// Models the physics OverlapSeparationTask scenario: kTotal items where the first
// kHotEnd are "hot" (expensive contact pairs in a dense spatial cluster) and the
// rest are "cold". Two chunking strategies compared:
//
//   Undersubscribed: W chunks (old WorldTaskQueue behavior) — one chunk covers
//     the entire hot zone; that worker becomes the straggler and holds the group
//     barrier while all other workers are idle.
//
//   Oversubscribed:  kTotal/128 chunks (new behavior) — hot zone is split across
//     multiple chunks processed concurrently; workers steal cold chunks while
//     hot-zone workers finish.
//
// Reports wall time, critical-path ideal, and straggler overhead for each
// strategy. The speedup ratio quantifies the improvement from oversubscription.
// No PASS/FAIL — the expected speedup depends on worker count and chunk geometry.

static void BenchImbalancedRange(IParallelExecutor& executor, uint32 baseQueueId)
{
    PHX_PROFILE_ZONE_SCOPED_N("bench.imbalanced_range");

    constexpr uint32  kTotal   = 4096;
    constexpr uint32  kHotEnd  = 512;    // items [0, kHotEnd) are "hot"
    constexpr int64_t kHotNs   = 200;    // ns per hot item
    constexpr int64_t kColdNs  = 10;     // ns per cold item
    constexpr int kRuns = 15, kWarmup = 3;

    const int W = (int)executor.GetNumWorkers();

    struct Strategy { const char* label; int numChunks; };
    const Strategy strategies[] = {
        { "undersubscribed (W chunks)",          W                      },
        { "oversubscribed  (kTotal/128 chunks)", (int)(kTotal / 128)    },
    };

    double wallAvg[2] = {};

    for (int si = 0; si < 2; ++si)
    {
        const int numChunks = std::max(strategies[si].numChunks, 1);
        const uint32 chunkItems = (kTotal + (uint32)numChunks - 1) / (uint32)numChunks;

        // Precompute total work (ns) for each chunk.
        std::vector<int64_t> chunkWork(numChunks, 0);
        for (int c = 0; c < numChunks; ++c)
        {
            uint32 s = (uint32)c * chunkItems;
            uint32 e = std::min(s + chunkItems, kTotal);
            for (uint32 i = s; i < e; ++i)
                chunkWork[c] += (i < kHotEnd) ? kHotNs : kColdNs;
        }

        int64_t critPath  = *std::max_element(chunkWork.begin(), chunkWork.end());
        int64_t totalWork =  std::accumulate(chunkWork.begin(), chunkWork.end(), int64_t(0));
        // Ideal = max(critical path, work/workers). The critical path bounds us
        // when one chunk is far heavier than total/W; work/W bounds us otherwise.
        int64_t ideal = std::max(critPath, totalWork / W);

        TaskQueue q(baseQueueId + (uint32)si, &executor);
        Sample wallSamples; wallSamples.ns.reserve(kRuns);

        auto runOnce = [&]
        {
            auto& grp = q.BeginGroup((uint32)numChunks);
            for (int c = 0; c < numChunks; ++c)
            {
                int64_t work = chunkWork[c];
                grp.emplace_back([work] { BusyWaitNs(work); });
            }
            q.EndGroup();
            int64_t t0 = NowNs();
            q.Flush();
            return NowNs() - t0;
        };

        for (int r = 0; r < kWarmup + kRuns; ++r)
        {
            int64_t wall = runOnce();
            if (r >= kWarmup)
                Push(wallSamples, wall);
        }

        Stats ws = Compute(wallSamples);
        double overheadPct = std::max(0.0, (ws.avg_us - ideal / 1e3) / (ideal / 1e3) * 100.0);
        printf("  %-42s  chunks=%3d  crit=%5.1f us  ideal=%5.1f us  avg=%5.1f us  overhead=+%.0f%%\n",
               strategies[si].label, numChunks,
               critPath / 1e3, ideal / 1e3, ws.avg_us, overheadPct);
        wallAvg[si] = ws.avg_us;
    }

    if (wallAvg[1] > 0.0)
        printf("  speedup (oversubscribed vs undersubscribed): %.2fx\n", wallAvg[0] / wallAvg[1]);
}

// ── Run a full benchmark suite for one executor ───────────────────────────────
//
// When W == 0 (NullExecutor), benchmarks 3, 5, and 6 require workers and are
// skipped. Benchmarks 1 and 2 still run: both measure the serial-inline path
// that TaskQueue::Flush takes when GetNumWorkers() == 0, giving a lower-bound
// baseline with zero cross-thread synchronization.

static void RunSuite(const char* label, IParallelExecutor& executor,
                     uint32 baseQueueId, ThreadPool* pool = nullptr)
{
    const uint32 W = executor.GetNumWorkers();
    printf("\n========== %s (%u workers) ==========\n", label, W);

    printf("\n--- [%s] Flush Overhead (empty Flush, 5000 iterations) ---\n", label);
    Report((std::string(label) + "/flush_overhead").c_str(),
           Compute(BenchFlushOverhead(executor, baseQueueId + 0)), 25.0);

    printf("\n--- [%s] Barrier Latency (80 single-task groups x 20 runs) ---\n", label);
    Report((std::string(label) + "/barrier_latency").c_str(),
           Compute(BenchBarrierLatency(executor, baseQueueId + 1)), 20.0);

    if (W > 0)
    {
        printf("\n--- [%s] Saturated Barrier Latency (40 groups x %u workers x 50us, 20 runs) ---\n",
               label, W);
        Report((std::string(label) + "/saturated_barrier_latency").c_str(),
               Compute(BenchSaturatedBarrierLatency(executor, baseQueueId + 2)), 100.0);

        printf("\n--- [%s] Parallel Efficiency (%u workers x 2ms tasks, 20 runs) ---\n", label, W);
        ReportEfficiency((std::string(label) + "/parallel_efficiency").c_str(),
                         BenchParallelEfficiency(executor, baseQueueId + 3, pool), 80.0);

        printf("\n--- [%s] Imbalanced Range (undersubscribed W vs oversubscribed kTotal/128 chunks) ---\n",
               label);
        printf("  Model: 4096 items, first 512 hot (200ns/item), rest cold (10ns/item)\n");
        BenchImbalancedRange(executor, baseQueueId + 4);
    }
    else
    {
        printf("\n  [--] Saturated Barrier Latency — skipped (no workers)\n");
        printf("  [--] Parallel Efficiency        — skipped (no workers)\n");
        printf("  [--] Imbalanced Range           — skipped (no workers)\n");
    }
}

// ── Post-run zone stats from bench.db ─────────────────────────────────────────

static void PrintZoneStats(const char* dbPath)
{
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
    {
        printf("  (could not open %s)\n", dbPath);
        return;
    }

    const char* sql = R"sql(
        SELECT
            COALESCE(custom_name, src_name, src_func) AS name,
            COUNT(*)                                   AS n,
            AVG(duration_ns) / 1000.0                 AS avg_us,
            MAX(duration_ns) / 1000.0                 AS max_us
        FROM zones
        GROUP BY name
        ORDER BY AVG(duration_ns) DESC
        LIMIT 30;
    )sql";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    { sqlite3_close(db); return; }

    printf("  %-55s  %7s  %10s  %10s\n", "zone", "calls", "avg (us)", "max (us)");
    printf("  %s\n", std::string(90, '-').c_str());
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* name   = (const char*)sqlite3_column_text(stmt, 0);
        int         n      = sqlite3_column_int(stmt, 1);
        double      avg_us = sqlite3_column_double(stmt, 2);
        double      max_us = sqlite3_column_double(stmt, 3);
        printf("  %-55s  %7d  %10.2f  %10.2f\n",
               name ? name : "?", n, avg_us, max_us);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    static Profiling::TracyProfiler      sTracy;
    static Profiling::StructuredProfiler sStructured("bench.db");
    static Profiling::CompositeProfiler  sProfiler({ &sTracy, &sStructured });
    Profiling::SetProfiler(&sProfiler);

    PHX_PROFILE_SET_THREAD_NAME("Bench", 0);

    const uint32 kWorkers = std::max(1u, std::thread::hardware_concurrency() - 1u);

    printf("Phoenix Job System Benchmark\n");
    printf("Workers: %u   Hardware threads: %u\n\n", kWorkers, std::thread::hardware_concurrency());

    printf("Calibrating RDTSC...\n");
    CalibrateRDTSC();

    // ── TaskPoolExecutor suite ────────────────────────────────────────────────

    SetThreadPool("SimThreadPool", kWorkers, 4096);
    ThreadPool& pool = *GetThreadPool();
    pool.WaitIdle(std::chrono::seconds(1));

    TaskPoolExecutor poolExecutor(pool);
    RunSuite("TaskPool", poolExecutor, /*baseQueueId=*/10, &pool);  // uses IDs 10-15

    printf("\n--- [TaskPool] Idle Wake Latency (15 x 300ms sleep, then 1 task) ---\n");
    printf("  (running ~%d seconds...)\n", 15 * 300 / 1000);
    Report("TaskPool/idle_wake_latency", Compute(BenchIdleWakeLatency(pool)), 5000.0);

    DestroyThreadPool();

    // ── EnkiTSExecutor suite ──────────────────────────────────────────────────

    EnkiTSExecutor enkiExecutor(kWorkers);
    // Register this (main/bench) thread so Submit → AddTaskSetToPipe works.
    enkiExecutor.RegisterExternalThread();

    RunSuite("EnkiTS", enkiExecutor, /*baseQueueId=*/20);           // uses IDs 20-25

    enkiExecutor.DeregisterExternalThread();

    // ── NullExecutor suite (serial baseline) ──────────────────────────────────
    // Benchmarks 3 and 5 are skipped automatically (W == 0).
    // Results show the cost floor of TaskQueue::Flush with zero threading overhead.

    NullExecutor nullExecutor;
    RunSuite("Null (serial)", nullExecutor, /*baseQueueId=*/30);    // uses IDs 30-31

    sStructured.Close();

    printf("\n--- Zone Stats (bench.db) ---\n");
    PrintZoneStats("bench.db");

    printf("\n=== %s ===\n", gAllPass ? "ALL PASSED" : "SOME FAILED");
    return gAllPass ? 0 : 1;
}
