#pragma once

#include <utility>

#include "PhoenixSim/ECS/ArchetypeList.h"
#include "PhoenixSim/ECS/CommandBuffer.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/ECS/EntityQuery.h"
#include "PhoenixSim/ECS/JobBatch.h"

namespace Phoenix::ECS
{
    struct ComponentAccessEntry
    {
        FName ComponentId;
        EComponentAccess Access;
    };

    struct SystemAccessDescriptor
    {
        std::vector<ComponentAccessEntry> Components;

        bool ConflictsWith(const SystemAccessDescriptor& other) const
        {
            for (const ComponentAccessEntry& a : Components)
            {
                for (const ComponentAccessEntry& b : other.Components)
                {
                    if (a.ComponentId == b.ComponentId)
                    {
                        if (a.Access != EComponentAccess::Read || b.Access != EComponentAccess::Read)
                            return true;
                    }
                }
            }
            return false;
        }
    };

    template <class... TComponents>
    void BuildAccessDescriptor(SystemAccessDescriptor& desc)
    {
        desc.Components = {
            ComponentAccessEntry{
                StaticTypeName<Underlying_T<TComponents>>::TypeId,
                ComponentAccessFromT<TComponents>::ComponentAccess
            }...
        };
    }

    class PHOENIX_SIM_API IJobBase
    {
    public:
        virtual ~IJobBase() = default;
        virtual void RunBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) = 0;
        virtual const SystemAccessDescriptor& GetAccessDescriptor() const = 0;
        virtual FName GetName() const { return FName::None; }
    };

    // Single-execution task: participates in the job dependency graph but runs
    // once per scheduler execution rather than once per matching archetype batch.
    // Use for setup/teardown work that must be ordered relative to IJob jobs.
    class PHOENIX_SIM_API ITask : public IJobBase
    {
    public:
        virtual void Run(WorldConstRef world, CommandBuffer& cb) = 0;

        void RunBatch(WorldConstRef world, const JobBatch& /*batch*/, CommandBuffer& cb) final
        {
            Run(world, cb);
        }

        const SystemAccessDescriptor& GetAccessDescriptor() const final
        {
            return EmptyDescriptor;
        }

    private:
        inline static const SystemAccessDescriptor EmptyDescriptor{};
    };

    template <class... TComponents>
    class IJob : public IJobBase
    {
    public:
        IJob()
        {
            BuildAccessDescriptor<TComponents...>(AccessDescriptor);
        }

        void RunBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) final
        {
            RunBatchImpl(world, batch, cb, std::index_sequence_for<TComponents...>{});
        }

        const SystemAccessDescriptor& GetAccessDescriptor() const final
        {
            return AccessDescriptor;
        }

        // Called before the entity loop for each matching batch.
        // Override to snapshot per-batch data (e.g. batch.UserData start offset).
        virtual void BeginBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) {}

        // Called once for each valid entity in the batch.
        virtual void Execute(WorldConstRef world, EntityId id, CommandBuffer& cb, TComponents... components) = 0;

        // Called after the entity loop for each matching batch.
        virtual void EndBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) {}

    private:
        template <std::size_t... Is>
        void RunBatchImpl(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb, std::index_sequence<Is...>)
        {
            uint8* data = static_cast<uint8*>(batch.List->GetData());
            const uint32 stride = batch.List->GetEntityTotalSize();
            const uint32 count = batch.List->GetNumInstances();

            BeginBatch(world, batch, cb);

            for (uint32 i = 0; i < count; ++i)
            {
                auto* inst = reinterpret_cast<ArchetypeInstance*>(data + i * stride);
                if (inst->EntityId == EntityId::Invalid)
                    continue;
                uint8* base = reinterpret_cast<uint8*>(inst + 1);
                Execute(world, inst->EntityId, cb,
                    *reinterpret_cast<Underlying_T<TComponents>*>(base + batch.ComponentOffsets[Is])...);
            }

            EndBatch(world, batch, cb);
        }

        SystemAccessDescriptor AccessDescriptor;
    };
}
