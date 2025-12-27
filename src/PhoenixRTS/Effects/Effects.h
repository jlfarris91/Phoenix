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

    struct PHOENIX_RTS_API EffectExecuteContext
    {
        EffectNodeId ParentId;
        EffectComponent* ParentComponent = nullptr;
        FName EffectId;
        ExecuteEffectArgs Overrides;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    struct PHOENIX_RTS_API EffectFinalizeContext
    {
        EffectNodeId EffectNodeId;
        EffectComponent* EffectComponent = nullptr;
        FName EffectId;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IEffectHandler : public IService
    {
        PHX_DECLARE_INTERFACE_WITH_BASE(IEffectHandler, IService)

    public:

        virtual FName GetEffectId() const;

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual bool Execute(WorldRef world, const EffectExecuteContext& context) const;
        virtual bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const;

        virtual bool Finalize(WorldRef world, const EffectFinalizeContext& context) const;

    protected:

        TSharedPtr<FeatureEffects> EffectsFeature;
    };
}
