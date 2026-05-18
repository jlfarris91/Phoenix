#include "FixedTimerManager.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

void FixedTimerManager::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedTimerManager::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedTimerManager>().Container<TStorage>(config.Capacity);
}

uint32 FixedTimerManager::GetCapacity() const
{
    return Storage.GetCapacity();
}

uint32 FixedTimerManager::GetNumActiveTimers() const
{
    return Storage.GetNumValidItems();
}

bool FixedTimerManager::AcquireTimer(const FName& id, Time duration, bool startNow, bool repeats)
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

bool FixedTimerManager::ReleaseTimer(const FName& id)
{
    return Storage.RemoveAll(id);
}

bool FixedTimerManager::IsTimerValid(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid();
}

bool FixedTimerManager::StartTimer(const FName& id, const TOptional<Time>& duration,
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

bool FixedTimerManager::StopTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return false;
    }

    StopTimerInternal(*timer);
    return true;
}

bool FixedTimerManager::RestartTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return false;
    }

    RestartTimerInternal(*timer);
    return true;
}

bool FixedTimerManager::PauseTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Running))
    {
        return false;
    }

    PauseTimerInternal(*timer);
    return true;
}

bool FixedTimerManager::ResumeTimer(const FName& id)
{
    Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid() || HasNoneFlags(timer->Flags, ETimerFlags::Paused))
    {
        return false;
    }

    ResumeTimerInternal(*timer);
    return true;
}

bool FixedTimerManager::IsRunning(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Running);
}

bool FixedTimerManager::IsPaused(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
}

bool FixedTimerManager::IsExpired(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    return timer && timer->IsValid() && HasAnyFlags(timer->Flags, ETimerFlags::Paused);
}

TOptional<TFixed<6>> FixedTimerManager::GetStartTime(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->StartTime;
}

TOptional<TFixed<6>> FixedTimerManager::GetEndTime(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->EndTime;
}

TOptional<TFixed<6>> FixedTimerManager::GetDuration(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->Duration;
}

TOptional<TFixed<6>> FixedTimerManager::GetTimeElapsed(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }

    return Time(CurrentTime - timer->StartTime);
}

TOptional<TFixed<6>> FixedTimerManager::GetTimeRemaining(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }

    return Time(timer->StartTime + timer->Duration - CurrentTime);
}

TOptional<ETimerFlags> FixedTimerManager::GetTimerFlags(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return {};
    }
    return timer->Flags;
}

const Timer* FixedTimerManager::GetTimer(const FName& id) const
{
    const Timer* timer = Storage.GetItem(id);
    if (!timer || !timer->IsValid())
    {
        return nullptr;
    }
    return timer;
}

void FixedTimerManager::Tick(Time currentTime)
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

void FixedTimerManager::StartTimerInternal(Timer& timer, const TOptional<Time>& duration,
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

void FixedTimerManager::StopTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    ClearFlagRef(timer.Flags, ETimerFlags::Running);
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
    ClearFlagRef(timer.Flags, ETimerFlags::Expired);
}

void FixedTimerManager::RestartTimerInternal(Timer& timer) const
{
    PHX_ASSERT(timer.IsValid());
    timer.StartTime = CurrentTime;
    timer.EndTime = CurrentTime + timer.Duration;
    SetFlagRef(timer.Flags, ETimerFlags::Running);
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
    ClearFlagRef(timer.Flags, ETimerFlags::Expired);
}

void FixedTimerManager::PauseTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    SetFlagRef(timer.Flags, ETimerFlags::Paused);
}

void FixedTimerManager::ResumeTimerInternal(Timer& timer)
{
    PHX_ASSERT(timer.IsValid());
    ClearFlagRef(timer.Flags, ETimerFlags::Paused);
}

void FixedTimerManager::TickTimer(Timer& timer) const
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
