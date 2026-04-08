#include "JobScheduler.h"

#include <algorithm>
#include <queue>

#include "PhoenixSim/ECS/ArchetypeManager.h"
#include "PhoenixSim/ECS/CommandBuffer.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/Parallel.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

JobHandle JobScheduler::RegisterJob(std::unique_ptr<IJobBase> job)
{
    const JobHandle handle = static_cast<JobHandle>(Jobs.size());
    Jobs.push_back(std::move(job));
    return handle;
}

void JobScheduler::AddDependency(JobHandle after, JobHandle before)
{
    ExplicitEdges.emplace_back(after, before);
}

void JobScheduler::AddEdge(uint32 from, uint32 to)
{
    // Avoid duplicate edges
    for (uint32 suc : Nodes[from].Successors)
    {
        if (suc == to)
            return;
    }
    Nodes[from].Successors.push_back(to);
    ++Nodes[to].TotalPredecessors;
}

void JobScheduler::Build(const ArchetypeManager& archetypes)
{
    const uint32 numJobs = static_cast<uint32>(Jobs.size());

    Nodes.clear();
    Nodes.resize(numJobs);
    for (uint32 i = 0; i < numJobs; ++i)
    {
        Nodes[i].Job = Jobs[i].get();
        Nodes[i].TotalPredecessors = 0;
        Nodes[i].Successors.clear();
    }

    // Implicit edges: component access conflicts (earlier registration serializes first)
    for (uint32 i = 0; i < numJobs; ++i)
    {
        for (uint32 j = i + 1; j < numJobs; ++j)
        {
            if (Nodes[i].Job->GetAccessDescriptor().ConflictsWith(Nodes[j].Job->GetAccessDescriptor()))
            {
                AddEdge(i, j);
            }
        }
    }

    // Explicit edges declared via AddDependency
    for (auto&& [after, before] : ExplicitEdges)
    {
        if (after < numJobs && before < numJobs)
            AddEdge(before, after);  // before → after (before is a predecessor of after)
    }

    // Topological sort via Kahn's algorithm
    TopologicalOrder.clear();
    TopologicalOrder.reserve(numJobs);

    std::vector<uint32> inDegree(numJobs);
    for (uint32 i = 0; i < numJobs; ++i)
        inDegree[i] = Nodes[i].TotalPredecessors;

    std::queue<uint32> ready;
    for (uint32 i = 0; i < numJobs; ++i)
    {
        if (inDegree[i] == 0)
            ready.push(i);
    }

    while (!ready.empty())
    {
        uint32 idx = ready.front();
        ready.pop();
        TopologicalOrder.push_back(idx);
        for (uint32 suc : Nodes[idx].Successors)
        {
            if (--inDegree[suc] == 0)
                ready.push(suc);
        }
    }

    BuildBatches(archetypes);
}

void JobScheduler::RebuildBatchesIfDirty(const ArchetypeManager& archetypes)
{
    if (archetypes.GetGeneration() != LastGeneration)
        BuildBatches(archetypes);
}

void JobScheduler::BuildBatches(const ArchetypeManager& archetypes)
{
    LastGeneration = archetypes.GetGeneration();

    for (JobNode& node : Nodes)
    {
        node.Batches.clear();

        const SystemAccessDescriptor& desc = node.Job->GetAccessDescriptor();

        // Tasks have no component requirements — they get a sentinel batch below.
        if (!desc.Components.empty())
        {
            archetypes.ForEachArchetypeList([&](const FixedArchetypeList& list)
            {
                const ArchetypeDefinition& archDef = list.GetDefinition();

                for (const ComponentAccessEntry& entry : desc.Components)
                {
                    bool found = false;
                    for (const ComponentDefinition& comp : archDef)
                    {
                        if (comp.Id == entry.ComponentId)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        return;
                }

                JobBatch batch;
                batch.List = const_cast<FixedArchetypeList*>(&list);
                batch.NumComponents = static_cast<uint8>(desc.Components.size());
                for (uint8 i = 0; i < batch.NumComponents; ++i)
                {
                    batch.ComponentOffsets[i] = static_cast<uint16>(
                        list.GetComponentLocalOffset(desc.Components[i].ComponentId));
                }

                node.Batches.push_back(batch);
            });
        }

        // Tasks (and any job with no matching archetypes) get a sentinel batch so they
        // still participate in the execute loop and RunBatch is called once.
        if (node.Batches.empty())
            node.Batches.push_back(JobBatch{});
    }
}

JobNode& JobScheduler::GetNode(JobHandle handle)
{
    return Nodes[handle];
}

void JobScheduler::Execute(WorldConstRef world, TaskQueue& queue, const std::vector<CommandBuffer*>& commandBuffers)
{
    if (Nodes.empty())
        return;

    if (queue.GetNumWorkers() == 0)
    {
        ExecuteSerial(world, commandBuffers);
        return;
    }

    const uint32 numNodes = static_cast<uint32>(Nodes.size());
    std::vector<uint32> waveDepth(numNodes, 0);

    for (uint32 i = 0; i < numNodes; ++i)
    {
        uint32 idx = TopologicalOrder[i];
        for (uint32 suc : Nodes[idx].Successors)
            waveDepth[suc] = std::max(waveDepth[suc], waveDepth[idx] + 1);
    }

    uint32 maxWave = *std::ranges::max_element(waveDepth);

    for (uint32 wave = 0; wave <= maxWave; ++wave)
    {
        std::vector<Task>& tasks = queue.BeginGroup();
        for (uint32 i = 0; i < numNodes; ++i)
        {
            if (waveDepth[i] != wave)
                continue;
            JobNode* node = &Nodes[i];
            tasks.emplace_back([node, &commandBuffers, worldPtr = &world]
            {
                CommandBuffer* cb = commandBuffers[GetCurrentThreadIndex()];
                for (const JobBatch& batch : node->Batches)
                    node->Job->RunBatch(*worldPtr, batch, *cb);
            });
        }
        queue.EndGroup();
        queue.Flush();
    }
}

void JobScheduler::ExecuteSerial(WorldConstRef world, const std::vector<CommandBuffer*>& commandBuffers)
{
    CommandBuffer* cb = commandBuffers[GetCurrentThreadIndex()];
    for (uint32 idx : TopologicalOrder)
    {
        JobNode& node = Nodes[idx];
        for (const JobBatch& batch : node.Batches)
            node.Job->RunBatch(world, batch, *cb);
    }
}
