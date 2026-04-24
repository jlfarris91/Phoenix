#include "TaskBase.h"

#include "FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::Tasks;

uint32 TaskBase::GetContext(WorldConstRef world)
{
    return FeatureTask::GetSelfTaskContext(world);
}

FName TaskBase::GetTaskId(WorldConstRef world)
{
    return FeatureTask::GetSelfTaskId(world);
}

FName TaskBase::GetTaskType(WorldConstRef world)
{
    return FeatureTask::GetSelfTaskType(world);
}

Time TaskBase::GetInterval(WorldConstRef world)
{
    return FeatureTask::GetSelfInterval(world);
}

void TaskBase::SetInterval(WorldRef world, Time interval)
{
    return FeatureTask::SetSelfInterval(world, interval);
}

uint32 TaskBase::GetIntervalTicks(WorldConstRef world)
{
    return FeatureTask::GetSelfIntervalTicks(world);
}

void TaskBase::SetIntervalTicks(WorldRef world, uint32 ticks)
{
    return FeatureTask::SetSelfIntervalTicks(world, ticks);
}

void TaskBase::FinishTask(WorldRef world)
{
    FeatureTask::SelfFinishTask(world);
}
