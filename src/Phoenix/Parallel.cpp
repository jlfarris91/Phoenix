#include "Phoenix/Parallel.h"

// xatomic.h is Windows-specific, using standard <atomic> and <thread> from Parallel.h

#include <random>

#include "Platform.h"
#include "Phoenix/Profiling.h"

using namespace Phoenix;

thread_local uint32 gCurrentThreadIndex = 0;

uint32 Phoenix::GetCurrentThreadIndex()
{
    return gCurrentThreadIndex;
}

namespace
{
    // Adaptive caller-side spin: emit a doubling burst of PAUSE/YIELD
    // instructions for the first few attempts, then fall back to
    // std::this_thread::yield(). The PAUSE phase keeps wake-up latency in the
    // nanosecond range on short waits; the yield fallback releases the core
    // when waits are long. The worker-side idle loop uses the same strategy
    // (see ThreadPool::Worker).
    struct SpinBackoff
    {
        uint32 Attempts = 0;

        void Tick()
        {
            if (Attempts < 8)
            {
                const uint32 count = 1u << Attempts;
                for (uint32 i = 0; i < count; ++i)
                {
                    PHX_THREAD_PAUSE();
                }
            }
            else
            {
                std::this_thread::yield();
            }
            ++Attempts;
        }
    };
}

bool TaskHandle::IsCompleted() const
{
    return bIsCompleted.load(std::memory_order_acquire);
}

bool TaskHandle::WaitForCompleted(std::chrono::milliseconds maxWaitTime) const
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;
    while (!IsCompleted())
    {
        backoff.Tick();
        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }
    }
    return true;
}

void TaskHandle::OnCompleted(std::function<void()>&& fn)
{
    OnCompletedFunc = std::move(fn);
    if (IsCompleted())
    {
        OnCompletedFunc();
    }
}

Task::Task() = default;

Task::Task(Task&& other) noexcept
    : WorkFunc(std::move(other.WorkFunc))
{
}

Task::Task(TTaskFunc&& work)
    : WorkFunc(std::move(work))
{
}

void Task::operator()() const
{
    Handle->bIsCompleted.store(false, std::memory_order_release);
    WorkFunc();
    Handle->bIsCompleted.store(true, std::memory_order_release);
    if (Handle->OnCompletedFunc)
    {
        Handle->OnCompletedFunc();
    }
}

bool Task::WaitAll(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime)
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;

    for (;;)
    {
        bool done = true;
        for (const std::shared_ptr<TaskHandle>& handle : handles)
        {
            if (!handle->IsCompleted())
            {
                done = false;
                break;
            }
        }

        if (done)
        {
            return true;
        }

        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }

        backoff.Tick();
    }
}

bool Task::WaitAny(const std::vector<std::shared_ptr<TaskHandle>>& handles, std::chrono::milliseconds maxWaitTime)
{
    auto startTime = PHX_SYS_CLOCK_NOW();
    SpinBackoff backoff;

    for (;;)
    {
        for (const std::shared_ptr<TaskHandle>& handle : handles)
        {
            if (handle->IsCompleted())
            {
                return true;
            }
        }

        if (maxWaitTime.count() > 0 && (PHX_SYS_CLOCK_NOW() - startTime) > maxWaitTime)
        {
            return false;
        }

        backoff.Tick();
    }
}

namespace
{
    // Pick a power-of-2 capacity for the MPMC submission inbox. It must be
    // big enough that the main thread doesn't spin on a full inbox often,
    // but small enough to bound memory. We size it to the slab count rounded
    // up to a power of 2 — there can never be more in-flight pointers than
    // slab slots anyway, so the inbox can hold every outstanding task.
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

    // Allocate the slab. Each TaskSlot starts with a default-constructed Task
    // and a NextFree pointer that links it to slot i+1 (kInvalidSlot at the
    // tail), so the initial free list is the simple chain 0 -> 1 -> ... -> end.
    Slots = std::make_unique<TaskSlot[]>(SlotCount);
    for (std::uint32_t i = 0; i < SlotCount; ++i)
    {
        Slots[i].NextFree.store(
            (i + 1 < SlotCount) ? (i + 1) : kInvalidSlot,
            std::memory_order_relaxed);
    }
    // Head: generation = 0, top index = 0.
    SlabFreeHead.store(0ULL, std::memory_order_relaxed);

    // Per-worker Chase-Lev deques.
    WorkerDeques =
        std::make_unique<TChaseLevDeque<Task*, kThreadPoolDequeCapacity>[]>(NumWorkers);

    // Spawn the workers last, so they only start running after both the slab
    // and the deques are fully initialised.
    Threads.reserve(NumWorkers);
    for (uint32 i = 0; i < NumWorkers; ++i)
    {
        Threads.emplace_back([this, i] { Worker(i); });
    }
}

ThreadPool::~ThreadPool()
{
    Shutdown();
    // Tasks held in the slab are destructed by ~TaskSlot via the unique_ptr
    // free path; nothing extra to clean up here.
}

