#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"

#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.RTS/Effects/Effects.h"
#include "Phoenix.Sim.RTS/Effects/EffectComponent.h"
#include "Phoenix.Sim.RTS/Effects/EffectSetHandler.h"
#include "Phoenix.Sim.RTS/Effects/Responses.h"
#include "Phoenix.Sim.RTS/Effects/PeriodicEffectSystem.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

void FeatureEffectsDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Responses.Construct(allocator, config.MaxEffectResponses);
}

BlockBufferLayout FeatureEffectsDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureEffectsDynamicBlock>()
        .Container<FixedResponseList>("Responses", config.MaxEffectResponses);
}

void FeatureEffects::RegisterEffectHandler(const std::shared_ptr<IEffectHandler>& handler)
{
    EffectIdToHandlerMap.emplace(handler->GetEffectTypeId(), handler);
}

bool FeatureEffects::UnregisterEffectHandler(const FName& baseObjectId)
{ 
    return EffectIdToHandlerMap.erase(baseObjectId) > 0;
}

std::shared_ptr<IEffectHandler> FeatureEffects::FindEffectHandlerCached(WorldConstRef world, const FName& effectId)
{
    std::shared_ptr<IEffectHandler> effectHandler = FindEffectHandler(world, effectId);

    // Cache the handler for this effect id for faster subsequent lookups even if it is null.
    EffectIdToHandlerMap.emplace(effectId, effectHandler);

    return effectHandler;
}

std::shared_ptr<IEffectHandler> FeatureEffects::FindEffectHandler(WorldConstRef world, const FName& effectId) const
{
    std::shared_ptr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return {};
    }

    FName currentObjectId = effectId;
    for (;;)
    {
        auto effectHandlerIter = EffectIdToHandlerMap.find(currentObjectId);
        if (effectHandlerIter != EffectIdToHandlerMap.end())
        {
            return effectHandlerIter->second;
        }

        // Find the base object id
        const LDSRecord* baseRecord = queryContext->QueryRecord({ effectId, "/base"_n }, ELDSRecordQueryFlags::Exact);
        if (!baseRecord)
        {
            return {};
        }

        FName baseId = baseRecord->GetValueAs<FName>();
        if (baseId == currentObjectId)
        {
            return {};
        }

        currentObjectId = baseId;
    }
}

void FeatureEffects::RegisterResponseHandler(const std::shared_ptr<IResponseHandler>& handler)
{
    ResponseIdToHandlerMap.emplace(handler->GetResponseId(), handler);
}

bool FeatureEffects::UnregisterResponseHandler(const FName& baseObjectId)
{
    return ResponseIdToHandlerMap.erase(baseObjectId) > 0;
}

std::shared_ptr<IResponseHandler> FeatureEffects::FindResponseHandlerCached(WorldConstRef world, const FName& responseId)
{
    std::shared_ptr<IResponseHandler> effectHandler = FindResponseHandler(world, responseId);

    // Cache the handler for this response id for faster subsequent lookups even if it is null.
    ResponseIdToHandlerMap.emplace(responseId, effectHandler);

    return effectHandler;
}

std::shared_ptr<IResponseHandler> FeatureEffects::FindResponseHandler(WorldConstRef world, const FName& responseId) const
{
    std::shared_ptr<const ILDSQueryContext> queryContext = FeatureLDS::StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return {};
    }

    FName currentObjectId = responseId;
    for (;;)
    {
        auto effectHandlerIter = ResponseIdToHandlerMap.find(currentObjectId);
        if (effectHandlerIter != ResponseIdToHandlerMap.end())
        {
            return effectHandlerIter->second;
        }

        // Find the base object id
        const LDSRecord* baseRecord = queryContext->QueryRecord({ responseId, "/base"_n }, ELDSRecordQueryFlags::Exact);
        if (!baseRecord)
        {
            return {};
        }

        FName baseId = baseRecord->GetValueAs<FName>();
        if (baseId == currentObjectId)
        {
            return {};
        }

        currentObjectId = baseId;
    }
}

