#pragma once

#include "Phoenix.Sim/WorldsFwd.h"
#include "Phoenix.Sim/FixedPoint/FixedTypes.h"
#include "Phoenix.Sim/Tasks/TaskDefinition.h"

namespace Phoenix::Tasks
{
    struct PHOENIX_SIM_API TaskBase
    {
        PHX_DECLARE_TYPE(TaskBase)
        static uint32 GetContext(WorldConstRef world);
        static FName GetTaskId(WorldConstRef world);
        static FName GetTaskType(WorldConstRef world);
        static Time GetInterval(WorldConstRef world);
        static void SetInterval(WorldRef world, Time interval);
        static uint32 GetIntervalTicks(WorldConstRef world);
        static void SetIntervalTicks(WorldRef world, uint32 ticks);
        static void FinishTask(WorldRef world);
    };
}

#define PHX_DECLARE_TASK(type) \
    PHX_DECLARE_TYPE(type, Phoenix::Tasks::TaskBase) \
    static Phoenix::Tasks::TaskDefinition GetTaskDefinition() { \
        return Phoenix::Tasks::TaskDefinition::Create<type>(); \
    } \
    private: \
    inline static const bool _s_phx_type_init_task = ( \
        [] { Phoenix::TypeDescriptorBuilder<type>{}.StaticMethod("GetTaskDefinition", &type::GetTaskDefinition); }(), true); \
    public: