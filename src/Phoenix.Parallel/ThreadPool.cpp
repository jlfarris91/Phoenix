#include "ThreadPool.h"

#include <random>

#include "Phoenix/SpinBackoff.h"
#include "TaskHandle.h"
#include "Phoenix/ParallelExecutor.h"
#include "Phoenix/Profiling.h"

using namespace Phoenix;

namespace
{
    std::size_t NextPow2(std::size_t v)
    {
        std::size_t p = 1;
        while (p < v) p <<= 1;
        return p;
    }
}

ThreadPool::ThreadPool(std::string id, uint32 numWorkers, uint32 queueCapacity)
    : Id(std::move(id))
    , NumWorkers(numWorkers)
    , SlotCount(queueCapacity)
    , SubmissionInbox(NextPow2(queueCapacity))
{
    PHX_ASSERT(numWorkers > 0);
    PHX_ASSERT(SlotCount > 0);

    Slots = std::make_unique<TaskSlot[]>(SlotCount);
    for (std::uint32_t i = 0; i < SlotCount; ++i)
    {
        Slots[i].NextFree.store(
            (i + 1 < SlotCount) ? (i + 1) : kInvalidSlot,
            std::memory_order_relaxed);
    }
    SlabFreeHead.store(0ULL, std::memory_order_relaxed);

    WorkerDeques =
        std::make_unique<TChaseLevDeque<Task*, kThreadPoolDequeCapacity>[]>(NumWorkers);

    Threads.reserve(NumWorkers);
    for (uint32 i = 0; i < NumWorkers; ++i)
    {
        Threads.emplace_back([this, i] { Worker(i); });
    }
}

ThreadPool::~ThreadPool()
{
    Shutdown();
}

Task* ThreadPool::SlabAllocate()
{
    std::uint64_t head = SlabFreeHead.load(std::memory_order_acquire);
    while (true)
    {
        const std::uint32_t idx = static_cast<std::uint32_t>(head & 0xFFFFFFFFu);
        if (idx == kInvalidSlot)
        {
            return nullptr;
        }
        const std::uint32_t nextIdx =
            Slots[idx].NextFree.load(std::memory_order_relaxed);
        const std::uint64_t newHead =
            ((head >> 32) + 1ULL) << 32 | static_cast<std::uint64_t>(nextIdx);
        if (SlabFreeHead.compare_exchange_weak(
                head, newHead,
                std::memory_order_acquire,
                std::memory_order_acquire))
        {
            return &Slots[idx].Body;
        }
    }
}

void ThreadPool::SlabFree(Task* task)
{
    const std::uint32_t idx = SlotIndex(task);
    std::uint64_t head = SlabFreeHead.load(std::memory_order_relaxed);
    while (true)
    {
        const std::uint32_t curTop = static_cast<std::uint32_t>(head & 0xFFFFFFFFu);
        Slots[idx].NextFree.store(curTop, std::memory_order_relaxed);
        const std::uint64_t newHead =
            ((head >> 32) + 1ULL) << 32 | static_cast<std::uint64_t>(idx);
        if (SlabFreeHead.compare_exchange_weak(
                head, newHead,
                std::memory_order_release,
                std::memory_order_relaxed))
        {
            return;
        }
    }
}

std::uint32_t ThreadPool::SlotIndex(const Task* task) const
{
    const auto* base = reinterpret_cast<const std::byte*>(&Slots[0]);
    const auto* p    = reinterpret_cast<const std::byte*>(task);
    return static_cast<std::uint32_t>((p - base) / sizeof(TaskSlot));
}

uint32 ThreadPool::GetNumWorkers() const
{
    return NumWorkers;
}

void ThreadPool::BeginBusyPeriod()
{
    const uint32_t prev = BusyPeriod.fetch_add(1, std::memory_order_seq_cst);
    if (prev == 0)
    {
        // Transition from idle to busy: wake any workers that fell asleep between
        // task waves so they are already spinning when the first task arrives.
        WakeNotifyCount.fetch_add(1, std::memory_order_seq_cst);
        WakeCV.notify_all();
    }
}

void ThreadPool::EndBusyPeriod()
{
    BusyPeriod.fetch_sub(1, std::memory_order_release);
}

void ThreadPool::Shutdown()
{
    bool expected = false;
    if (Done.compare_exchange_strong(expected, true))
    {
        WakeCV.notify_all();
        for (std::thread& thread : Threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }
}

std::shared_ptr<TaskHandle> ThreadPool::Submit(const Task& task)
{
    auto handle = std::make_shared<TaskHandle>();

    Task* slotTask = nullptr;
    {
        SpinBackoff backoff;
        while ((slotTask = SlabAllocate()) == nullptr)
        {
            backoff.Tick();
        }
    }

    *slotTask = task;
    slotTask->Handle = handle;

    InFlight.fetch_add(1, std::memory_order_release);

    const uint32 currentIdx = GetCurrentThreadIndex();
    if (currentIdx > 0 && currentIdx <= NumWorkers)
    {
        const std::uint32_t ownIdx = currentIdx - 1;
        SpinBackoff backoff;
        while (!WorkerDeques[ownIdx].Push(slotTask))
        {
            backoff.Tick();
        }
    }
    else
    {
        SpinBackoff backoff;
        while (!SubmissionInbox.TryEnqueue(slotTask))
        {
            backoff.Tick();
        }
    }

    WakeNotifyCount.fetch_add(1, std::memory_order_seq_cst);
    if (SleepingWorkerCount.load(std::memory_order_seq_cst) > 0)
        WakeCV.notify_one();

    return handle;
}

std::shared_ptr<TaskHandle> ThreadPool::Submit(TTaskFunc&& work)
{
    return Submit(Task(std::move(work)));
}

bool ThreadPool::IsEmpty() const
{
    if (!SubmissionInbox.IsEmpty()) return false;
    for (uint32 i = 0; i < NumWorkers; ++i)
    {
        if (!WorkerDeques[i].IsEmptyApprox())
        {
            return false;
        }
    }
    return true;
}

bool ThreadPool::WaitIdle(std::chrono::milliseconds maxWaitTime) const
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;
    while (InFlight.load(std::memory_order_acquire) != 0)
    {
        backoff.Tick();
        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }
    }
    return true;
}

