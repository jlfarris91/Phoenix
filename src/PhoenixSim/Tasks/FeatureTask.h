#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/Tasks/TaskDefinition.h"
#include "PhoenixSim/Tasks/TaskList.h"

#ifndef PHX_TASKS_MAX_TASKS
#define PHX_TASKS_MAX_TASKS (32768 * 4)
#endif

#ifndef PHX_TASKS_MAX_TASK_SIZE
#define PHX_TASKS_MAX_TASK_SIZE 64
#endif

namespace Phoenix::Tasks
{
    class FixedTaskList;

    struct PHOENIX_SIM_API FeatureTaskDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureTaskDynamicBlock)

        struct Config
        {
            uint32 MaxTasks = 0;
            uint32 MaxTaskSize = 0;
        };

        FixedTaskList Tasks;
    };

    template <bool bConst, class T, class Fn>
    struct TaskExecuteHelper
    {
        using ReturnType = void;
    };

    class PHOENIX_SIM_API FeatureTask : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureTask)
        {
            FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        }

        static void RegisterTaskDefinition(const TaskDefinition& definition);

        template <class T>
        static void RegisterTaskDefinition()
        {
            RegisterTask(TaskDefinition::Create<T>());
        }

        // Allocates a new task with the given type and data, returning a handle to the task.
        // The task is created within the given context, which is used to group related tasks together.
        static TaskHandle Allocate(
            WorldRef world,
            uint32 context,
            FName type,
            const void* data,
            uint32 size);

        // Allocates a new task with the given type and data, returning a handle to the task.
        // The task is created within the given context, which is used to group related tasks together.
        static TaskHandle Allocate(
            WorldRef world,
            uint32 context,
            FName id,
            FName type,
            const void* data,
            uint32 size);

        template <IsTask T>
        static TaskHandle Allocate(WorldRef world, uint32 context, const T& taskData = T{})
        {
            return Allocate(world, context, StaticTypeName<T>::TypeId, &taskData, sizeof(T));
        }

        template <IsTask T>
        static TaskHandle Allocate(WorldRef world, uint32 context, FName id, const T& taskData = T{})
        {
            return Allocate(world, context, id, StaticTypeName<T>::TypeId, &taskData, sizeof(T));
        }

        // Finishes a task with the given handle, explicitly removing it from the engine.
        static bool FinishTask(WorldRef world, TaskHandle handle);

        // Finishes all tasks in the given context and removes them from the engine.
        static uint32 FinishAllTasks(WorldRef world, uint32 context);

        // Finishes all tasks with the given id in the given context and removes them from the engine.
        static uint32 FinishAllTasks(WorldRef world, uint32 context, FName id);

        // Returns true if the context has a task with the given id.
        static bool HasTask(WorldConstRef world, uint32 context, FName id);

        // Gets the handle of the first task within the given context, if any exist.
        static TaskHandle GetFirstTask(WorldConstRef world, uint32 context, uint32& outIndex);

        // Gets the handle of the next task within the given context, if any exist.
        static TaskHandle GetNextTask(WorldConstRef world, uint32 context, uint32& inOutIndex);

        // Gets the handle of the first task with the given id within the given context, if any exist.
        static TaskHandle GetFirstTask(WorldConstRef world, uint32 context, FName id, uint32& outIndex);

        // Gets the handle of the next task with the given id within the given context, if any exist.
        static TaskHandle GetNextTask(WorldConstRef world, uint32 context, FName id, uint32& inOutIndex);

        template <class T>
        static TaskHandle GetFirstTask(WorldConstRef world, uint32 context)
        {
            uint32 index;
            return GetFirstTask<T>(world, context, index);
        }

        template <class T>
        static TaskHandle GetFirstTask(WorldConstRef world, uint32 context, uint32& inOutIndex)
        {
            return GetFirstTask(world, context, StaticTypeName<T>::TypeId, inOutIndex);
        }

        template <class T>
        static TaskHandle GetNextTask(WorldConstRef world, uint32 context, uint32& inOutIndex)
        {
            return GetNextTask(world, context, StaticTypeName<T>::TypeId, inOutIndex);
        }

        // Gets the context of a task with the given handle, if it exists.
        static TOptional<uint32> GetTaskContext(WorldConstRef world, TaskHandle handle);

        // Gets the id of a task with the given handle, if it exists.
        static TOptional<FName> GetTaskId(WorldConstRef world, TaskHandle handle);

        // Gets the type of a task with the given handle, if it exists.
        static TOptional<FName> GetTaskType(WorldConstRef world, TaskHandle handle);

        // Gets the next tick time for a task with the given handle, if it exists.
        static TOptional<Time> GetTaskTickTime(WorldConstRef world, TaskHandle handle);

        // Gets the update frequency for a task with the given handle, if it exists.
        static TOptional<Time> GetTaskInterval(WorldConstRef world, TaskHandle handle);

        // Gets the update frequency for a task with the given handle, if it exists.
        static TOptional<uint32> GetTaskIntervalTicks(WorldConstRef world, TaskHandle handle);

        // Sets the update frequency for a task with the given handle.
        static bool SetTaskInterval(WorldRef world, TaskHandle handle, Time interval);

        // Sets the update frequency for a task with the given handle.
        static bool SetTaskIntervalTicks(WorldRef world, TaskHandle handle, uint32 ticks);

        // Send an action to a task with the given id within the given context.
        // If multiple tasks match the context and id, the action is sent to each task until one returns an action.
        // Returns the response from the task, if any.
        static TOptional<Action> SendAction(WorldRef world, uint32 context, FName id, const Action& action);

        // Sends an action to a specific task with the given handle.
        // Returns the response from the task, if any.
        static TOptional<Action> SendAction(WorldRef world, TaskHandle handle, const Action& action);

        // Callback type for receiving results from tasks in response to a broadcasted action.
        typedef std::function<void(WorldRef, uint32, FName, const Action&)> ActionResultCallback;

        // Sends an action to all tasks within the given context.
        // Returns the number of tasks that received the action.
        static uint32 BroadcastAction(
            WorldRef world,
            uint32 context,
            const Action& action,
            const ActionResultCallback& callback = nullptr);

        // Sends an action to all tasks with the given id within the given context.
        // Returns the number of tasks that received the action.
        static uint32 BroadcastAction(
            WorldRef world,
            uint32 context,
            FName id,
            const Action& action,
            const ActionResultCallback& callback = nullptr);

        // Calls a function on a task with the given handle, if the task exists and has the given function.
        template <class T, class TRet, class ...TArgs>
        static auto Execute(WorldRef world, TaskHandle handle, TRet (T::*fn)(TArgs...), TArgs... args);

        // Calls a function on a task with the given handle, if the task exists and has the given function.
        template <class T, class TRet, class ...TArgs>
        static auto Execute(WorldConstRef world, TaskHandle handle, TRet (T::*fn)(TArgs...) const, TArgs... args);

        //
        //  Utility functions for querying and interacting with the current executing task.
        //

        // Returns true if the current code is executing within the context of a task.
        static bool IsExecutingTask();

        // Gets the handle of the current task being executed.
        static TaskHandle GetSelfTask();

        // Gets the context of the current task being executed.
        static uint32 GetSelfTaskContext(WorldConstRef world);

        // Gets the id of the current task being executed.
        static FName GetSelfTaskId(WorldConstRef world);

        // Gets the type of the current task being executed.
        static FName GetSelfTaskType(WorldConstRef world);

        // Gets the update frequency for the current task being executed.
        static Time GetSelfInterval(WorldConstRef world);

        // Gets the update frequency for the current task being executed.
        static uint32 GetSelfIntervalTicks(WorldConstRef world);

        // Sets the update frequency for the current task being executed.
        static void SetSelfInterval(WorldRef world, Time interval);

        // Sets the update frequency for the current task being executed.
        static void SetSelfIntervalTicks(WorldRef world, uint32 ticks);

        // Finishes the current task being executed, explicitly removing it from the engine.
        static void SelfFinishTask(WorldRef world);

    private:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        void ExecuteTaskOnCreate(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry);
        void ExecuteTaskOnUpdate(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry);
        TOptional<Action> ExecuteTaskOnAction(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry, const Action& action);
        bool ExecuteTaskOnFinish(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry);

        static void ThrowIfCurrentTaskIsInvalid();

        static std::unordered_map<FName, TaskDefinition> Definitions;

        template <bool bConst, class T, class Fn>
        friend struct TaskExecuteHelper;

        struct ScopedTaskStack
        {
            ScopedTaskStack(TaskHandle newHandle);
            ~ScopedTaskStack();
            TaskHandle PreviousTask;
        };

        thread_local static TaskHandle CurrentTask;
    };
}

#include "FeatureTask.inl"