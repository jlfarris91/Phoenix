#include "FeatureTask.h"

#include "TaskBase.h"
#include "TaskList.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::Tasks;

thread_local TaskHandle FeatureTask::CurrentTask = TaskHandle::Invalid;
std::unordered_map<FName, TaskDefinition> FeatureTask::Definitions;

FeatureTaskDynamicBlock::FeatureTaskDynamicBlock(BlockBufferAllocator& allocator, const Config& config)
    : Tasks(allocator, { config.MaxTasks, config.MaxTaskSize })
{
}

FeatureTaskDynamicBlock::FeatureTaskDynamicBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureTaskDynamicBlock& other)
    : Tasks(allocator, { .MaxTasks = config.MaxTasks, .MaxTaskSize = config.MaxTaskSize }, other.Tasks)
{
}

BufferBlockLayout FeatureTaskDynamicBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureTaskDynamicBlock);
    layout.AllocSize = FixedTaskList::GetAllocSizeBytes({ config.MaxTasks, config.MaxTaskSize });
    return layout;
}

void FeatureTaskDynamicBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureTaskDynamicBlock(allocator, config);
}

void FeatureTask::RegisterTaskDefinition(const TaskDefinition& definition)
{
    Definitions[definition.TypeId] = definition;
}

TaskHandle FeatureTask::Allocate(WorldRef world, uint32 context, FName type, const void* data, uint32 size)
{
    return Allocate(world, context, type, type, data, size);
}

TaskHandle FeatureTask::Allocate(
    WorldRef world,
    uint32 context,
    FName id,
    FName type,
    const void* data,
    uint32 size)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return TaskHandle::Invalid;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return TaskHandle::Invalid;
    }

    TaskHandle handle = block->Tasks.Allocate(context, id, type, data, size);
    if (handle == TaskHandle::Invalid)
    {
        return TaskHandle::Invalid;
    }

    TaskEntry* taskEntry = block->Tasks.GetEntry(handle);
    PHX_ASSERT(taskEntry);

    feature->ExecuteTaskOnCreate(world, block->Tasks, *taskEntry);

    return handle;
}

bool FeatureTask::FinishTask(WorldRef world, TaskHandle handle)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return false;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    TaskEntry* entry = block->Tasks.GetEntry(handle);
    if (!entry)
    {
        return false;
    }

    if (!feature->ExecuteTaskOnFinish(world, block->Tasks, *entry))
    {
        return false;
    }

    if (!block->Tasks.Deallocate(handle))
    {
        return false;
    }

    return true;
}

bool FeatureTask::FinishTask(WorldRef world, uint32 context, FName id)
{
    uint32 index;
    TaskHandle handle = GetFirstTask(world, context, id, index);
    if (handle == TaskHandle::Invalid)
    {
        return false;
    }
    return FinishTask(world, handle);
}

uint32 FeatureTask::FinishAllTasks(WorldRef world, uint32 context)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return false;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    uint32 index;
    TaskEntry* entry = block->Tasks.GetFirstEntry(context, index);

    uint32 count = 0;
    while (entry)
    {
        feature->ExecuteTaskOnFinish(world, block->Tasks, *entry);
        if (block->Tasks.Deallocate(entry->GetHandle()))
        {
            ++count;
        }
        entry = block->Tasks.GetNextEntry(context, index);
    }

    return count;
}

uint32 FeatureTask::FinishAllTasks(WorldRef world, uint32 context, FName id)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return false;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    uint32 index;
    TaskEntry* entry = block->Tasks.GetFirstEntry(context, id, index);

    uint32 count = 0;
    while (entry)
    {
        feature->ExecuteTaskOnFinish(world, block->Tasks, *entry);
        if (block->Tasks.Deallocate(entry->GetHandle()))
        {
            ++count;
        }
        entry = block->Tasks.GetNextEntry(context, id, index);
    }

    return count;
}

bool FeatureTask::HasTask(WorldConstRef world, uint32 context, FName id)
{
    uint32 index;
    return GetFirstTask(world, context, id, index) != TaskHandle::Invalid;
}

TaskHandle FeatureTask::GetFirstTask(WorldConstRef world, uint32 context, uint32& outIndex)
{
    if (const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>())
    {
        if (const TaskEntry* entry = block->Tasks.GetFirstEntry(context, outIndex))
        {
            return entry->GetHandle();
        }
    }
    return TaskHandle::Invalid;
}

TaskHandle FeatureTask::GetNextTask(WorldConstRef world, uint32 context, uint32& inOutIndex)
{
    if (const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>())
    {
        if (const TaskEntry* entry = block->Tasks.GetNextEntry(context, inOutIndex))
        {
            return entry->GetHandle();
        }
    }
    return TaskHandle::Invalid;
}

TaskHandle FeatureTask::GetFirstTask(WorldConstRef world, uint32 context, FName id, uint32& outIndex)
{
    if (const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>())
    {
        if (const TaskEntry* entry = block->Tasks.GetFirstEntry(context, id, outIndex))
        {
            return entry->GetHandle();
        }
    }
    return TaskHandle::Invalid;
}