EffectScopeId FeatureEffects::AcquireEffectScope(WorldRef world, const ExecuteEffectArgs& args)
{
    PHX_ASSERT(args.SourceId.IsSet());
    PHX_ASSERT(args.TargetId.IsSet());
    PHX_ASSERT(!FName::IsNoneOrEmpty(args.EffectId));

    EffectScopeId effectScopeId = EffectScopeId(FeatureECS::StaticAcquireEntity(world, "EffectScope"_n));
    if (effectScopeId == EntityId::Invalid)
    {
        return effectScopeId;
    }

    EffectComponent* effectComp = FeatureECS::GetOrAddComponent<EffectComponent>(world, effectScopeId);
    if (!effectComp)
    {
        FeatureECS::StaticReleaseEntity(world, effectScopeId);
        return EffectScopeId{};
    }

    effectComp->Name = args.Name.GetValue(args.EffectId);
    effectComp->SourceId = args.SourceId.Get();
    effectComp->SourcePos = args.SourcePos.GetValue([&]{ return FeatureECS::GetWorldPosition(world, args.SourceId.Get()); });
    effectComp->TargetId = args.TargetId.Get();
    effectComp->TargetPos = args.TargetPos.GetValue([&]{ return FeatureECS::GetWorldPosition(world, args.TargetId.Get()); });
    effectComp->RefCount = 1;
    effectComp->ChannelingCount = 0;

    if (!FeatureECS::SetBlackboardValue<FName>(world, effectScopeId, "effect_node_object_id"_n, args.EffectId))
    {
        FeatureECS::StaticReleaseEntity(world, effectScopeId);
        return EffectScopeId{};
    }

    return { effectScopeId };
}

uint16 FeatureEffects::ReleaseEffectScope(WorldRef world, EffectScopeId id)
{
    EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    if (!effectScope)
    {
        return 0;
    }

    if (--effectScope->RefCount == 0)
    {
        FeatureECS::ForEachEntityInGroup(world, id, [](const EntityId& entity)
        {
            
        });

        FeatureECS::StaticReleaseEntity(world, id);
    }

    return effectScope->RefCount;
}

FName FeatureEffects::GetEffectScopeObjectId(WorldConstRef world, EffectScopeId id)
{
    return FeatureECS::GetBlackboardValue(world, id, "effect_node_object_id"_n, FName::None);
}

FName FeatureEffects::GetEffectScopeName(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->Name : FName::None;
}

EntityId FeatureEffects::GetEffectScopeSourceId(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->SourceId : EntityId::Invalid;
}

EntityId FeatureEffects::GetEffectScopeTargetId(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->TargetId : EntityId::Invalid;
}

Vec2 FeatureEffects::GetEffectScopeSourcePos(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->SourcePos : Vec2::Zero;
}

Vec2 FeatureEffects::GetEffectScopeTargetPos(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->TargetPos : Vec2::Zero;
}

uint16 FeatureEffects::GetEffectScopeChannelingCount(WorldConstRef world, EffectScopeId id)
{
    const EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id);
    return effectScope ? effectScope->ChannelingCount : 0;
}

bool FeatureEffects::SetEffectScopeChannelingCount(WorldRef world, EffectScopeId id, uint16 count)
{
    if (EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id))
    {
        effectScope->ChannelingCount = count;
        return true;
    }
    return false;
}

bool FeatureEffects::IsEffectScopeChanneling(WorldConstRef world, EffectScopeId id)
{
    return GetEffectScopeChannelingCount(world, id) > 0;
}

bool FeatureEffects::BeginEffectScopeChanneling(WorldRef world, EffectScopeId id)
{
    if (EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id))
    {
        ++effectScope->ChannelingCount;
        return true;
    }
    return false;
}

