#include "FixedTimerManager.h"

Phoenix::uint32 Phoenix::RTS::FixedTimerManager::GetCapacity() const
{
    return Storage.GetCapacity();
}

Phoenix::uint32 Phoenix::RTS::FixedTimerManager::GetAllocSizeBytes(uint32 capacity)
{
    return TStorage::GetAllocSizeBytes(capacity);
}

Phoenix::uint32 Phoenix::RTS::FixedTimerManager::GetAllocSizeBytes() const
{
    return Storage.GetAllocSizeBytes();
}

Phoenix::uint32 Phoenix::RTS::FixedTimerManager::GetNumActiveTimers() const
{
    return Storage.GetNumValidItems();
}

bool Phoenix::RTS::FixedTimerManager::AcquireTimer(const FName& id, Time duration, bool startNow, bool repeats)
{
    if (IsTimerValid(id))
    {
        return false;
    }

    ETimerFlags flags = ETimerFlags::None;
    SetFlagRef(flags, ETimerFlags::Running, startNow);
    SetFlagRef(flags, ETimerFlags::Repeat, repeats);
    return Storage.PushBack({ id, CurrentTime, CurrentTime + duration, duration, flags });
}

bool Phoenix::RTS::FixedTimerManager::ReleaseTimer(const FName& id)
{
    return Storage.RemoveAll(id);
}

bool Phoenix::RTS::FixedTimerManager::IsTimerValid(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid();
}

bool Phoenix::RTS::FixedTimerManager::StartTimer(const FName& id, const TOptional<Time>& duration,
    const TOptional<bool>& repeats)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return false;
    }

    StartTimerInternal(*timer, duration, repeats);
    return true;
}

bool Phoenix::RTS::FixedTimerManager::StopTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return false;
    }

    StopTimerInternal(*timer);
    return true;
}

bool Phoenix::RTS::FixedTimerManager::RestartTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return false;
    }

    RestartTimerInternal(*timer);
    return true;
}

bool Phoenix::RTS::FixedTimerManager::PauseTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Running))
    {
        return false;
    }

    PauseTimerInternal(*timer);
    return true;
}

bool Phoenix::RTS::FixedTimerManager::ResumeTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Paused))
    {
        return false;
    }

    ResumeTimerInternal(*timer);
    return true;
}

bool Phoenix::RTS::FixedTimerManager::IsRunning(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Running);
}

bool Phoenix::RTS::FixedTimerManager::IsPaused(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
}

bool Phoenix::RTS::FixedTimerManager::IsExpired(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
}

Phoenix::TOptional<Phoenix::TFixed<6>> Phoenix::RTS::FixedTimerManager::GetStartTime(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->StartTime;
}

Phoenix::TOptional<Phoenix::TFixed<6>> Phoenix::RTS::FixedTimerManager::GetEndTime(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->EndTime;
}

Phoenix::TOptional<Phoenix::TFixed<6>> Phoenix::RTS::FixedTimerManager::GetDuration(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->Duration;
}

Phoenix::TOptional<Phoenix::TFixed<6>> Phoenix::RTS::FixedTimerManager::GetTimeElapsed(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }

    return Time(CurrentTime - timer->StartTime);
}

Phoenix::TOptional<Phoenix::TFixed<6>> Phoenix::RTS::FixedTimerManager::GetTimeRemaining(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }

    return Time(timer->StartTime + timer->Duration - CurrentTime);
}

Phoenix::TOptional<Phoenix::RTS::ETimerFlags> Phoenix::RTS::FixedTimerManager::GetTimerFlags(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->Flags;
}

const Phoenix::RTS::Timer* Phoenix::RTS::FixedTimerManager::GetTimer(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return nullptr;
    }
    return timer;
}

void Phoenix::RTS::FixedTimerManager::Tick(Time currentTime)
{
    PHX_ASSERT(currentTime > CurrentTime);
    DeltaTime = currentTime - CurrentTime;
    CurrentTime = currentTime;

    Storage.Sort();

    Storage.ForEachItem([this](Timer& timer)
    {
        TickTimer(timer);
    });
}

void Phoenix::RTS::FixedTimerManager::StartTimerInternal(Timer& timer, const TOptional<Time>& duration,
    const TOptional<bool>& repeats) const
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

void Phoenix::RTS::FixedTimerManager::StopTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    ClearFlagRef(timer.Flags, ETimerFlags::Running);
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
    ClearFlagRef(timer.Flags, ETimerFlags::Expired);
}

void Phoenix::RTS::FixedTimerManager::RestartTimerInternal(Timer& timer) const
{
    PHX_ASSERT(timer.IsValid());
    timer.StartTime = CurrentTime;
    timer.EndTime = CurrentTime + timer.Duration;
    SetFlagRef(timer.Flags, ETimerFlags::Running);
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
    ClearFlagRef(timer.Flags, ETimerFlags::Expired);
}

void Phoenix::RTS::FixedTimerManager::PauseTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    SetFlagRef(timer.Flags, ETimerFlags::Paused);
}

void Phoenix::RTS::FixedTimerManager::ResumeTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
}

void Phoenix::RTS::FixedTimerManager::TickTimer(Timer& timer) const
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