void ThreadPool::Worker(uint32 workerId)
{
    SetCurrentThreadIndex(workerId + 1);

#if _WIN32
    wchar_t buf[256];
    size_t len;
    (void)mbstowcs_s(&len, buf, Id.c_str(), 256);
    SetThreadDescription(GetCurrentThread(), buf);
#endif

    PHX_PROFILE_SET_THREAD_NAME(Id.c_str(), (int32)workerId);

    auto& myDeque = WorkerDeques[workerId];

    std::mt19937 rng{static_cast<std::uint32_t>(workerId) * 0x9E3779B9u + 1u};

    // Number of PAUSE-based SpinBackoff ticks before a worker sleeps on the
    // semaphore. At 128 PAUSE instructions per tick this is ~1ms on most
    // modern x86 hardware; tunable without recompiling by changing this value.
    static constexpr uint32 kSpinBeforeSleep = 1'000;

    auto runOne = [&](Task* slotTask)
    {
        ActiveWorkerCount.fetch_add(1, std::memory_order_acq_rel);
        (*slotTask)();
        *slotTask = Task();
        ActiveWorkerCount.fetch_sub(1, std::memory_order_acq_rel);
        SlabFree(slotTask);
        InFlight.fetch_sub(1, std::memory_order_acq_rel);
    };

    auto tryGetWork = [&]() -> Task*
    {
        Task* t = nullptr;
        if (myDeque.PopOwner(t))
            return t;
        if (SubmissionInbox.TryDequeue(t))
            return t;
        if (NumWorkers <= 1) return nullptr;
        const uint32 attempts = NumWorkers * 2;
        std::uniform_int_distribution<std::uint32_t> pick(0, NumWorkers - 1);
        for (uint32 i = 0; i < attempts; ++i)
        {
            const std::uint32_t v = pick(rng);
            if (v == workerId) continue;
            if (WorkerDeques[v].TrySteal(t))
                return t;
        }
        return nullptr;
    };

    while (!Done.load(std::memory_order_acquire))
    {
        // Fast path: just finished a task or just woke up — check immediately.
        if (Task* t = tryGetWork())
        {
            runOne(t);
            continue;
        }

        // Spin phase: stay hot for a brief window to catch burst work without
        // a syscall. Exits as soon as work is found or after kSpinBeforeSleep
        // ticks, whichever comes first.
        bool foundInSpin = false;
        {
            SpinBackoff backoff(kSpinBeforeSleep);
            while (!Done.load(std::memory_order_acquire) &&
                   backoff.Attempts < backoff.PauseAttempts)
            {
                if (Task* t = tryGetWork())
                {
                    runOne(t);
                    foundInSpin = true;
                    break;
                }
                backoff.Tick();
            }
        }
        if (foundInSpin)
            continue;
        if (Done.load(std::memory_order_acquire))
            break;

        // During a busy period workers skip sleep entirely and loop back to
        // the fast path, keeping the spin alive without an OS wakeup cost.
        if (BusyPeriod.load(std::memory_order_acquire) > 0)
            continue;

        // Sleep phase: sample WakeNotifyCount before the final tryGetWork() so
        // any Submit() that arrives between the spin-end and wait() is visible
        // via the predicate — preventing a lost wakeup.
        SleepingWorkerCount.fetch_add(1, std::memory_order_relaxed);
        const uint64_t expected = WakeNotifyCount.load(std::memory_order_seq_cst);
        Task* wokeWithTask = nullptr;
        {
            PHX_PROFILE_ZONE_SCOPED_N("WorkerSleep");
            if (Task* t = tryGetWork())
            {
                SleepingWorkerCount.fetch_sub(1, std::memory_order_relaxed);
                wokeWithTask = t;
            }
            else
            {
                std::unique_lock<std::mutex> lk(WakeMutex);
                WakeCV.wait(lk, [&] {
                    return Done.load(std::memory_order_acquire) ||
                           WakeNotifyCount.load(std::memory_order_seq_cst) != expected ||
                           BusyPeriod.load(std::memory_order_acquire) > 0;
                });
                SleepingWorkerCount.fetch_sub(1, std::memory_order_relaxed);
            }
        }
        if (wokeWithTask)
            runOne(wokeWithTask);
    }

    // Drain any tasks that arrived after Done was set.
    while (Task* t = tryGetWork())
        runOne(t);
}