bool FeatureEffects::EndEffectScopeChanneling(WorldRef world, EffectScopeId id)
{
    if (EffectComponent* effectScope = FeatureECS::GetComponent<EffectComponent>(world, id))
    {
        if (effectScope->ChannelingCount > 0)
        {
            --effectScope->ChannelingCount;
            return true;
        }
    }
    return false;
}

EffectNodeId FeatureEffects::AcquireEffectNode(WorldRef world, EffectNodeId parentId, const ExecuteEffectArgs& args)
{
    EffectComponent* parentComp = FeatureECS::GetComponent<EffectComponent>(world, parentId);
    if (!parentComp)
    {
        return EffectNodeId{};
    }

    return AcquireEffectNode(world, parentId, *parentComp, args);
}

EffectNodeId FeatureEffects::AcquireEffectNode(
    WorldRef world,
    EffectNodeId parentId,
    EffectComponent& parent,
    const ExecuteEffectArgs& args)
{
    EffectNodeId effectNodeId = EffectNodeId(FeatureECS::StaticAcquireEntity(world, "EffectNode"_n));
    if (effectNodeId == EntityId::Invalid)
    {
        return EffectNodeId{};
    }

    EffectComponent* nodeComp = FeatureECS::GetOrAddComponent<EffectComponent>(world, effectNodeId);
    if (!nodeComp)
    {
        FeatureECS::StaticReleaseEntity(world, effectNodeId);
        return EffectNodeId{};
    }

    nodeComp->Name = args.Name.GetValue(args.EffectId);
    nodeComp->SourceId = args.SourceId.GetValue(parent.SourceId);
    nodeComp->SourcePos = args.SourcePos.GetValue(parent.SourcePos);
    nodeComp->TargetId = args.TargetId.GetValue(parent.TargetId);
    nodeComp->TargetPos = args.TargetPos.GetValue(parent.TargetPos);
    nodeComp->RefCount = 1;
    nodeComp->ChannelingCount = 0;

    if (!FeatureECS::SetBlackboardValue<FName>(world, effectNodeId, "effect_node_object_id"_n, args.EffectId))
    {
        FeatureECS::StaticReleaseEntity(world, effectNodeId);
        return EffectNodeId{};
    }

    if (!FeatureECS::SetBlackboardValue<EntityId>(world, effectNodeId, "effect_node_parent"_n, parentId))
    {
        FeatureECS::StaticReleaseEntity(world, effectNodeId);
        return EffectNodeId{};
    }

    if (!FeatureECS::AddEntityToGroup(world, parentId, effectNodeId))
    {
        FeatureECS::StaticReleaseEntity(world, effectNodeId);
        return EffectNodeId{};
    }

    ReferenceEffectNode(world, parentId, parent);

    return effectNodeId;
}

EffectComponent* FeatureEffects::GetEffectComponent(WorldRef world, EffectNodeId id)
{
    return FeatureECS::GetComponent<EffectComponent>(world, id);
}

const EffectComponent* FeatureEffects::GetEffectComponent(WorldConstRef world, EffectNodeId id)
{
    return FeatureECS::GetComponent<EffectComponent>(world, id);
}

uint32 FeatureEffects::ReferenceEffectNode(WorldRef world, EffectNodeId id)
{
    EffectComponent* nodeComp = FeatureECS::GetOrAddComponent<EffectComponent>(world, id);
    return nodeComp ? ReferenceEffectNode(world, id, *nodeComp) : 0;
}

uint32 FeatureEffects::ReferenceEffectNode(WorldRef world, EffectNodeId id, EffectComponent& effectComp)
{
    ++effectComp.RefCount;

    // Recursively reference parent nodes too
    EffectNodeId parentNodeId = GetEffectNodeParent(world, id);
    if (parentNodeId != EntityId::Invalid)
    {
        ReferenceEffectNode(world, parentNodeId);
    }

    return effectComp.RefCount;
}

