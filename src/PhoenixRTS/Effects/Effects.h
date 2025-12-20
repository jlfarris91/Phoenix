#pragma once

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/EffectId.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::RTS
{
    class FeatureEffects;
    struct EffectComponent;

    struct PHOENIX_RTS_API ExecuteEffectArgs
    {
        FName EffectId;
        TOptional<FName> Name;

        TOptional<ECS::EntityId> SourceId;
        TOptional<Vec2> SourcePos;
        TOptional<uint8> SourcePlayer;

        TOptional<ECS::EntityId> TargetId;
        TOptional<Vec2> TargetPos;
        TOptional<uint8> TargetPlayer;
    };

    struct PHOENIX_RTS_API EffectContext
    {
        EffectNodeId ParentId;
        EffectComponent* Parent = nullptr;
        FName EffectId;
        ExecuteEffectArgs Overrides;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IEffectHandler : TSharedAsThis<IEffectHandler>
    {
    public:

        virtual ~IEffectHandler() = default;

        virtual FName GetEffectId() const;

        virtual void Initialize(SessionRef session);
        virtual void Shutdown(SessionRef session);

        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

        virtual bool Execute(WorldRef world, const EffectContext& context) const;
        virtual bool CanExecute(WorldConstRef world, const EffectContext& context) const;
    };

    class PHOENIX_RTS_API EffectHandlerBase : public IEffectHandler
    {
    public:

        EffectHandlerBase(const FName& effectId);

        FName GetEffectId() const override;

        void Initialize(SessionRef session) override;
        void Shutdown(SessionRef session) override;

        bool Execute(WorldRef world, const EffectContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectContext& context) const override;

        FName EffectId;
        TSharedPtr<FeatureEffects> EffectsFeature;
    };
}
