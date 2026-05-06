#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ETimerFlags : uint8
    {
        None,
        Running = 1,
        Paused = 2,
        Expired = 4,
        Repeat = 8,
        Invalid = 16
    };

    struct PHOENIX_RTS_API Timer
    {
        FName Id;
        Time StartTime;
        Time EndTime;
        Time Duration;
        ETimerFlags Flags;

        bool IsValid() const { return HasAnyFlags(Flags, ETimerFlags::Invalid); }
        void Invalidate() { SetFlagRef(Flags, ETimerFlags::Invalid); }

        struct GetItemKey
        {
            FName operator()(const Timer& timer) const
            {
                return timer.Id;
            }
        };
    };

    class PHOENIX_SIM_API FixedTimerManager
    {
        using TStorage = TFixedSortedList<Timer, Timer::GetItemKey>;

    public:
        
        PHX_DECLARE_BLOCK_CONTAINER(FixedTimerManager)
        {
            uint32 Capacity;
        };

        uint32 GetCapacity() const;

        uint32 GetNumActiveTimers() const;

        // Creates a new timer.
        // Returns false if a timer with the given id already exists.
        bool AcquireTimer(const FName& id, Time duration, bool startNow = true, bool repeats = false);

        // Releases an existing timer if it exists.
        bool ReleaseTimer(const FName& id);

        // Returns true if the timer exists and is valid.
        bool IsTimerValid(const FName& id) const;

        // Starts an existing time optionally with new settings.
        bool StartTimer(const FName& id, const TOptional<Time>& duration = {}, const TOptional<bool>& repeats = {});

        // Stops a timer but preserves the duration and whether it repeats.
        bool StopTimer(const FName& id);

        // Restarts a timer using the initial duration. 
        bool RestartTimer(const FName& id);

        // Returns true if the timer exists and is currently running.
        bool PauseTimer(const FName& id);

        // Returns true if the timer exists and is currently paused.
        bool ResumeTimer(const FName& id);

        // Returns true if a timer is currently running.
        // Note that this returns true even if the timer is currently paused.
        bool IsRunning(const FName& id) const;

        // Returns true if a timer was previously started and is currently paused.
        bool IsPaused(const FName& id) const;

        // Returns true if a timer was running and has expired.
        bool IsExpired(const FName& id) const;

        // Gets the time when a timer was started.
        TOptional<Time> GetStartTime(const FName& id) const;

        // Gets the time that a timer will expire.
        TOptional<Time> GetEndTime(const FName& id) const;

        // Gets the duration of a timer.
        TOptional<Time> GetDuration(const FName& id) const;

        // Gets the time elapsed since the timer was started.
        TOptional<Time> GetTimeElapsed(const FName& id) const;

        // Gets the time remaining until the timer will expire.
        TOptional<Time> GetTimeRemaining(const FName& id) const;

        // Gets the flags of a timer.
        TOptional<ETimerFlags> GetTimerFlags(const FName& id) const;

        // Gets a pointer to a timer.
        const Timer* GetTimer(const FName& id) const;

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            ForEachItem(callback);
        }

        // Sort and tick all timers.
        void Tick(Time currentTime);

    private:

        void StartTimerInternal(Timer& timer, const TOptional<Time>& duration, const TOptional<bool>& repeats) const;

        static void StopTimerInternal(Timer& timer);

        void RestartTimerInternal(Timer& timer) const;

        static void PauseTimerInternal(Timer& timer);

        static void ResumeTimerInternal(Timer& timer);

        void TickTimer(Timer& timer) const;

        TStorage Storage;
        Time CurrentTime;
        Time DeltaTime;
    };
}
