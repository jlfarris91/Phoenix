#include "WorldDoubleBuffer.h"

void WorldDoubleBuffer::OnSimUpdate(Phoenix::WorldConstRef world)
{
    if (!Enabled)
        return;

    UpdateCalc.Time = PHX_SYS_CLOCK_NOW();

    // If a swap happened, SimView is stale. Patch it before SyncTo so the dirty-page
    // diff in the next step is computed from a correct baseline.
    // RenderView: both threads read — no write race.
    // SimView: sim-exclusive after the swap — safe to write without a mutex.
    if (SimViewNeedsPatching.load(std::memory_order_acquire))
    {
        SimViewNeedsPatching.store(false, std::memory_order_relaxed);
        if (SimView != nullptr)
        {
            RenderView->SyncTo(*const_cast<Phoenix::World*>(SimView), PatchSteps);
        }
        PatchSteps.clear();
    }

    // Sync this step into SimView. SimView is sim-exclusive — no mutex needed.
    if (!SimView)
    {
        SimView = new Phoenix::World(world);
    }
    else
    {
        world.SyncTo(*const_cast<Phoenix::World*>(SimView));
    }

    // Capture this step's dirty pages outside the mutex (O(n) work stays off the mutex).
    const std::vector<Phoenix::uint32>& dirty = world.GetBuffer().GetDirtyPageOffsets();
    CurrentStepPages.assign(dirty.begin(), dirty.end());

    // Hold the mutex only for O(1) work: move the step vector in and signal.
    {
        std::lock_guard lock(SwapMutex);
        PendingSteps.push_back(std::move(CurrentStepPages));
        CurrentStepPages.clear();
        NewFrameReady.store(true, std::memory_order_release);
    }

    UpdateCalc.Tick();
}

void WorldDoubleBuffer::OnRenderFrameStart()
{
    if (!NewFrameReady.load(std::memory_order_acquire))
    {
        return;
    }

    {
        std::lock_guard lock(SwapMutex);
        std::swap(RenderView, SimView);
        std::swap(PendingSteps, PatchSteps);
        SimViewNeedsPatching.store(true, std::memory_order_release);
        NewFrameReady.store(false, std::memory_order_relaxed);
    }

    AccumulatedDirtyPageCount = 0;
    for (const auto& step : PatchSteps)
    {
        AccumulatedDirtyPageCount += static_cast<Phoenix::uint32>(step.size());
    }
}

const Phoenix::World* WorldDoubleBuffer::GetRenderView() const
{
    return RenderView;
}

bool WorldDoubleBuffer::IsEnabled() const
{
    return Enabled;
}

void WorldDoubleBuffer::SetEnabled(bool enabled)
{
    Enabled = enabled;
}

double WorldDoubleBuffer::GetUpdateRate() const
{
    return UpdateCalc.GetFPS();
}

Phoenix::uint32 WorldDoubleBuffer::GetAccumulatedDirtyPageCount() const
{
    return AccumulatedDirtyPageCount;
}
