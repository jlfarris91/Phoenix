#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include <Phoenix/FPSCalc.h>
#include <Phoenix/Platform.h>
#include <Phoenix.Sim/Worlds.h>

// Double-buffered world view for a sim/game thread split.
//
// The sim thread calls OnSimUpdate() after each world step. The game thread calls
// Sink() at the start of each frame and then reads from GetWorldView().
//
// Thread ownership:
//   SimView   — sim thread exclusive between swaps (no lock needed for SyncTo)
//   WorldView — game thread exclusive between swaps (no lock needed for reading)
//   SwapMutex — held only for the pointer swap + O(1) set swap (~ns)
//
// Stale-buffer correctness:
//   After a swap, SimView is the old WorldView (possibly many steps behind). Before the
//   next world.SyncTo we patch it with PendingPatchPages — the union of dirty pages from
//   all sim steps since the last swap — so the dirty-page diff is computed from a correct
//   baseline. The patch reads from WorldView concurrently with the game thread, which
//   is safe because both sides only read at that point.
class WorldDoubleBuffer
{
public:

    // Called by the sim thread after each world step completes.
    void OnSimUpdate(Phoenix::WorldConstRef world);

    // Called by the game thread at the start of each frame.
    // Swaps SimView and WorldView if a new sim frame is ready.
    // Holds SwapMutex for ~ns (pointer swap + O(1) set swap) — no SyncTo here.
    void Sink();

    // Returns the world the game thread should read from. Null until the first sim step.
    const Phoenix::World* GetWorldView() const;

    bool IsEnabled() const;
    void SetEnabled(bool enabled);

    double GetUpdateRate() const;
    Phoenix::uint32 GetAccumulatedDirtyPageCount() const;

private:

    const Phoenix::World* WorldView = nullptr;
    const Phoenix::World* SimView   = nullptr;

    std::mutex SwapMutex;
    std::atomic<bool> NewFrameReady{false};
    bool Enabled = true;
    Phoenix::FPSCalc UpdateCalc;

    // Sim-private: accumulated outside the mutex, moved in O(1) under the mutex.
    std::vector<Phoenix::uint32> CurrentStepPages;
    std::vector<std::vector<Phoenix::uint32>> PendingSteps;
    Phoenix::uint32 AccumulatedDirtyPageCount = 0;

    // Game-to-sim handoff: swapped O(1) under mutex, flattened outside mutex.
    std::vector<std::vector<Phoenix::uint32>> PatchSteps;
    std::atomic<bool> SimViewNeedsPatching{false};
};
