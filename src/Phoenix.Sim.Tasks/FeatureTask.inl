#pragma once

#include "FeatureTask.h"

namespace Phoenix::Tasks
{
    template <class T, class ...TArgs>
    struct TaskExecuteHelper<false, T, void(T::*)(TArgs...)>
    {
        using ReturnType = void;
        static ReturnType Execute(WorldRef world, TaskHandle handle, void(T::*fn)(TArgs...), TArgs... args)
        {
            FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
            if (!block)
            {
                return;
            }
            void* data = block->Tasks.GetData(handle);
            if (!data)
            {
                return;
            }
            FeatureTask::ScopedTaskStack _(handle);
            T* typedData = static_cast<T*>(data);
            std::invoke(fn, *typedData, args...);
        }
    };

    template <class T, class ...TArgs>
    struct TaskExecuteHelper<true, T, void(T::*)(TArgs...) const>
    {
        using ReturnType = void;
        static ReturnType Execute(WorldRef world, TaskHandle handle, void(T::*fn)(TArgs...) const, TArgs... args)
        {
            const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
            if (!block)
            {
                return;
            }
            const void* data = block->Tasks.GetData(handle);
            if (!data)
            {
                return;
            }
            FeatureTask::ScopedTaskStack _(handle);
            const T* typedData = static_cast<const T*>(data);
            std::invoke(fn, *typedData, args...);
        }
    };

    template <class T, class TRet, class ...TArgs>
    struct TaskExecuteHelper<false, T, TRet(T::*)(TArgs...)>
    {
        using ReturnType = TOptional<TRet>;
        static ReturnType Execute(WorldRef world, TaskHandle handle, TRet(T::*fn)(TArgs...), TArgs... args)
        {
            FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
            if (!block)
            {
                return {};
            }
            void* data = block->Tasks.GetData(handle);
            if (!data)
            {
                return {};
            }
            FeatureTask::ScopedTaskStack _(handle);
            T* typedData = static_cast<T*>(data);
            return std::invoke(fn, *typedData, args...);
        }
    };

    template <class T, class TRet, class ...TArgs>
    struct TaskExecuteHelper<true, T, TRet(T::*)(TArgs...) const>
    {
        using ReturnType = TOptional<TRet>;
        static ReturnType Execute(WorldConstRef world, TaskHandle handle, TRet(T::*fn)(TArgs...) const, TArgs... args)
        {
            const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
            if (!block)
            {
                return {};
            }
            const void* data = block->Tasks.GetData(handle);
            if (!data)
            {
                return {};
            }
            FeatureTask::ScopedTaskStack _(handle);
            const T* typedData = static_cast<const T*>(data);
            return std::invoke(fn, *typedData, args...);
        }
    };

    template <class T, class TRet, class ...TArgs>
    auto FeatureTask::Execute(WorldRef world, TaskHandle handle, TRet(T::*fn)(TArgs...), TArgs... args)
    {
        using Helper = TaskExecuteHelper<false, T, TRet(T::*)(TArgs...)>;
        if constexpr (std::is_void_v<typename Helper::ReturnType>)
        {
            Helper::Execute(world, handle, fn, args...);
        }
        else
        {
            return Helper::Execute(world, handle, fn, args...);
        }
    }

    template <class T, class TRet, class ...TArgs>
    auto FeatureTask::Execute(WorldConstRef world, TaskHandle handle, TRet(T::*fn)(TArgs...) const, TArgs... args)
    {
        using Helper = TaskExecuteHelper<true, T, TRet(T::*)(TArgs...) const>;
        if constexpr (std::is_void_v<typename Helper::ReturnType>)
        {
            Helper::Execute(world, handle, fn, args...);
        }
        else
        {
            return Helper::Execute(world, handle, fn, args...);
        }
    }
}