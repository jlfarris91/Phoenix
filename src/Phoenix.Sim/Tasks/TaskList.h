#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/Containers/FixedBlockAllocator.h"
#include "Phoenix.Sim/Containers/FixedSortedList.h"
#include "Phoenix.Sim/Containers/Optional.h"
#include "Phoenix.Sim/FixedPoint/FixedTypes.h"

namespace Phoenix::Tasks
{
    struct PHOENIX_SIM_API TaskKey
    {
        TaskKey() = default;
        TaskKey(const TaskKey&) = default;
        TaskKey(uint32 context, FName id = FName::None, FName type = FName::None, uint32 handleId = 0);

        // Gets the context for the task.
        uint32 GetContext() const;

        // Gets the user-defined id for this task.
        FName GetId() const;

        // Gets the type of this task.
        // The type is used to determine which task definition to use when executing the task.
        FName GetType() const;

        // Gets the handle id for this task.
        // Each task gets a monotonically incrementing id since user-assigned ids are not guaranteed to be unique.
        uint32 GetHandleId() const;

        uint64 Hi = 0;
        uint64 Lo = 0;

        bool operator==(const TaskKey& other) const = default;
        bool operator!=(const TaskKey& other) const = default;
        bool operator>(const TaskKey& other) const;
        bool operator<(const TaskKey& other) const;
    };

    struct PHOENIX_SIM_API TaskHandle
    {
        static const TaskHandle Invalid;
        TaskKey Key;

        bool operator==(const TaskHandle&) const = default;
        bool operator!=(const TaskHandle&) const = default;
    };

    class PHOENIX_SIM_API TaskEntry
    {
    public:
        TaskEntry() = default;
        TaskEntry(uint32 context, FName id, FName type, uint32 handleId);

        // Gets the context for the task.
        uint32 GetContext() const;

        // Gets the user-defined id for this task.
        FName GetId() const;

        // Gets the type of this task.
        // The type is used to determine which task definition to use when executing the task.
        FName GetType() const;

        // Gets the handle id for this task.
        // Each task gets a monotonically incrementing id since user-assigned ids are not guaranteed to be unique.
        uint32 GetHandleId() const;

        // Gets the key for this task.
        const TaskKey& GetKey() const;

        // Gets the handle for this task.
        TaskHandle GetHandle() const;

        // Gets the handle to the data for this task.
        // User data is opaque to the engine and is managed by the task itself.
        FixedBlockAllocator::Handle GetDataHandle() const;

        // Gets the interval
        Time GetInterval() const;

        // Sets the interval for this task. Update takes effect in the next step.
        void SetInterval(Time interval);

        // Gets the next time that this task will tick.
        Time GetTickTime() const;

        // Returns true if the task is valid.
        bool IsValid() const;

        // Invalidates a task. Invalid tasks are pruned at the end of the step.
        // You should not call this.
        void Invalidate();

    private:

        friend class FixedTaskList;

        struct GetItemKey
        {
            TaskKey operator()(const TaskEntry& entry) const;
        };

        TaskKey Key;
        FixedBlockAllocator::Handle DataHandle;
        Time Interval = Time::QOne;
        Time TickTime = 0;
    };

    // Stores a collection of active tasks, their associated data, and their next tick times.
    // The engine supports efficient allocation and deallocation of tasks, as well as iteration over active tasks in
    // order of their next tick time.
    class PHOENIX_SIM_API FixedTaskList
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(FixedTaskList)
        {
            uint32 MaxTasks = 0;
            uint32 MaxTaskSize = 0;
        };

        // Gets the total number of tasks in the data buffer, including invalid tasks pending pruning.
        uint32 GetNum() const;

        // Gets the number of valid tasks in the data buffer.
        // This may be less than the total number of tasks allocated, as some tasks may be invalid and pending pruning.
        uint32 GetNumValid() const;
        
        // Gets the maximum number of tasks that can be allocated in the data buffer.
        uint32 GetCapacity() const;

        // Returns true if there are no tasks in the data buffer.
        bool IsEmpty() const;

        // Returns true if the data buffer is at capacity and cannot allocate more tasks until some are deallocated.
        bool IsFull() const;

        // Gets the total number of blocks in the data buffer, including unoccupied blocks.
        uint32 GetNumBlocks() const;

        // Gets the number of occupied blocks in the data buffer.
        uint32 GetNumOccupiedBlocks() const;

        // Gets the size of a block in the data buffer in bytes.
        uint32 GetBlockSize() const;

        // Gets the maximum number of blocks the data buffer can hold.
        uint32 GetBlockCapacity() const;

        TaskHandle Allocate(uint32 context, FName type, const void* source, uint32 size);
        TaskHandle Allocate(uint32 context, FName id, FName type, const void* source, uint32 size);

        bool Deallocate(TaskHandle handle);

        TaskEntry* GetEntry(TaskHandle handle);
        const TaskEntry* GetEntry(TaskHandle handle) const;

        TaskEntry* GetEntry(TaskHandle handle, uint32& outIndex);
        const TaskEntry* GetEntry(TaskHandle handle, uint32& outIndex) const;

        TaskEntry* GetFirstEntry(uint32 context, uint32& outIndex);
        const TaskEntry* GetFirstEntry(uint32 context, uint32& outIndex) const;

        TaskEntry* GetNextEntry(uint32 context, uint32& inOutIndex);
        const TaskEntry* GetNextEntry(uint32 context, uint32& inOutIndex) const;

        TaskEntry* GetFirstEntry(uint32 context, FName id, uint32& outIndex);
        const TaskEntry* GetFirstEntry(uint32 context, FName id, uint32& outIndex) const;

        TaskEntry* GetNextEntry(uint32 context, FName id, uint32& inOutIndex);
        const TaskEntry* GetNextEntry(uint32 context, FName id, uint32& inOutIndex) const;

        // Gets the context of a task, if it exists.
        TOptional<uint32> GetContext(TaskHandle handle) const;

        // Gets the type of a task, if it exists.
        TOptional<FName> GetType(TaskHandle handle) const;

        // Gets the id of a task, if it exists.
        TOptional<FName> GetId(TaskHandle handle) const;

        // Gets the next tick time of a task, if it exists.
        TOptional<Time> GetTickTime(TaskHandle handle) const;

        TOptional<Time> GetInterval(TaskHandle handle) const;
        bool SetInterval(TaskHandle handle, Time interval);

        TOptional<uint32> GetIntervalTicks(TaskHandle handle) const;
        bool SetIntervalTicks(TaskHandle handle, uint32 ticks);

        void* GetData(const TaskEntry* entry);
        const void* GetData(const TaskEntry* entry) const;

        void* GetData(TaskHandle handle);
        const void* GetData(TaskHandle handle) const;

        void Update(Time worldTime);

        auto begin() { return Entries.begin(); }
        auto begin() const { return Entries.begin(); }

        auto end() { return Entries.end(); }
        auto end() const { return Entries.end(); }

    private:

        Config Configuration;
        TFixedSortedList<TaskEntry, TaskEntry::GetItemKey> Entries;
        FixedBlockAllocator Data;
        uint32 TaskIdGen = 0;
    };
}
