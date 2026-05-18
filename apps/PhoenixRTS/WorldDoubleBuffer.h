#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include <Phoenix/FPSCalc.h>
#include <Phoenix/Platform.h>
#include <Phoenix.Sim/Worlds.h>

// Double-buffered world view for a sim/render split.
//
// The sim thread calls OnSimUpdate() after each world step. The render thread calls
// OnRenderFrameStart() at the start of each frame and then reads from GetRenderView().
//
// Thread ownership:
//   SimView    — sim thread exclusive between swaps (no lock needed for SyncTo)
//   RenderView — render thread exclusive between swaps (no lock needed for reading)
//   SwapMutex  — held only for the pointer swap + O(1) set swap (~ns)
//
// Stale-buffer correctness:
//   After a swap, SimView is the old RenderView (possibly many steps behind). Before the
//   next world.SyncTo we patch it with PendingPatchPages — the union of dirty pages from
//   all sim steps since the last swap — so the dirty-page diff is computed from a correct
//   baseline. The patch reads from RenderView concurrently with the render thread, which
//   is safe because both sides only read at that point.
class WorldDoubleBuffer
{
public:

    // Called by the sim thread after each world step completes.
    void OnSimUpdate(Phoenix::WorldConstRef world);

    // Called by the render thread at the start of each frame.
    // Swaps SimView and RenderView if a new sim frame is ready.
    // Holds SwapMutex for ~ns (pointer swap + O(1) set swap) — no SyncTo here.
    void OnRenderFrameStart();

    // Returns the world the render thread should read from. Null until the first sim step.
    const Phoenix::World* GetRenderView() const;

    bool IsEnabled() const;
    void SetEnabled(bool enabled);

    double GetUpdateRate() const;
    Phoenix::uint32 GetAccumulatedDirtyPageCount() const;

private:

    const Phoenix::World* RenderView = nullptr;
    const Phoenix::World* SimView = nullptr;

    std::mutex SwapMutex;
    std::atomic<bool> NewFrameReady{false};
    bool Enabled = true;
    Phoenix::FPSCalc UpdateCalc;

    // Sim-private: accumulated outside the mutex, moved in O(1) under the mutex.
    std::vector<Phoenix::uint32> CurrentStepPages;
    std::vector<std::vector<Phoenix::uint32>> PendingSteps;
    Phoenix::uint32 AccumulatedDirtyPageCount = 0;

    // Render-to-sim handoff: swapped O(1) under mutex, flattened outside mutex.
    std::vector<std::vector<Phoenix::uint32>> PatchSteps;
    std::atomic<bool> SimViewNeedsPatching{false};
};
