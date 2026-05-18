#pragma once

#include "Phoenix.Sim/Features.h"
#include "Phoenix.Sim/Containers/FixedArray.h"
#include "Phoenix/FixedPoint/FixedVector.h"
#include "Phoenix.Sim/Worlds.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Effects/Effects.h"
#include "Phoenix.Sim.RTS/Effects/FixedResponseList.h"

#ifndef PHX_RTS_MAX_RESPONSES
#define PHX_RTS_MAX_RESPONSES 4096
#endif

#ifndef PHX_RTS_MAX_DEFERRED_EFFECTS
#define PHX_RTS_MAX_DEFERRED_EFFECTS 4096
#endif

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    class PeriodicEffectSystem;
    struct UnitId;
    struct Order;

    class IEffectHandler;
    struct EffectComponent;

    class IResponseHandler;
    struct ResponseContext;

    struct PHOENIX_RTS_API FeatureEffectsDynamicBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureEffectsDynamicBlock)
        {
            uint32 MaxEffectResponses = PHX_RTS_MAX_RESPONSES;
        };

        FixedResponseList Responses;
    };

    struct PHOENIX_RTS_API FeatureEffectsScratchBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK(FeatureEffectsScratchBlock)

        TInlineArray<EffectNodeId, PHX_RTS_MAX_DEFERRED_EFFECTS> DeferredEffects[2];
        uint8 DeferredEffectsWriteIndex;
    };

    class PHOENIX_RTS_API FeatureEffects : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureEffects)
        {
            FEATURE_WORLD_BLOCK(FeatureEffectsScratchBlock, EBufferBlockType::Scratch)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        }

    public:

        //
        // Effect Handlers
        //

        // Registers an effect handler with the feature.
        void RegisterEffectHandler(const std::shared_ptr<IEffectHandler>& handler);

        template <class T, class ...TArgs>
        std::shared_ptr<T> RegisterEffectHandler(TArgs&&... args)
        {
            auto handler = std::make_shared<T>(std::forward<TArgs>(args)...);
            RegisterEffectHandler(handler);
            return handler;
        }

        // Unregisters an effect handler with the feature.
        bool UnregisterEffectHandler(const FName& baseObjectId);

        // Walks the effect object's bases and returns the first matching effect handler.
        // This overload caches the resulting handler for faster subsequent lookups even if it is null.
        std::shared_ptr<IEffectHandler> FindEffectHandlerCached(WorldConstRef world, const FName& effectId);

        // Walks the effect object's bases and returns the first matching effect handler.
        std::shared_ptr<IEffectHandler> FindEffectHandler(WorldConstRef world, const FName& effectId) const;

        //
        // Response Handlers
        //

        // Registers a response handler with the feature.
        void RegisterResponseHandler(const std::shared_ptr<IResponseHandler>& handler);

        template <class T, class ...TArgs>
        std::shared_ptr<T> RegisterResponseHandler(TArgs&&... args)
        {
            auto handler = std::make_shared<T>(std::forward<TArgs>(args)...);
            RegisterResponseHandler(handler);
            return handler;
        }

        // Unregisters a response handler with the feature.
        bool UnregisterResponseHandler(const FName& baseObjectId);

        // Walks the response object's bases and returns the first matching response handler.
        // This overload caches the resulting handler for faster subsequent lookups even if it is null.
        std::shared_ptr<IResponseHandler> FindResponseHandlerCached(WorldConstRef world, const FName& responseId);

        // Walks the response object's bases and returns the first matching response handler.
        std::shared_ptr<IResponseHandler> FindResponseHandler(WorldConstRef world, const FName& responseId) const;

        //
        // Effect Scopes
        //

        static EffectScopeId AcquireEffectScope(WorldRef world, const ExecuteEffectArgs& args);

        static uint16 ReleaseEffectScope(WorldRef world, EffectScopeId id);

        static FName GetEffectScopeObjectId(WorldConstRef world, EffectScopeId id);

        static FName GetEffectScopeName(WorldConstRef world, EffectScopeId id);

        static ECS::EntityId GetEffectScopeSourceId(WorldConstRef world, EffectScopeId id);

        static ECS::EntityId GetEffectScopeTargetId(WorldConstRef world, EffectScopeId id);

        static Vec2 GetEffectScopeSourcePos(WorldConstRef world, EffectScopeId id);

        static Vec2 GetEffectScopeTargetPos(WorldConstRef world, EffectScopeId id);

        static uint16 GetEffectScopeChannelingCount(WorldConstRef world, EffectScopeId id);

        static bool SetEffectScopeChannelingCount(WorldRef world, EffectScopeId id, uint16 count);

        static bool IsEffectScopeChanneling(WorldConstRef world, EffectScopeId id);

        static bool BeginEffectScopeChanneling(WorldRef world, EffectScopeId id);

        static bool EndEffectScopeChanneling(WorldRef world, EffectScopeId id);

        //
        // Effect Nodes
        //

        static EffectNodeId AcquireEffectNode(
            WorldRef world,
            EffectNodeId parentId,
            const ExecuteEffectArgs& args = {});

        static EffectNodeId AcquireEffectNode(
            WorldRef world,
            EffectNodeId parentId,
            EffectComponent& parent,
            const ExecuteEffectArgs& args = {});

        static EffectComponent* GetEffectComponent(WorldRef world, EffectNodeId id);

        static const EffectComponent* GetEffectComponent(WorldConstRef world, EffectNodeId id);

        static uint32 ReferenceEffectNode(WorldRef world, EffectNodeId id);

        static uint32 ReferenceEffectNode(WorldRef world, EffectNodeId id, EffectComponent& effectComp);

        static uint32 DereferenceEffectNode(WorldRef world, EffectNodeId id);

        static uint32 DereferenceEffectNode(WorldRef world, EffectNodeId id, EffectComponent& effectComp);

        static EffectScopeId GetEffectScope(WorldConstRef world, EffectNodeId id);

        static EffectNodeId GetEffectNodeParent(WorldConstRef world, EffectNodeId id);

        static EffectNodeId GetNamedParentOrScope(WorldConstRef world, EffectNodeId id, const FName& name);

        static FName GetEffectNodeObjectId(WorldConstRef world, EffectNodeId id);

        //
        // Response Management
        //

        static bool RegisterResponse(WorldRef world, ECS::EntityId entityId, const FName& responseId);

        static bool UnregisterResponse(WorldRef world, ECS::EntityId entityId, const FName& responseId);

        static bool ClearResponses(WorldRef world, ECS::EntityId entityId);

        template <class TCallback>
        static void ForEachResponse(WorldConstRef world, ECS::EntityId entityId, const TCallback& callback)
        {
            if (const FeatureEffectsDynamicBlock* block = world.GetBlock<FeatureEffectsDynamicBlock>())
            {
                block->Responses.ForEachResponse(entityId, callback);
            }
        }

        //
        // Execution
        //

        static bool StaticExecuteEffect(
            WorldRef world,
            EffectNodeId parentNode,
            const FName& effectId,
            const ExecuteEffectArgs& overrides = {});

        bool ExecuteEffect(
            WorldRef world,
            EffectNodeId parentNode,
            const FName& effectId,
            const ExecuteEffectArgs& overrides = {});

        static bool DeferEffectExecution(WorldRef world, EffectNodeId id);
        static bool DeferEffectExecution(WorldRef world, EffectNodeId id, EffectComponent& comp);

        //
        // Specializations
        //

        static Value GetEffectDamage(WorldConstRef world, EffectNodeId id);
        static bool SetEffectDamage(WorldRef world, EffectNodeId id, Value damage);

    protected:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        void ProcessDeferredEffects(WorldRef world);
        void RespondToEffect(WorldRef world, EffectNodeId effectNodeId);
        bool FinalizeEffect(WorldRef world, EffectNodeId effectNodeId, EffectComponent& effectComponent);

        using PriorityResponseHandler = std::tuple<int32, std::shared_ptr<IResponseHandler>, FName>;

        void GetPrioritizedResponseHandlers(
            WorldConstRef world,
            ECS::EntityId entityId,
            const ResponseContext& responseContext,
            std::vector<PriorityResponseHandler>& outResponseHandlers);

        std::shared_ptr<PeriodicEffectSystem> PeriodicEffectSystem;
        std::unordered_map<FName, std::shared_ptr<IEffectHandler>> EffectIdToHandlerMap;
        std::unordered_map<FName, std::shared_ptr<IResponseHandler>> ResponseIdToHandlerMap;
    };
}
