
#include "EnkiTSExecutor.h"

using namespace Phoenix;

EnkiTSExecutor::EnkiTSExecutor(uint32 numWorkers)
{
    MainThreadId = std::this_thread::get_id();

    enki::TaskSchedulerConfig config;
    config.numTaskThreadsToCreate = numWorkers; // 0 → hardware_concurrency - 1
    config.numExternalTaskThreads = 1;          // reserve a slot for the session worker thread

    Scheduler.Initialize(config);

    // enkiTS's GetNumTaskThreads() counts all task threads (excludes main).
    // Size the pool so every worker can have kSlotsPerWorker tasks in flight.
    PoolSize = Scheduler.GetNumTaskThreads() * kSlotsPerWorker;
    if (PoolSize == 0)
        PoolSize = kSlotsPerWorker; // safe floor for zero-worker edge case

    Pool = std::make_unique<LambdaTask[]>(PoolSize);
}

EnkiTSExecutor::~EnkiTSExecutor()
{
    Scheduler.WaitforAll();
}

uint32 EnkiTSExecutor::GetNumWorkers() const
{
    return Scheduler.GetNumTaskThreads();
}

uint32 EnkiTSExecutor::GetCurrentThreadIndex() const
{
    // Main thread is always index 0. Worker threads return 1..N via the
    // SetCurrentThreadIndex call that ExecuteRange makes before running Fn.
    if (std::this_thread::get_id() == MainThreadId)
        return 0;
    return Phoenix::GetCurrentThreadIndex();
}

void EnkiTSExecutor::Submit(std::function<void()> fn)
{
    LambdaTask& slot = AcquireSlot();
    slot.Fn = std::move(fn);
    Scheduler.AddTaskSetToPipe(&slot);
}

EnkiTSExecutor::LambdaTask& EnkiTSExecutor::AcquireSlot()
{
    // Clock-hand scan: start at the hint position and wrap around.
    // On typical ECS workloads (O(numWorkers) concurrent tasks) this finds a
    // free slot in 1-2 iterations. We yield only when every slot is in use.
    for (;;)
    {
        const uint32 start = NextHint.fetch_add(1, std::memory_order_relaxed) % PoolSize;
        for (uint32 i = 0; i < PoolSize; ++i)
        {
            LambdaTask& slot = Pool[(start + i) % PoolSize];
            bool expected = true;
            if (slot.Free.compare_exchange_weak(expected, false,
                    std::memory_order_acquire, std::memory_order_relaxed))
            {
                return slot;
            }
        }
        std::this_thread::yield();
    }
}

bool EnkiTSExecutor::RegisterExternalThread()
{
    return Scheduler.RegisterExternalTaskThread();
}

void EnkiTSExecutor::DeregisterExternalThread()
{
    Scheduler.DeRegisterExternalTaskThread();
}

void EnkiTSExecutor::LambdaTask::ExecuteRange(enki::TaskSetPartition /*range*/, uint32_t threadNum_)
{
    // Set this thread's Phoenix index so GetCurrentThreadIndex() returns the
    // correct value for CommandBuffer selection. +1 because index 0 is the
    // main thread; enkiTS worker numbers are 0-based.
    Phoenix::SetCurrentThreadIndex(threadNum_ + 1);

    Fn();

    // Release the slot back to the pool AFTER Fn() returns.
    // Fn() may itself call Submit() (successor-node unlocking in JobScheduler),
    // which acquires a *different* slot — releasing this one first would only
    // be a problem if Fn() tried to reuse it, which it never does.
    Free.store(true, std::memory_order_release);
}