uint32 FeatureEffects::DereferenceEffectNode(WorldRef world, EffectNodeId id)
{
    EffectComponent* nodeComp = FeatureECS::GetOrAddComponent<EffectComponent>(world, id);
    return nodeComp ? DereferenceEffectNode(world, id, *nodeComp) : 0;
}

uint32 FeatureEffects::DereferenceEffectNode(WorldRef world, EffectNodeId id, EffectComponent& effectComp)
{
    if (effectComp.RefCount == 0)
    {
        return 0;
    }

    --effectComp.RefCount;

    // Recursively dereference parent nodes too
    EffectNodeId parentNodeId = GetEffectNodeParent(world, id);
    if (parentNodeId != EntityId::Invalid)
    {
        DereferenceEffectNode(world, parentNodeId);
    }

    if (effectComp.RefCount == 0)
    {
        FeatureECS::StaticReleaseEntity(world, id);
    }
    
    return effectComp.RefCount;
}

EffectScopeId FeatureEffects::GetEffectScope(WorldConstRef world, EffectNodeId id)
{
    for (;;)
    {
        const Entity* entity = FeatureECS::GetEntityPtr(world, id);
        if (!entity)
        {
            return EffectScopeId{};
        }

        if (entity->Kind == "EffectScope"_n)
        {
            return EffectScopeId(id);
        }

        if (entity->Kind != "EffectNode"_n)
        {
            return EffectScopeId{};
        }

        id = GetEffectNodeParent(world, id);
    }
}

EffectNodeId FeatureEffects::GetEffectNodeParent(WorldConstRef world, EffectNodeId id)
{
    return FeatureECS::GetBlackboardValue<EffectScopeId>(world, id, "effect_node_parent"_n);
}

EffectNodeId FeatureEffects::GetNamedParentOrScope(WorldConstRef world, EffectNodeId id, const FName& name)
{
    for (;;)
    {
        const Entity* entity = FeatureECS::GetEntityPtr(world, id);
        if (!entity)
        {
            return EffectNodeId{};
        }

        if (entity->Kind == "EffectScope"_n)
        {
            return id;
        }

        if (entity->Kind != "EffectNode"_n)
        {
            return EffectNodeId{};
        }

        const EffectComponent* comp = FeatureECS::GetComponent<EffectComponent>(world, id);
        if (comp && comp->Name == name)
        {
            return id;
        }

        id = GetEffectNodeParent(world, id);
    }
}

FName FeatureEffects::GetEffectNodeObjectId(WorldConstRef world, EffectNodeId id)
{
    return FeatureECS::GetBlackboardValue(world, id, "effect_node_object_id"_n, FName::None);
}

bool FeatureEffects::RegisterResponse(WorldRef world, EntityId entityId, const FName& responseId)
{
    FeatureEffectsDynamicBlock* block = world.GetBlock<FeatureEffectsDynamicBlock>();
    return block && block->Responses.AddResponse(entityId, responseId);
}

bool FeatureEffects::UnregisterResponse(WorldRef world, EntityId entityId, const FName& responseId)
{
    FeatureEffectsDynamicBlock* block = world.GetBlock<FeatureEffectsDynamicBlock>();
    return block && block->Responses.RemoveResponse(entityId, responseId);
}

bool FeatureEffects::ClearResponses(WorldRef world, EntityId entityId)
{
    FeatureEffectsDynamicBlock* block = world.GetBlock<FeatureEffectsDynamicBlock>();
    return block && block->Responses.ClearResponses(entityId) > 0;
}

bool FeatureEffects::StaticExecuteEffect(
    WorldRef world,
    EffectNodeId parentNode,
    const FName& effectId,
    const ExecuteEffectArgs& overrides)
{
    std::shared_ptr<FeatureEffects> feature = GetFeature<FeatureEffects>(world);
    return feature && feature->ExecuteEffect(world, parentNode, effectId, overrides);
}