TaskHandle FeatureTask::GetNextTask(WorldConstRef world, uint32 context, FName id, uint32& inOutIndex)
{
    if (const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>())
    {
        if (const TaskEntry* entry = block->Tasks.GetNextEntry(context, id, inOutIndex))
        {
            return entry->GetHandle();
        }
    }
    return TaskHandle::Invalid;
}

TOptional<uint32> FeatureTask::GetTaskContext(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetContext(handle) : TOptional<uint32>();
}

TOptional<FName> FeatureTask::GetTaskId(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetId(handle) : TOptional<FName>();
}

TOptional<FName> FeatureTask::GetTaskType(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetType(handle) : TOptional<FName>();
}

TOptional<Time> FeatureTask::GetTaskTickTime(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetTickTime(handle) : TOptional<Time>();
}

TOptional<Time> FeatureTask::GetTaskInterval(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetInterval(handle) : TOptional<Time>();
}

TOptional<uint32> FeatureTask::GetTaskIntervalTicks(WorldConstRef world, TaskHandle handle)
{
    const FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    return block ? block->Tasks.GetIntervalTicks(handle) : TOptional<uint32>();
}

bool FeatureTask::SetTaskInterval(WorldRef world, TaskHandle handle, Time interval)
{
    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->Tasks.SetInterval(handle, interval);
}

bool FeatureTask::SetTaskIntervalTicks(WorldRef world, TaskHandle handle, uint32 ticks)
{
    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    return block->Tasks.SetIntervalTicks(handle, ticks);
}

TOptional<Action> FeatureTask::SendAction(WorldRef world, uint32 context, FName id, const Action& action)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return {};
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return {};
    }

    uint32 index;
    TOptional<Action> result;
    TaskEntry* entry = block->Tasks.GetFirstEntry(context, id, index);

    // Search all tasks with the given context and id for one that will handle the action.
    while (entry)
    {
        result = feature->ExecuteTaskOnAction(world, block->Tasks, *entry, action);
        if (result.IsSet())
        {
            break;
        }
        entry = block->Tasks.GetNextEntry(context, id, index);
    }

    return result;
}

TOptional<Action> FeatureTask::SendAction(WorldRef world, TaskHandle handle, const Action& action)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return {};
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return {};
    }

    TaskEntry* entry = block->Tasks.GetEntry(handle);
    if (!entry)
    {
        return {};
    }

    return feature->ExecuteTaskOnAction(world, block->Tasks, *entry, action);
}

uint32 FeatureTask::BroadcastAction(
    WorldRef world,
    uint32 context,
    const Action& action,
    const ActionResultCallback& callback)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return false;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    uint32 index;
    TaskEntry* entry = block->Tasks.GetFirstEntry(context, index);
    if (!entry)
    {
        return false;
    }

    // Send the action to all tasks within the given context.
    uint32 count = 0;
    while (entry)
    {
        TOptional<Action> result = feature->ExecuteTaskOnAction(world, block->Tasks, *entry, action);
        if (result.IsSet() && callback)
        {
            callback(world, context, entry->GetId(), result.Get());
        }
        entry = block->Tasks.GetNextEntry(context, index);
        ++count;
    }

    return count;
}

uint32 FeatureTask::BroadcastAction(
    WorldRef world,
    uint32 context,
    FName id,
    const Action& action,
    const ActionResultCallback& callback)
{
    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return false;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return false;
    }

    uint32 index;
    TaskEntry* entry = block->Tasks.GetFirstEntry(context, id, index);
    if (!entry)
    {
        return false;
    }

    // Send the action to all tasks within the given context and id.
    uint32 count = 0;
    while (entry)
    {
        TOptional<Action> result = feature->ExecuteTaskOnAction(world, block->Tasks, *entry, action);
        if (result.IsSet() && callback)
        {
            callback(world, context, id, result.Get());
        }
        entry = block->Tasks.GetNextEntry(context, id, index);
        ++count;
    }

    return count;
}

bool FeatureTask::IsExecutingTask()
{
    return CurrentTask != TaskHandle::Invalid;
}

TaskHandle FeatureTask::GetSelfTask()
{
    ThrowIfCurrentTaskIsInvalid();
    return CurrentTask;
}

uint32 FeatureTask::GetSelfTaskContext(WorldConstRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    return GetTaskContext(world, GetSelfTask()).Get();
}

FName FeatureTask::GetSelfTaskId(WorldConstRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    return GetTaskId(world, GetSelfTask()).Get();
}

FName FeatureTask::GetSelfTaskType(WorldConstRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    return GetTaskType(world, GetSelfTask()).Get();
}

Time FeatureTask::GetSelfInterval(WorldConstRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    return GetTaskInterval(world, GetSelfTask()).Get();
}

uint32 FeatureTask::GetSelfIntervalTicks(WorldConstRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    return GetTaskIntervalTicks(world, GetSelfTask()).Get();
}

void FeatureTask::SetSelfInterval(WorldRef world, Time interval)
{
    ThrowIfCurrentTaskIsInvalid();
    SetTaskInterval(world, GetSelfTask(), interval);
}

