#include "TaskList.h"

#include "Phoenix/Profiling.h"

using namespace Phoenix;
using namespace Phoenix::Tasks;

namespace
{
    struct TaskKeyEquals
    {
        uint64 HiMask = UINT64_MAX;
        uint64 LoMask = UINT64_MAX;
        bool operator()(const TaskKey& a, const TaskKey& b) const
        {
            return (a.Hi & HiMask) == (b.Hi & HiMask) && (a.Lo & LoMask) == (b.Lo & LoMask);
        }

        static const TaskKeyEquals ByContext;
        static const TaskKeyEquals ByContextAndId;
    };

    const TaskKeyEquals TaskKeyEquals::ByContext        = { .HiMask = (uint64)UINT32_MAX << 32, .LoMask = 0LL };
    const TaskKeyEquals TaskKeyEquals::ByContextAndId   = { .HiMask = UINT64_MAX, .LoMask = 0LL };
}

const TaskHandle TaskHandle::Invalid = {};

TaskKey::TaskKey(uint32 context, FName id, FName type, uint32 handleId)
{
    Hi = (uint64)context << 32 | id;
    Lo = (uint64)type << 32 | handleId;
}

uint32 TaskKey::GetContext() const
{
    return Hi >> 32 & UINT32_MAX;
}

FName TaskKey::GetId() const
{
    return Hi & UINT32_MAX;
}

FName TaskKey::GetType() const
{
    return Lo >> 32 & UINT32_MAX;
}

uint32 TaskKey::GetHandleId() const
{
    return Lo & UINT32_MAX;
}

bool TaskKey::operator>(const TaskKey& other) const
{
    // Sorts by next tick time, then by user data, then by task id.
    return std::tie(Hi, Lo) > std::tie(other.Hi, other.Lo);
}

bool TaskKey::operator<(const TaskKey& other) const
{
    // Sorts by next tick time, then by user data, then by task id.
    return std::tie(Hi, Lo) < std::tie(other.Hi, other.Lo);
}

TaskEntry::TaskEntry(uint32 context, FName id, FName type, uint32 handleId)
    : Key(context, id, type, handleId)
{
}

uint32 TaskEntry::GetContext() const
{
    return Key.GetContext();
}

FName TaskEntry::GetId() const
{
    return Key.GetId();
}

FName TaskEntry::GetType() const
{
    return Key.GetType();
}

uint32 TaskEntry::GetHandleId() const
{
    return Key.GetHandleId();
}

const TaskKey& TaskEntry::GetKey() const
{
    return Key;
}

TaskHandle TaskEntry::GetHandle() const
{
    return { Key };
}

FixedBlockAllocator::Handle TaskEntry::GetDataHandle() const
{
    return DataHandle;
}

Time TaskEntry::GetInterval() const
{
    return Interval;
}

void TaskEntry::SetInterval(Time interval)
{
    // Ensure that interval is greater than 0 since an interval value of 0 is used to indicate that the task is invalid.
    Interval = Time::QT(std::max(interval.Value, 1));
}

Time TaskEntry::GetTickTime() const
{
    return TickTime;
}

bool TaskEntry::IsValid() const
{
    return Interval > 0;
}

void TaskEntry::Invalidate()
{
    Interval = 0;
}

TaskKey TaskEntry::GetItemKey::operator()(const TaskEntry& entry) const
{
    return entry.Key;
}

void FixedTaskList::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Entries.Construct(allocator, config.MaxTasks);
    Data.Construct(allocator, config.MaxTaskSize, config.MaxTasks);
}

BlockBufferLayout FixedTaskList::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedTaskList>()
        .Container<TFixedSortedList<TaskEntry, TaskEntry::GetItemKey>>("Entries", config.MaxTasks)
        .Container<FixedBlockAllocator>("Data", config.MaxTaskSize, config.MaxTasks);
}

uint32 FixedTaskList::GetNum() const
{
    return Entries.GetNum();
}

uint32 FixedTaskList::GetNumValid() const
{
    return Entries.GetNumValidItems();
}

uint32 FixedTaskList::GetCapacity() const
{
    return Entries.GetCapacity();
}

bool FixedTaskList::IsEmpty() const
{
    return Entries.IsEmpty();
}

bool FixedTaskList::IsFull() const
{
    return Entries.IsFull();
}

uint32 FixedTaskList::GetNumBlocks() const
{
    return Data.GetNumBlocks();
}

uint32 FixedTaskList::GetNumOccupiedBlocks() const
{
    return Data.GetNumOccupiedBlocks();
}

uint32 FixedTaskList::GetBlockSize() const
{
    return Data.GetBlockSize();
}

uint32 FixedTaskList::GetBlockCapacity() const
{
    return Data.GetBlockCapacity();
}

TaskHandle FixedTaskList::Allocate(uint32 context, FName type, const void* source, uint32 size)
{
    return Allocate(context, type, type, source, size);
}

TaskHandle FixedTaskList::Allocate(uint32 context, FName id, FName type, const void* source, uint32 size)
{
    if (IsFull())
    {
        return TaskHandle::Invalid;
    }

    TaskEntry entry(context, id, type, ++TaskIdGen);
    entry.DataHandle = Data.Allocate(entry.GetHandleId(), source, size);

    Entries.PushBack(entry);

    return entry.GetHandle();
}

bool FixedTaskList::Deallocate(TaskHandle handle)
{
    uint32 index;
    TaskEntry* entry = GetEntry(handle, index);
    if (!entry)
    {
        return false;
    }

    if (!Data.Deallocate(entry->DataHandle))
    {
        return false;
    }

    Entries.RemoveAt(index);

    return true;
}