bool FeatureEffects::ExecuteEffect(
    WorldRef world,
    EffectNodeId parentNode,
    const FName& effectId,
    const ExecuteEffectArgs& overrides)
{
    std::shared_ptr<IEffectHandler> effectHandler = FindEffectHandlerCached(world, effectId);
    if (!effectHandler)
    {
        return false;
    }

    EffectNodeId scopeId = GetEffectScope(world, parentNode);

    EffectComponent* comp = FeatureECS::GetComponent<EffectComponent>(world, scopeId);
    if (!comp)
    {
        return false;
    }

    EffectExecuteContext context;
    context.ParentId = { scopeId };
    context.ParentComponent = comp;
    context.EffectId = effectId;
    context.Overrides = overrides;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    if (!effectHandler->CanExecute(world, context))
    {
        return false;
    }

    return effectHandler->Execute(world, context);
}

bool FeatureEffects::DeferEffectExecution(WorldRef world, EffectNodeId id)
{
    EffectComponent* comp = GetEffectComponent(world, id);
    return comp && DeferEffectExecution(world, id, *comp);
}

bool FeatureEffects::DeferEffectExecution(WorldRef world, EffectNodeId id, EffectComponent& comp)
{
    FeatureEffectsScratchBlock* block = world.GetBlock<FeatureEffectsScratchBlock>();
    if (!block)
    {
        return false;
    }

    if (!block->DeferredEffects[block->DeferredEffectsWriteIndex].PushBack(id))
    {
        return false;
    }

    // Keep the effect alive until it is finalized
    ReferenceEffectNode(world, id, comp);
    return true;
}

Value FeatureEffects::GetEffectDamage(WorldConstRef world, EffectNodeId id)
{
    return FeatureECS::GetBlackboardValue<Value>(world, id, "damage"_n);
}

bool FeatureEffects::SetEffectDamage(WorldRef world, EffectNodeId id, Value damage)
{
    return FeatureECS::SetBlackboardValue<Value>(world, id, "damage"_n, damage);
}

void FeatureEffects::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    // Register default handlers
    RegisterEffectHandler<EffectSetHandler>();

    std::vector<std::shared_ptr<IEffectHandler>> effectHandlers;
    Session->ResolveServices<IEffectHandler>(effectHandlers);

    for (const auto& effectHandler : effectHandlers)
    {
        RegisterEffectHandler(effectHandler);
    }

    std::vector<std::shared_ptr<IResponseHandler>> responseHandlers;
    Session->ResolveServices<IResponseHandler>(responseHandlers);

    for (const auto& responseHandler : responseHandlers)
    {
        RegisterResponseHandler(responseHandler);
    }

    PeriodicEffectSystem = std::make_shared<RTS::PeriodicEffectSystem>();
    if (auto featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->RegisterSystem(PeriodicEffectSystem);
    }
}

void FeatureEffects::Shutdown()
{
    while (!ResponseIdToHandlerMap.empty())
    {
        UnregisterResponseHandler(ResponseIdToHandlerMap.begin()->first);
    }

    while (!EffectIdToHandlerMap.empty())
    {
        UnregisterEffectHandler(EffectIdToHandlerMap.begin()->first);
    }

    if (auto featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(PeriodicEffectSystem);
    }

    PeriodicEffectSystem.reset();

    IFeature::Shutdown();
}

void FeatureEffects::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureEffectsDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxEffectResponses = PHX_RTS_MAX_RESPONSES;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();
        dynamicBlockConfig.MaxEffectResponses = featureConfigData.value("max_effect_responses", dynamicBlockConfig.MaxEffectResponses);
    }

    builder.RegisterBlockWithAlloc<FeatureEffectsDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
}

void FeatureEffects::OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPostWorldUpdate(world, args);

    ProcessDeferredEffects(world);
}