void FeatureTask::SetSelfIntervalTicks(WorldRef world, uint32 ticks)
{
    ThrowIfCurrentTaskIsInvalid();
    SetTaskIntervalTicks(world, GetSelfTask(), ticks);
}

void FeatureTask::SelfFinishTask(WorldRef world)
{
    ThrowIfCurrentTaskIsInvalid();
    FinishTask(world, GetSelfTask());
}

void FeatureTask::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    auto taskDescriptors = TypeRegistry::GetAllDerivedFrom<TaskBase>();
    for (const TypeDescriptor* taskDescriptor : taskDescriptors)
    {
        auto methods = taskDescriptor->GetMethods();
        Variant taskDefinition = methods["GetTaskDefinition"].Execute();
        RegisterTaskDefinition(*taskDefinition.GetData<TaskDefinition>());
    }
}

void FeatureTask::OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureTaskDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxTasks = PHX_TASKS_MAX_TASKS;
    dynamicBlockConfig.MaxTaskSize = PHX_TASKS_MAX_TASK_SIZE;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();

        dynamicBlockConfig.MaxTasks = featureConfigData.value("max_tasks", dynamicBlockConfig.MaxTasks);
        dynamicBlockConfig.MaxTaskSize = featureConfigData.value("max_task_size", dynamicBlockConfig.MaxTaskSize);
    }

    builder.RegisterBlockWithAlloc<FeatureTaskDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
}

void FeatureTask::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnWorldUpdate(world, args);

    std::shared_ptr<FeatureTask> feature = GetFeature<FeatureTask>(world);
    if (!feature)
    {
        return;
    }

    FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
    if (!block)
    {
        return;
    }

    {
        PHX_PROFILE_ZONE_SCOPED_N("ExecuteTasks");

        uint32 count = 0;
        for (TaskEntry& entry : block->Tasks)
        {
            if (entry.IsValid() && entry.GetTickTime() > 0 && entry.GetTickTime() <= world.GetSimTime())
            {
                feature->ExecuteTaskOnUpdate(world, block->Tasks, entry);
                ++count;
            }
        }

        PHX_PROFILE_ZONE_VALUE(count);
    }
}

void FeatureTask::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    {
        PHX_PROFILE_ZONE_SCOPED_N("SortAndCompact");
        FeatureTaskDynamicBlock* block = world.GetBlock<FeatureTaskDynamicBlock>();
        block->Tasks.Update(world.GetSimTime());
    }
}

void FeatureTask::ExecuteTaskOnCreate(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry)
{
    PHX_PROFILE_ZONE_SCOPED;

    void* data = tasks.GetData(&entry);
    if (!data)
    {
        return;
    }

    const TaskDefinition& definition = Definitions[entry.GetType()];
    if (definition.OnCreate)
    {
        ScopedTaskStack _(entry.GetHandle());
        definition.OnCreate(world, entry.GetContext(), data);
    }
}

void FeatureTask::ExecuteTaskOnUpdate(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry)
{
    PHX_PROFILE_ZONE_SCOPED;

    void* data = tasks.GetData(&entry);
    if (!data)
    {
        return;
    }

    const TaskDefinition& definition = Definitions[entry.GetType()];
    if (definition.OnUpdate)
    {
        ScopedTaskStack _(entry.GetHandle());
        definition.OnUpdate(world, entry.GetContext(), data);
    }
}

TOptional<Action> FeatureTask::ExecuteTaskOnAction(
    WorldRef world,
    FixedTaskList& tasks,
    const TaskEntry& entry,
    const Action& action)
{
    PHX_PROFILE_ZONE_SCOPED;

    void* data = tasks.GetData(&entry);
    if (!data)
    {
        return {};
    }

    TOptional<Action> result;

    const TaskDefinition& definition = Definitions[entry.GetType()];
    if (definition.OnAction)
    {
        ScopedTaskStack _(entry.GetHandle());
        result = definition.OnAction(world, entry.GetContext(), data, action);
    }

    return result;
}

bool FeatureTask::ExecuteTaskOnFinish(WorldRef world, FixedTaskList& tasks, const TaskEntry& entry)
{
    PHX_PROFILE_ZONE_SCOPED;

    void* data = tasks.GetData(&entry);
    if (!data)
    {
        return false;
    }

    const TaskDefinition& definition = Definitions[entry.GetType()];
    if (definition.OnFinish)
    {
        ScopedTaskStack _(entry.GetHandle());
        definition.OnFinish(world, entry.GetContext(), data);
    }

    return true;
}

void FeatureTask::ThrowIfCurrentTaskIsInvalid()
{
    if (CurrentTask == TaskHandle::Invalid)
    {
        throw std::runtime_error("Function must be called within the context of a task.");
    }
}

FeatureTask::ScopedTaskStack::ScopedTaskStack(TaskHandle newHandle)
    : PreviousTask(CurrentTask)
{
    CurrentTask = newHandle;
}

FeatureTask::ScopedTaskStack::~ScopedTaskStack()
{
    CurrentTask = PreviousTask;
}