Task* ThreadPool::SlabAllocate()
{
    std::uint64_t head = SlabFreeHead.load(std::memory_order_acquire);
    while (true)
    {
        const std::uint32_t idx = static_cast<std::uint32_t>(head & 0xFFFFFFFFu);
        if (idx == kInvalidSlot)
        {
            return nullptr; // slab exhausted
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
        // CAS failed: head was updated by another thread, retry with the
        // refreshed value already loaded into `head`.
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

void ThreadPool::Shutdown()
{
    bool expected = false;
    if (Done.compare_exchange_strong(expected, true))
    {
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

    // Acquire a slab slot. If the slab is exhausted, spin with backoff —
    // matches the historical TryEnqueue-loop behaviour.
    Task* slotTask = nullptr;
    {
        SpinBackoff backoff;
        while ((slotTask = SlabAllocate()) == nullptr)
        {
            backoff.Tick();
        }
    }

    // Copy the body and attach the handle in place.
    *slotTask = task;
    slotTask->Handle = handle;

    // Claim a slot in the in-flight count BEFORE pushing into a deque so
    // WaitIdle can't observe an interim state where the task is in transit
    // (popped from a deque but the worker hasn't yet decremented).
    InFlight.fetch_add(1, std::memory_order_release);

    // Submission routing:
    //
    //   • From a worker thread (continuation / sub-job): push to OWN deque.
    //     The owner-only invariant of Chase-Lev holds and the LIFO fast
    //     path keeps that work hot in L1.
    //
    //   • From any other thread (main thread, external integrations): push
    //     to the SubmissionInbox MPMC. Workers drain it as part of their
    //     tryGetWork rotation. We do this because Chase-Lev's Push is
    //     single-producer; cross-thread push into a worker's deque would
    //     race its Bottom store with the worker's PopOwner.
    const uint32 currentIdx = gCurrentThreadIndex;
    if (currentIdx > 0 && currentIdx <= NumWorkers)
    {
        const std::uint32_t ownIdx = currentIdx - 1;
        SpinBackoff backoff;
        // Own deque is single-producer (just us), so this only fails on
        // capacity exhaustion. Drain the inbox or steal from peers as
        // helping behaviour if it ever fills.
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

    return handle;
}

std::shared_ptr<TaskHandle> ThreadPool::Submit(TTaskFunc&& work)
{
    return Submit(Task(std::move(work)));
}

bool ThreadPool::IsEmpty() const
{
    // Approximate — racy across deques and the inbox. WaitIdle uses
    // InFlight (authoritative) instead.
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
    // Set thread-local index for worker threads (main thread is index 0)
    gCurrentThreadIndex = workerId + 1;

#if _WIN32
    wchar_t buf[256];
    size_t len;
    (void)mbstowcs_s(&len, buf, Id.c_str(), 256);
    SetThreadDescription(GetCurrentThread(), buf);
#endif

    PHX_PROFILE_SET_THREAD_NAME(Id.c_str(), (int32)workerId);

    auto& myDeque = WorkerDeques[workerId];

    // Per-thread RNG for victim selection. Seeded from the workerId so
    // different workers diverge immediately and steal patterns don't
    // converge on a single victim.
    std::mt19937 rng{static_cast<std::uint32_t>(workerId) * 0x9E3779B9u + 1u};

    auto runOne = [&](Task* slotTask)
    {
        ActiveWorkerCount.fetch_add(1, std::memory_order_acq_rel);
        (*slotTask)();
        // Reset the slot so captured state (shared_ptrs, etc) is released
        // promptly, rather than waiting for the slot to be reused by a
        // future Submit.
        *slotTask = Task();
        ActiveWorkerCount.fetch_sub(1, std::memory_order_acq_rel);
        SlabFree(slotTask);
        // Decrement only AFTER the body has fully completed so WaitIdle's
        // acquire-load of InFlight synchronises-with the body's writes.
        InFlight.fetch_sub(1, std::memory_order_acq_rel);
    };

    // tryGetWork: own deque first (LIFO, hot in L1), then the global
    // submission inbox, then steal from a random victim. Returns the task
    // pointer or nullptr if all three fail.
    auto tryGetWork = [&]() -> Task*
    {
        Task* t = nullptr;
        if (myDeque.PopOwner(t))
        {
            return t;
        }
        if (SubmissionInbox.TryDequeue(t))
        {
            return t;
        }
        // Try a few steal attempts before giving up to the spin-backoff path.
        // 2× NumWorkers attempts covers picking each peer at least once on
        // average even with a small RNG bias.
        if (NumWorkers <= 1) return nullptr;
        const uint32 attempts = NumWorkers * 2;
        std::uniform_int_distribution<std::uint32_t> pick(0, NumWorkers - 1);
        for (uint32 i = 0; i < attempts; ++i)
        {
            const std::uint32_t v = pick(rng);
            if (v == workerId) continue;
            if (WorkerDeques[v].TrySteal(t))
            {
                return t;
            }
        }
        return nullptr;
    };

    while (!Done.load(std::memory_order_acquire))
    {
        if (Task* t = tryGetWork())
        {
            runOne(t);
            continue;
        }

        SpinningWorkerCount.fetch_add(1, std::memory_order_relaxed);

        // Spin with exponential backoff. Adaptive PAUSE keeps wake latency
        // low when work shows up quickly; yield once we've spun long enough
        // to suggest the system is genuinely idle.
        uint32 spins = 0;
        while (!Done.load(std::memory_order_acquire))
        {
            if (Task* t = tryGetWork())
            {
                runOne(t);
                break;
            }
            if (spins < 8)
            {
                for (uint32 i = 0; i < (1ULL << spins); ++i)
                {
                    PHX_THREAD_PAUSE();
                }
            }
            else
            {
                std::this_thread::yield();
            }
            ++spins;
        }

        SpinningWorkerCount.fetch_sub(1, std::memory_order_relaxed);
    }

    // Drain any remaining work on shutdown so handles caller-owned tasks
    // are still completed before the pool tears down.
    while (Task* t = tryGetWork())
    {
        runOne(t);
    }
}

std::unordered_map<uint32, std::shared_ptr<TaskQueue>> gTaskQueues;
std::mutex gTaskQueueMutex;

TaskQueue::TaskQueue(uint32 id, Phoenix::ThreadPool* threadPool)
    : Id (id)
    , ThreadPool(threadPool)
{
    Tasks.reserve(32);
}

std::shared_ptr<TaskQueue> TaskQueue::CreateTaskQueue(uint32 id)
{
    std::scoped_lock lock(gTaskQueueMutex);
    auto taskQueue = std::make_shared<TaskQueue>(id);
    gTaskQueues[id] = taskQueue;
    return taskQueue;
}

std::shared_ptr<TaskQueue> TaskQueue::GetTaskQueue(uint32 id)
{
    PHX_PROFILE_ZONE_SCOPED;
    std::scoped_lock lock(gTaskQueueMutex);
    auto iter = gTaskQueues.find(id);
    if (iter != gTaskQueues.end())
    {
        return iter->second;
    }
    return nullptr;
}

bool TaskQueue::ReleaseTaskQueue(uint32 id)
{
    std::scoped_lock lock(gTaskQueueMutex);

    auto iter = gTaskQueues.find(id);
    if (iter != gTaskQueues.end())
    {
        gTaskQueues.erase(iter);
        return true;
    }
    return false;
}

uint32 TaskQueue::GetId() const
{
    return Id;
}

ThreadPool* TaskQueue::GetThreadPool() const
{
    return ThreadPool;
}

uint32 TaskQueue::GetNumWorkers() const
{
    return ThreadPool ? ThreadPool->GetNumWorkers() : 0;
}

void TaskQueue::Enqueue(const Task& task)
{
    PHX_PROFILE_ZONE_SCOPED;

    if (Tasks.empty())
    {
        Tasks.emplace_back();
    }

    Tasks.back().push_back(task);
}

void TaskQueue::Enqueue(Task&& task)
{
    PHX_PROFILE_ZONE_SCOPED;

    if (Tasks.empty())
    {
        Tasks.emplace_back();
    }

    Tasks.back().push_back(std::move(task));
}

void TaskQueue::Enqueue(TTaskFunc&& work)
{
    Enqueue(Task(std::move(work)));
}

void TaskQueue::Enqueue(std::vector<Task>&& tasks)
{
    Tasks.push_back(std::move(tasks));
}

std::vector<Task>& TaskQueue::BeginGroup(uint32 size)
{
    Tasks.emplace_back().reserve(size);
    return Tasks.back();
}

void TaskQueue::EndGroup()
{
    Tasks.emplace_back();
}

void TaskQueue::Flush()
{
    PHX_PROFILE_ZONE_SCOPED;

    // Run tasks on the current thread if no thread pool is available.
    // This allows using TaskQueue for simple task grouping even without a thread pool.
    if (ThreadPool == nullptr)
    {
        for (const std::vector<Task>& tasks : Tasks)
        {
            for (const Task& task : tasks)
            {
                task();
            }
        }
        Complete();
        return;
    }

    bIsCompleted.store(false, std::memory_order_release);

    for (const std::vector<Task>& tasks : Tasks)
    {
        std::vector<std::shared_ptr<TaskHandle>> handles;
        handles.reserve(tasks.size());
        
        for (const Task& task : tasks)
        {
            handles.push_back(ThreadPool->Submit(task));
        }

        Task::WaitAll(handles);
    }

    Complete();
}

void TaskQueue::Complete()
{
    Tasks.clear();
    CurrTaskIndex = 0;
    bIsCompleted.store(true, std::memory_order_release);
}

std::unique_ptr<ThreadPool> gThreadPool;

bool Phoenix::HasThreadPool()
{
    return gThreadPool != nullptr;
}

ThreadPool* Phoenix::GetThreadPool()
{
    return gThreadPool.get();
}

void Phoenix::SetThreadPool(const std::string& id, uint32 numWorkers, uint32 queueCapacity)
{
    gThreadPool = std::make_unique<ThreadPool>(id, numWorkers, queueCapacity);
}

void Phoenix::DestroyThreadPool()
{
    gThreadPool.release();
}