void FeatureEffects::ProcessDeferredEffects(WorldRef world)
{
    FeatureEffectsScratchBlock& block = world.GetBlockRef<FeatureEffectsScratchBlock>();

    uint8 readIndex = block.DeferredEffectsWriteIndex;
    block.DeferredEffectsWriteIndex = Wrap<uint8>(block.DeferredEffectsWriteIndex + 1, 0, _countof(block.DeferredEffects));

    for (const EffectNodeId& effectNodeId : block.DeferredEffects[readIndex])
    {
        RespondToEffect(world, effectNodeId);
    }

    block.DeferredEffects[readIndex].Reset();
}

void FeatureEffects::RespondToEffect(WorldRef world, EffectNodeId effectNodeId)
{
    FeatureEffectsDynamicBlock* dynamicBlock = world.GetBlock<FeatureEffectsDynamicBlock>();
    if (!dynamicBlock)
    {
        return;
    }

    EffectComponent* effectComponent = GetEffectComponent(world, effectNodeId);
    if (!effectComponent)
    {
        return;
    }

    ResponseContext responseContext;
    responseContext.EffectNodeId = effectNodeId;
    responseContext.EffectComponent = effectComponent;
    responseContext.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    std::vector<PriorityResponseHandler> prioritizedHandlers;
    prioritizedHandlers.reserve(8);

    if (effectComponent->SourceId != EntityId::Invalid)
    {
        GetPrioritizedResponseHandlers(world, effectComponent->SourceId, responseContext, prioritizedHandlers);

        for (auto && [pri, handler, responseId] : prioritizedHandlers)
        {
            responseContext.ResponseId = responseId;
            handler->Execute(world, responseContext);
        }
    }

    prioritizedHandlers.clear();

    if (effectComponent->TargetId != EntityId::Invalid)
    {
        GetPrioritizedResponseHandlers(world, effectComponent->TargetId, responseContext, prioritizedHandlers);

        for (auto && [pri, handler, responseId] : prioritizedHandlers)
        {
            responseContext.ResponseId = responseId;
            handler->Execute(world, responseContext);
        }
    }

    FinalizeEffect(world, effectNodeId, *effectComponent);

    DereferenceEffectNode(world, effectNodeId, *effectComponent);
}

bool FeatureEffects::FinalizeEffect(WorldRef world, EffectNodeId effectNodeId, EffectComponent& effectComponent)
{
    FName effectObjectId = GetEffectNodeObjectId(world, effectNodeId);
    if (FName::IsNoneOrEmpty(effectObjectId))
    {
        return false;
    }

    std::shared_ptr<IEffectHandler> handler = FindEffectHandlerCached(world, effectObjectId);
    if (!handler)
    {
        return false;
    }

    EffectFinalizeContext context;
    context.EffectNodeId = effectNodeId;
    context.EffectComponent = &effectComponent;
    context.EffectId = effectObjectId;
    context.LdsQueryContext = FeatureLDS::StaticGetWorldQueryContext(world);

    return handler->Finalize(world, context);
}

void FeatureEffects::GetPrioritizedResponseHandlers(
    WorldConstRef world,
    EntityId entityId,
    const ResponseContext& responseContext,
    std::vector<PriorityResponseHandler>& outResponseHandlers)
{
    const FeatureEffectsDynamicBlock* dynamicBlock = world.GetBlock<FeatureEffectsDynamicBlock>();
    dynamicBlock->Responses.ForEachResponse(entityId, [&](const FName& responseId)
    {
        std::shared_ptr<IResponseHandler> handler = FindResponseHandlerCached(world, responseId);
        if (!handler)
        {
            return;
        }

        auto context = responseContext;
        context.ResponseId = responseId;

        int32 priority = handler->GetPriority(world, responseContext);
        outResponseHandlers.emplace_back(priority, handler, responseId);
    });

    if (!outResponseHandlers.empty())
    {
        std::ranges::sort(
            outResponseHandlers,
            {},
            [](const PriorityResponseHandler& handler) { return std::get<0>(handler); });
    }
}