TaskEntry* FixedTaskList::GetEntry(TaskHandle handle)
{
    uint32 index;
    return GetEntry(handle, index);
}

const TaskEntry* FixedTaskList::GetEntry(TaskHandle handle) const
{
    uint32 index;
    return GetEntry(handle, index);
}

TaskEntry* FixedTaskList::GetEntry(TaskHandle handle, uint32& outIndex)
{
    return Entries.GetFirstItem(handle.Key, outIndex);
}

const TaskEntry* FixedTaskList::GetEntry(TaskHandle handle, uint32& outIndex) const
{
    return Entries.GetFirstItem(handle.Key, outIndex);
}

TaskEntry* FixedTaskList::GetFirstEntry(uint32 context, uint32& outIndex)
{
    TaskKey key(context);
    return Entries.GetFirstItem(key, outIndex, TaskKeyEquals::ByContext);
}

const TaskEntry* FixedTaskList::GetFirstEntry(uint32 context, uint32& outIndex) const
{
    TaskKey key(context);
    return Entries.GetFirstItem(key, outIndex, TaskKeyEquals::ByContext);
}

TaskEntry* FixedTaskList::GetNextEntry(uint32 context, uint32& inOutIndex)
{
    TaskKey key(context);
    return Entries.GetNextItem(key, inOutIndex, inOutIndex, TaskKeyEquals::ByContext);
}

const TaskEntry* FixedTaskList::GetNextEntry(uint32 context, uint32& inOutIndex) const
{
    TaskKey key(context);
    return Entries.GetNextItem(key, inOutIndex, inOutIndex, TaskKeyEquals::ByContext);
}

TaskEntry* FixedTaskList::GetFirstEntry(uint32 context, FName id, uint32& outIndex)
{
    TaskKey key(context, id);
    return Entries.GetFirstItem(key, outIndex, TaskKeyEquals::ByContextAndId);
}

const TaskEntry* FixedTaskList::GetFirstEntry(uint32 context, FName id, uint32& outIndex) const
{
    TaskKey key(context, id);
    return Entries.GetFirstItem(key, outIndex, TaskKeyEquals::ByContextAndId);
}

TaskEntry* FixedTaskList::GetNextEntry(uint32 context, FName id, uint32& inOutIndex)
{
    TaskKey key(context, id);
    return Entries.GetNextItem(key, inOutIndex, inOutIndex, TaskKeyEquals::ByContextAndId);
}

const TaskEntry* FixedTaskList::GetNextEntry(uint32 context, FName id, uint32& inOutIndex) const
{
    TaskKey key(context, id);
    return Entries.GetNextItem(key, inOutIndex, inOutIndex, TaskKeyEquals::ByContextAndId);
}

TOptional<uint32> FixedTaskList::GetContext(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetContext() : TOptional<uint32>();
}

TOptional<FName> FixedTaskList::GetType(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetType() : TOptional<FName>();
}

TOptional<FName> FixedTaskList::GetId(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetId() : TOptional<FName>();
}

TOptional<Time> FixedTaskList::GetTickTime(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetTickTime() : TOptional<Time>();
}

TOptional<Time> FixedTaskList::GetInterval(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetInterval() : TOptional<Time>();
}

bool FixedTaskList::SetInterval(TaskHandle handle, Time interval)
{
    if (TaskEntry* entry = GetEntry(handle))
    {
        entry->SetInterval(interval);
        return true;
    }
    return false;
}

TOptional<uint32> FixedTaskList::GetIntervalTicks(TaskHandle handle) const
{
    const TaskEntry* entry = GetEntry(handle);
    return entry ? entry->GetInterval().Value : TOptional<uint32>();
}

bool FixedTaskList::SetIntervalTicks(TaskHandle handle, uint32 ticks)
{
    return SetInterval(handle, Time::QT(ticks));
}

void* FixedTaskList::GetData(const TaskEntry* entry)
{
    return Data.GetPtr(entry->DataHandle);
}

const void* FixedTaskList::GetData(const TaskEntry* entry) const
{
    return Data.GetPtr(entry->DataHandle);
}

void* FixedTaskList::GetData(TaskHandle handle)
{
    uint32 index;
    const TaskEntry* entry = Entries.GetFirstItem(handle.Key, index);
    return entry ? GetData(entry) : nullptr;
}

const void* FixedTaskList::GetData(TaskHandle handle) const
{
    uint32 index;
    const TaskEntry* entry = Entries.GetFirstItem(handle.Key, index);
    return entry ? GetData(entry) : nullptr;
}

void FixedTaskList::Update(Time worldTime)
{
    PHX_PROFILE_ZONE_SCOPED;

    uint32 prevNumSorted = Entries.GetSortedNum();

    // Update the next tick time for each active task prior to sorting
    for (uint32 i = 0; i < Entries.GetNum(); ++i)
    {
        auto entry = Entries.GetData() + i;
        if (entry->IsValid())
        {
            if (entry->TickTime <= worldTime)
            {
                entry->TickTime = worldTime + entry->Interval;
            }
        }
        else
        {
            entry->TickTime = Time::Max;
        }
    }

    Entries.Sort();

    // Find any invalid entries that were sorted out and deallocate their data
    bool foundInvalidEntry = false;
    for (uint32 i = Entries.GetSortedNum(); i < prevNumSorted; ++i)
    {
        auto invalidEntry = Entries.GetData() + i;
        PHX_ASSERT(invalidEntry->IsValid() == false);
        Data.Deallocate(invalidEntry->DataHandle);
        foundInvalidEntry = true;
    }

    if (foundInvalidEntry)
    {
        Data.Compact();
    }
}
