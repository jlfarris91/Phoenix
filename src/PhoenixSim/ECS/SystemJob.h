#pragma once

#include <utility>

#include "PhoenixSim/ECS/ArchetypeList.h"
#include "PhoenixSim/ECS/CommandBuffer.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/ECS/EntityQuery.h"
#include "PhoenixSim/ECS/JobBatch.h"

namespace Phoenix { class IFeature; }

namespace Phoenix::ECS
{
    // -------------------------------------------------------------------------
    // Access descriptors
    // -------------------------------------------------------------------------

    struct ComponentAccessEntry
    {
        FName ComponentId;
        EComponentAccess Access;
    };

    struct FeatureAccessEntry
    {
        FName FeatureId;
        EComponentAccess Access;
    };

    struct SystemAccessDescriptor
    {
        std::vector<ComponentAccessEntry> Components;
        std::vector<FeatureAccessEntry>   Features;

        bool ConflictsWith(const SystemAccessDescriptor& other) const
        {
            // Component conflicts
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
            // Feature conflicts
            for (const FeatureAccessEntry& a : Features)
            {
                for (const FeatureAccessEntry& b : other.Features)
                {
                    if (a.FeatureId == b.FeatureId)
                    {
                        if (a.Access != EComponentAccess::Read || b.Access != EComponentAccess::Read)
                            return true;
                    }
                }
            }
            return false;
        }
    };

    // -------------------------------------------------------------------------
    // Type traits
    // -------------------------------------------------------------------------

    // True when the underlying type (strip ref/cv) derives from IFeature.
    template <class T>
    using IsFeatureType = std::is_base_of<IFeature, Underlying_T<T>>;

    // Resolves the Execute() argument type for a single template parameter:
    //   - Component ref  (T&)       → T&
    //   - Const component ref  (const T&) → const T&
    //   - Feature ref  (TFeature&)       → TFeature::Ref&
    //   - Const feature ref  (const TFeature&) → const TFeature::Ref&
    template <class T, bool = IsFeatureType<T>::value>
    struct ExecuteArg { using type = T; };  // component: pass through (T already has ref qualifiers)

    template <class T>
    struct ExecuteArg<T&, true>  { using type = typename T::Ref&; };

    template <class T>
    struct ExecuteArg<const T&, true> { using type = const typename T::Ref&; };

    template <class T>
    using ExecuteArg_t = typename ExecuteArg<T>::type;

    // -------------------------------------------------------------------------
    // BuildAccessDescriptor — populates Components and Features separately
    // -------------------------------------------------------------------------

    template <class T>
    void AddAccessEntry(SystemAccessDescriptor& desc)
    {
        if constexpr (IsFeatureType<T>::value)
        {
            desc.Features.push_back(FeatureAccessEntry{
                Underlying_T<T>::StaticFeatureId,
                ComponentAccessFromT<T>::ComponentAccess
            });
        }
        else
        {
            desc.Components.push_back(ComponentAccessEntry{
                StaticTypeName<Underlying_T<T>>::TypeId,
                ComponentAccessFromT<T>::ComponentAccess
            });
        }
    }

    template <class... TComponents>
    void BuildAccessDescriptor(SystemAccessDescriptor& desc)
    {
        desc.Components.clear();
        desc.Features.clear();
        (AddAccessEntry<TComponents>(desc), ...);
    }

    // -------------------------------------------------------------------------
    // IJobBase / ITask
    // -------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------
    // IJob — per-archetype job
    //
    // TComponents is a mixed list of component and feature access descriptors:
    //   IJob<TransformComponent&, const UnitComponent&, const FeatureECS&>
    //
    // Component args (non-IFeature): resolved per entity from the archetype batch.
    // Feature args  (IFeature):      resolved once per batch from WorldConstRef;
    //                                passed as T::Ref& / const T::Ref& to Execute.
    // -------------------------------------------------------------------------

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
        virtual void BeginBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) {}

        // Called once for each valid entity in the batch.
        virtual void Execute(WorldConstRef world, EntityId id, CommandBuffer& cb,
                             ExecuteArg_t<TComponents>... args) = 0;

        // Called after the entity loop for each matching batch.
        virtual void EndBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) {}

    private:
        // ---- Compile-time helpers ----

        // Count non-feature args before index I in TComponents (= component-local index).
        template <std::size_t I, class T0, class... Ts>
        static constexpr std::size_t ComponentIndexOf()
        {
            if constexpr (I == 0)
                return 0;
            else
                return (IsFeatureType<T0>::value ? 0 : 1) + ComponentIndexOf<I - 1, Ts...>();
        }

        // Resolve a single arg for Execute at position I:
        //   - Feature: return ref from pre-resolved tuple (batch-stable).
        //   - Component: dereference via per-entity pointer arithmetic.
        template <class T, std::size_t I, class TRefTuple>
        static decltype(auto) GetArg(uint8* base, const JobBatch& batch, TRefTuple& refs)
        {
            if constexpr (IsFeatureType<T>::value)
            {
                // Feature ref — same for every entity in this batch
                auto& ref = std::get<I>(refs);
                if constexpr (std::is_const_v<std::remove_reference_t<T>>)
                    return static_cast<const std::remove_reference_t<decltype(ref)>&>(ref);
                else
                    return ref;
            }
            else
            {
                // Component ref — resolved per entity via batch offset
                constexpr std::size_t compIdx = ComponentIndexOf<I, TComponents...>();
                return *reinterpret_cast<Underlying_T<T>*>(base + batch.ComponentOffsets[compIdx]);
            }
        }

        // ---- Pre-resolved arg storage ----
        // Each position in TComponents holds either:
        //   - Underlying_T<T>::Ref  (for feature args)
        //   - std::monostate         (placeholder for component args; never accessed)
        //
        // Use specialization (not std::conditional_t) so the ::Ref member is only
        // instantiated when IsFeatureType<T> is true — otherwise MSVC tries to
        // evaluate it for component types and hits C1202.
        template <class T, bool = IsFeatureType<T>::value>
        struct ArgStorageHelper { using type = std::monostate; };

        template <class T>
        struct ArgStorageHelper<T, true> { using type = typename Underlying_T<T>::Ref; };

        template <class T>
        using ArgStorage = typename ArgStorageHelper<T>::type;

        template <class T>
        static auto MakeArgStorage(WorldConstRef world)
        {
            if constexpr (IsFeatureType<T>::value)
                return typename Underlying_T<T>::Ref(world);
            else
                return std::monostate{};
        }

        // ---- RunBatchImpl ----

        template <std::size_t... Is>
        void RunBatchImpl(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb,
                          std::index_sequence<Is...>)
        {
            // Resolve feature refs once per batch (outside the entity loop).
            // Component positions hold std::monostate — they are never accessed here.
            auto refs = std::make_tuple(MakeArgStorage<TComponents>(world)...);

            BeginBatch(world, batch, cb);

            if (batch.List)
            {
                uint8* data        = static_cast<uint8*>(batch.List->GetData());
                const uint32 stride = batch.List->GetEntityTotalSize();
                const uint32 count  = batch.List->GetNumInstances();

                for (uint32 i = 0; i < count; ++i)
                {
                    auto* inst = reinterpret_cast<ArchetypeInstance*>(data + i * stride);
                    if (inst->EntityId == EntityId::Invalid)
                        continue;
                    uint8* base = reinterpret_cast<uint8*>(inst + 1);
                    Execute(world, inst->EntityId, cb,
                        GetArg<TComponents, Is>(base, batch, refs)...);
                }
            }

            EndBatch(world, batch, cb);
        }

        SystemAccessDescriptor AccessDescriptor;
    };
}
