#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/Containers/FixedSortedList.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_SIM_API ETimerFlags : uint8
    {
        None,
        Running = 1,
        Paused = 2,
        Expired = 4,
        Repeat = 8,
        Invalid = 16
    };

    struct PHOENIX_SIM_API Timer
    {
        FName Id;
        Time StartTime;
        Time EndTime;
        Time Duration;
        ETimerFlags Flags;

        bool IsValid() const { return HasAnyFlags(Flags, ETimerFlags::Invalid); }
        void Invalidate() { SetFlagRef(Flags, ETimerFlags::Invalid); }
    };

    template <uint32 N>
    struct FixedTimerManager
    {
        static constexpr uint32 Capacity = N;

        uint32 GetSize() const
        {
            return Timers.GetSize();
        }

        uint32 GetNumActiveTimers() const
        {
            return Timers.GetNumValidItems();
        }

        // Creates a new timer.
        // Returns false if a timer with the given id already exists.
        bool AcquireTimer(const FName& id, Time duration, bool startNow = true, bool repeats = false)
        {
            ETimerFlags flags = ETimerFlags::None;
            SetFlagRef(flags, ETimerFlags::Running, startNow);
            SetFlagRef(flags, ETimerFlags::Repeat, repeats);
            return Timers.PushBackUnique(id, CurrentTime, CurrentTime + duration, flags);
        }

        // Releases an existing timer if it exists.
        bool ReleaseTimer(const FName& id)
        {
            return Timers.RemoveAll(id);
        }

        // Returns true if the timer exists and is valid.
        bool IsTimerValid(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            return timer && timer->IsValid();
        }

        // Starts an existing time optionally with new settings.
        bool StartTimer(const FName& id, const TOptional<Time>& duration = {}, const TOptional<bool>& repeats = {})
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return false;
            }

            StartTimerInternal(*timer, duration, repeats);
            return true;
        }

        // Stops a timer but preserves the duration and whether it repeats.
        bool StopTimer(const FName& id)
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return false;
            }

            StopTimerInternal(*timer);
            return true;
        }

        // Restarts a timer using the initial duration. 
        bool RestartTimer(const FName& id)
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return false;
            }

            RestartTimerInternal(*timer);
            return true;
        }

        // Returns true if the timer exists and is currently running.
        bool PauseTimer(const FName& id)
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Running))
            {
                return false;
            }

            PauseTimerInternal(*timer);
            return true;
        }

        // Returns true if the timer exists and is currently paused.
        bool ResumeTimer(const FName& id)
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Paused))
            {
                return false;
            }

            ResumeTimerInternal(*timer);
            return true;
        }

        // Returns true if a timer is currently running.
        // Note that this returns true even if the timer is currently paused.
        bool IsRunning(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Running);
        }

        // Returns true if a timer was previously started and is currently paused.
        bool IsPaused(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
        }

        // Returns true if a timer was running and has expired.
        bool IsExpired(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
        }

        // Gets the time when a timer was started.
        TOptional<Time> GetStartTime(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }
            return timer->StartTime;
        }

        // Gets the time that a timer will expire.
        TOptional<Time> GetEndTime(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }
            return timer->EndTime;
        }

        // Gets the duration of a timer.
        TOptional<Time> GetDuration(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }
            return timer->Duration;
        }

        // Gets the time elapsed since the timer was started.
        TOptional<Time> GetTimeElapsed(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }

            return Time(CurrentTime - timer->StartTime);
        }

        // Gets the time remaining until the timer will expire.
        TOptional<Time> GetTimeRemaining(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }

            return Time(timer->StartTime + timer->Duration - CurrentTime);
        }

        // Gets the flags of a timer.
        TOptional<ETimerFlags> GetTimerFlags(const FName& id) const
        {
            uint32 index;
            Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return {};
            }
            return timer->Flags;
        }

        // Gets a pointer to a timer.
        const Timer* GetTimer(const FName& id) const
        {
            uint32 index;
            const Timer* timer = Timers.GetFirstSubItem(id, index);
            if (!timer || !timer->IsValid())
            {
                return nullptr;
            }
            return timer;
        }

        template <class TCallback>
        void ForEach(const TCallback& callback) const
        {
            Timers.ForEachItem([&](const Timer& timer)
            {
                callback(timer);
            });
        }

        // Sort and tick all timers.
        void Tick(Time currentTime)
        {
            PHX_ASSERT(currentTime > CurrentTime);
            DeltaTime = currentTime - CurrentTime;
            CurrentTime = currentTime;

            Timers.Sort();

            Timers.ForEachItem([this](Timer& timer)
            {
                TickTimer(timer);
            });
        }

    private:

        struct GetItemKey
        {
            FName operator()(const Timer& timer) const
            {
                return timer.Id;
            }
        };

        void StartTimerInternal(Timer& timer, const TOptional<Time>& duration, const TOptional<bool>& repeats) const
        {
            PHX_ASSERT(timer.IsValid());

            timer.StartTime = CurrentTime;

            if (duration.IsSet())
            {
                timer.Duration = duration.Get();
            }

            timer.EndTime = CurrentTime + timer.Duration;

            timer.Flags = ETimerFlags::Running;

            if (repeats.IsSet())
            {
                SetFlagRef(timer.Flags, ETimerFlags::Repeat, repeats.Get());
            }
        }

        static void StopTimerInternal(Timer& timer)
        {
            PHX_ASSERT(timer.IsValid());
            ClearFlagRef(timer.Flags, ETimerFlags::Running);
            ClearFlagRef(timer.Flags, ETimerFlags::Paused);
            ClearFlagRef(timer.Flags, ETimerFlags::Expired);
        }

        void RestartTimerInternal(Timer& timer) const
        {
            PHX_ASSERT(timer.IsValid());
            timer.StartTime = CurrentTime;
            timer.EndTime = CurrentTime + timer.Duration;
            SetFlagRef(timer.Flags, ETimerFlags::Running);
            ClearFlagRef(timer.Flags, ETimerFlags::Paused);
            ClearFlagRef(timer.Flags, ETimerFlags::Expired);
        }

        static void PauseTimerInternal(Timer& timer)
        {
            PHX_ASSERT(timer.IsValid());
            SetFlagRef(timer.Flags, ETimerFlags::Paused);
        }

        static void ResumeTimerInternal(Timer& timer)
        {
            PHX_ASSERT(timer.IsValid());
            ClearFlagRef(timer.Flags, ETimerFlags::Paused);
        }

        void TickTimer(Timer& timer) const
        {
            PHX_ASSERT(timer.IsValid());

            if (HasAnyFlags(timer.Flags, ETimerFlags::Expired))
            {
                if (HasAnyFlags(timer.Flags, ETimerFlags::Repeat))
                {
                    RestartTimerInternal(timer);
                }
                return;
            }

            if (HasAnyFlags(timer.Flags, ETimerFlags::Paused))
            {
                timer.EndTime += DeltaTime;
                return;
            }

            if (HasAnyFlags(timer.Flags, ETimerFlags::Running) && CurrentTime >= timer.EndTime)
            {
                ClearFlagRef(timer.Flags, ETimerFlags::Running);
                SetFlagRef(timer.Flags, ETimerFlags::Expired);
            }
        }

        Time CurrentTime;
        Time DeltaTime;
        TFixedSortedList<Timer, N, GetItemKey> Timers;
    };
}
