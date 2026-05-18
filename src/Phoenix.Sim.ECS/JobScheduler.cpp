#include "JobScheduler.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <thread>

#include "Phoenix.Sim.ECS/ArchetypeManager.h"
#include "Phoenix.Sim.ECS/CommandBuffer.h"
#include "Phoenix.Sim.ECS/SystemJob.h"
#include "Phoenix/Parallel.h"
#include "Phoenix.Sim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

JobScheduler::JobScheduler(const std::string& name)
    : Name(name)
{
}

JobHandle JobScheduler::RegisterJob(IJobBase* job)
{
    const JobHandle handle = static_cast<JobHandle>(Jobs.size());
    Jobs.push_back(job);
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
        Nodes[i].Job = Jobs[i];
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

const std::string& JobScheduler::GetName() const
{
    return Name;
}

uint32 JobScheduler::GetJobCount() const
{
    return (uint32)Jobs.size();
}

const char* JobScheduler::GetJobName(uint32 index) const
{
    return Jobs[index]->GetName();
}

const SystemAccessDescriptor& JobScheduler::GetJobAccess(uint32 index) const
{
    return Jobs[index]->GetAccessDescriptor();
}

const std::vector<uint32>& JobScheduler::GetJobSuccessors(uint32 index) const
{
    return Nodes[index].Successors;
}

uint32 JobScheduler::GetJobPredecessorCount(uint32 index) const
{
    return Nodes[index].TotalPredecessors;
}

void JobScheduler::Execute(WorldConstRef world, TaskQueue& queue, const std::vector<CommandBuffer*>& commandBuffers)
{
    const char* schedulerName = GetName().c_str();
    size_t schedulerNameLen = std::strlen(schedulerName);

    PHX_PROFILE_ZONE_SCOPED;
    PHX_PROFILE_ZONE_NAME(schedulerName, schedulerNameLen);

    if (Nodes.empty())
        return;

    if (queue.GetNumWorkers() == 0)
    {
        ExecuteSerial(world, commandBuffers);
        return;
    }

    const uint32 numNodes = static_cast<uint32>(Nodes.size());
    ThreadPool* pool = queue.GetThreadPool();

    // totalRemaining counts every batch that has been submitted and not yet completed.
    // It is incremented inside submitNode (after any predecessor may have modified Batches)
    // and decremented at the end of each batch lambda.
    // When it reaches zero every lambda has fully executed and it is safe to return.
    std::atomic<uint32> totalRemaining{0};

    // Per-node remaining-batch count. Set inside submitNode so it reflects the batch list
    // AFTER predecessors have had a chance to modify it (e.g. PrePartitionSortedEntitiesTask
    // rewrites PopulateSortedEntitiesJob's Batches before Populate is submitted).
    std::vector<std::atomic<uint32>> remainingBatches(numNodes);

    for (uint32 i = 0; i < numNodes; ++i)
        Nodes[i].RemainingPredecessors.store(Nodes[i].TotalPredecessors, std::memory_order_relaxed);

    // Submit all batches for a node directly to the thread pool.
    // When the last batch of a node finishes it atomically decrements each successor's
    // predecessor count and immediately submits any that reach zero — no wave barriers.
    std::function<void(uint32)> submitNode = [&](uint32 nodeIdx)
    {
        JobNode* node = &Nodes[nodeIdx];

        // Read the batch list and set the counter here — after all predecessors have run —
        // so runtime modifications to Batches (e.g. PrePartition) are fully visible.
        const uint32 batchCount = static_cast<uint32>(node->Batches.size());
        remainingBatches[nodeIdx].store(batchCount, std::memory_order_relaxed);
        totalRemaining.fetch_add(batchCount, std::memory_order_release);

        for (const JobBatch& batch : node->Batches)
        {
            pool->Submit([&, node, batch, nodeIdx]
            {
                CommandBuffer* cb = commandBuffers[GetCurrentThreadIndex()];
                node->Job->RunBatch(world, batch, *cb);

                // Last batch for this node → release successors
                if (remainingBatches[nodeIdx].fetch_sub(1, std::memory_order_acq_rel) == 1)
                {
                    for (uint32 suc : node->Successors)
                    {
                        if (Nodes[suc].RemainingPredecessors.fetch_sub(1, std::memory_order_acq_rel) == 1)
                            submitNode(suc);
                    }
                }

                // Signal completion after successor submissions so totalRemaining
                // only hits zero once every lambda body has finished executing.
                totalRemaining.fetch_sub(1, std::memory_order_acq_rel);
            });
        }
    };

    // Kick off all root nodes (no predecessors)
    for (uint32 i = 0; i < numNodes; ++i)
    {
        if (Nodes[i].TotalPredecessors == 0)
            submitNode(i);
    }

    // Spin until every batch lambda has completed.
    // totalRemaining is only decremented after all successor submissions within a lambda,
    // so zero means the full call graph is done and stack locals are safe to destroy.
    while (totalRemaining.load(std::memory_order_acquire) != 0)
        std::this_thread::yield();
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
