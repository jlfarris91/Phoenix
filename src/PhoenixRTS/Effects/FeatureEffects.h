
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/Effects.h"
#include "PhoenixRTS/Effects/FixedResponseList.h"

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
    struct UnitId;
    struct Order;

    class IEffectHandler;
    struct EffectComponent;

    class IResponseHandler;
    struct ResponseContext;

    struct PHOENIX_RTS_API FeatureEffectsDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureEffectsDynamicBlock)

        FixedResponseList<PHX_RTS_MAX_RESPONSES> Responses;
    };

    struct PHOENIX_RTS_API FeatureEffectsScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_SCRATCH(FeatureEffectsScratchBlock)

        TFixedArray<EffectNodeId, PHX_RTS_MAX_DEFERRED_EFFECTS> DeferredEffects[2];
        uint8 DeferredEffectsWriteIndex;
    };

    class PHOENIX_RTS_API FeatureEffects : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureEffects)
            FEATURE_WORLD_BLOCK(FeatureEffectsDynamicBlock)
            FEATURE_WORLD_BLOCK(FeatureEffectsScratchBlock)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        PHX_FEATURE_END()

    public:

        //
        // Effect Handlers
        //

        // Registers an effect handler with the feature.
        void RegisterEffectHandler(const TSharedPtr<IEffectHandler>& handler);

        template <class T, class ...TArgs>
        TSharedPtr<T> RegisterEffectHandler(TArgs&&... args)
        {
            TSharedPtr<T> handler = MakeShared<T>(std::forward<TArgs>(args)...);
            RegisterEffectHandler(handler);
            return handler;
        }

        // Unregisters an effect handler with the feature.
        bool UnregisterEffectHandler(const FName& baseObjectId);

        // Walks the effect object's bases and returns the first matching effect handler.
        // This overload caches the resulting handler for faster subsequent lookups even if it is null.
        TSharedPtr<IEffectHandler> FindEffectHandlerCached(WorldConstRef world, const FName& effectId);

        // Walks the effect object's bases and returns the first matching effect handler.
        TSharedPtr<IEffectHandler> FindEffectHandler(WorldConstRef world, const FName& effectId) const;

        //
        // Response Handlers
        //

        // Registers a response handler with the feature.
        void RegisterResponseHandler(const TSharedPtr<IResponseHandler>& handler);

        template <class T, class ...TArgs>
        TSharedPtr<T> RegisterResponseHandler(TArgs&&... args)
        {
            TSharedPtr<T> handler = MakeShared<T>(std::forward<TArgs>(args)...);
            RegisterResponseHandler(handler);
            return handler;
        }

        // Unregisters a response handler with the feature.
        bool UnregisterResponseHandler(const FName& baseObjectId);

        // Walks the response object's bases and returns the first matching response handler.
        // This overload caches the resulting handler for faster subsequent lookups even if it is null.
        TSharedPtr<IResponseHandler> FindResponseHandlerCached(WorldConstRef world, const FName& responseId);

        // Walks the response object's bases and returns the first matching response handler.
        TSharedPtr<IResponseHandler> FindResponseHandler(WorldConstRef world, const FName& responseId) const;

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

        static uint32 ReferenceEffectNode(EffectComponent& effectComp);

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

        //
        // Specializations
        //

        static Value GetEffectDamage(WorldConstRef world, EffectNodeId id);
        static bool SetEffectDamage(WorldRef world, EffectNodeId id, Value damage);

    protected:

        void Initialize() override;
        void Shutdown() override;

        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        void ProcessDeferredEffects(WorldRef world);
        void RespondToEffect(WorldRef world, EffectNodeId effectNodeId);
        bool FinalizeEffect(WorldRef world, EffectNodeId effectNodeId, EffectComponent& effectComponent);

        using PriorityResponseHandler = TTuple<int32, TSharedPtr<IResponseHandler>, FName>;

        void GetPrioritizedResponseHandlers(
            WorldConstRef world,
            ECS::EntityId entityId,
            const ResponseContext& responseContext,
            TArray<PriorityResponseHandler>& outResponseHandlers);

        TMap<FName, TSharedPtr<IEffectHandler>> EffectIdToHandlerMap;
        TMap<FName, TSharedPtr<IResponseHandler>> ResponseIdToHandlerMap;
    };
}
