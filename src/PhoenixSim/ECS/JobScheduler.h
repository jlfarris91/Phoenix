#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#include "JobBatch.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class TaskQueue;
}

namespace Phoenix::ECS
{
    class CommandBuffer;
    class ArchetypeManager;
    class IJobBase;

    using JobHandle = uint32;
    static constexpr JobHandle InvalidJobHandle = UINT32_MAX;

    struct JobNode
    {
        IJobBase*               Job;
        std::vector<JobBatch>   Batches;
        std::vector<uint32>     Successors;
        uint32                  TotalPredecessors = 0;
        std::atomic<uint32>     RemainingPredecessors = 0;
    };

    class JobScheduler
    {
    public:
        // Register a job and return its handle. Must be called before Build().
        JobHandle RegisterJob(std::unique_ptr<IJobBase> job);

        // Declare that 'after' must not start until 'before' has completed.
        // Must be called before Build().
        void AddDependency(JobHandle after, JobHandle before);

        // Build the dependency graph and topological order.
        // Call once after all RegisterJob/AddDependency calls.
        void Build(const ArchetypeManager& archetypes);

        void RebuildBatchesIfDirty(const ArchetypeManager& archetypes);
        void Execute(WorldConstRef world, TaskQueue& queue, const std::vector<CommandBuffer*>& commandBuffers);
        void ExecuteSerial(WorldConstRef world, const std::vector<CommandBuffer*>& commandBuffers);

        // Access a node after Build() — e.g. to read Batches from a dependent task.
        JobNode& GetNode(JobHandle handle);

    private:
        void BuildBatches(const ArchetypeManager& archetypes);
        void AddEdge(uint32 from, uint32 to);

        std::vector<std::unique_ptr<IJobBase>>  Jobs;
        std::vector<JobNode>                    Nodes;
        std::vector<uint32>                     TopologicalOrder;
        std::vector<std::pair<uint32, uint32>>  ExplicitEdges;   // (after, before) pairs
        uint32                                  LastGeneration = UINT32_MAX;
    };
}
