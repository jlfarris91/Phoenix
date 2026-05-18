#pragma once

#include "PhoenixSim/Actions.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    struct Action;
}

namespace Phoenix::Tasks
{
    typedef void (*TaskCreateFunc)(WorldRef world, uint32 context, void* task);
    typedef void (*TaskUpdateFunc)(WorldRef world, uint32 context, void* task);
    typedef TOptional<Action> (*TaskActionFunc)(WorldRef world, uint32 context, void* task, const Action& action);
    typedef void (*TaskFinishFunc)(WorldRef world, uint32 context, void* task);
    
    template <class T>
    concept TaskHasOnCreate = requires(T task, WorldRef world, uint32 context)
    {
        { task.OnCreate(world, context) } -> std::same_as<void>;
    };

    template <class T>
    concept TaskHasOnUpdate = requires(T task, WorldRef world, uint32 context)
    {
        { task.OnUpdate(world, context) } -> std::same_as<void>;
    };

    template <class T>
    concept TaskHasOnAction = requires(T task, WorldRef world, uint32 context, const Action& action)
    {
        { task.OnAction(world, context, action) } -> std::same_as<TOptional<Action>>;
    };

    template <class T>
    concept TaskHasOnFinish = requires(T task, WorldRef world, uint32 context)
    {
        { task.OnFinish(world, context) } -> std::same_as<void>;
    };

    template <class T>
    concept IsTask = std::is_trivially_copyable_v<T> && (TaskHasOnCreate<T> || TaskHasOnUpdate<T> || TaskHasOnAction<T> || TaskHasOnFinish<T>);

    template <IsTask T>
    struct StaticTaskCallbacks
    {
        static void OnCreate(WorldRef world, uint32 context, void* task)
        {
            if constexpr (TaskHasOnCreate<T>)
            {
                T* typedTask = static_cast<T*>(task);
                typedTask->OnCreate(world, context);
            }
        }
        static void OnUpdate(WorldRef world, uint32 context, void* task)
        {
            if constexpr (TaskHasOnUpdate<T>)
            {
                T* typedTask = static_cast<T*>(task);
                typedTask->OnUpdate(world, context);
            }
        }
        static TOptional<Action> OnAction(WorldRef world, uint32 context, void* task, const Action& action)
        {
            if constexpr (TaskHasOnAction<T>)
            {
                T* typedTask = static_cast<T*>(task);
                return typedTask->OnAction(world, context, action);
            }
            else
            {
                return {};
            }
        }
        static void OnFinish(WorldRef world, uint32 context, void* task)
        {
            if constexpr (TaskHasOnFinish<T>)
            {
                T* typedTask = static_cast<T*>(task);
                typedTask->OnFinish(world, context);
            }
        }
    };

    struct PHOENIX_SIM_API TaskDefinition
    {
        FName TypeId;
        TaskCreateFunc OnCreate = nullptr;
        TaskUpdateFunc OnUpdate = nullptr;
        TaskActionFunc OnAction = nullptr;
        TaskFinishFunc OnFinish = nullptr;

        template <IsTask T>
        static TaskDefinition Create()
        {
            TaskDefinition definition;
            definition.TypeId = StaticTypeName<T>::TypeId;
            
            if constexpr (TaskHasOnCreate<T>)
            {
                definition.OnCreate = &StaticTaskCallbacks<T>::OnCreate;
            }

            if constexpr (TaskHasOnUpdate<T>)
            {
                definition.OnUpdate = &StaticTaskCallbacks<T>::OnUpdate;
            }

            if constexpr (TaskHasOnAction<T>)
            {
                definition.OnAction = &StaticTaskCallbacks<T>::OnAction;
            }

            if constexpr (TaskHasOnFinish<T>)
            {
                definition.OnFinish = &StaticTaskCallbacks<T>::OnFinish;
            }

            return definition;
        }
    };
}
