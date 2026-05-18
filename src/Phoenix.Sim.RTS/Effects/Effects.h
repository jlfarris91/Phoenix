#pragma once

#include "Phoenix.Sim/Containers/Optional.h"
#include "Phoenix.Sim/ECS/EntityId.h"
#include "Phoenix.Sim/FixedPoint/FixedVector.h"
#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/Services/Service.h"
#include "Phoenix.Sim/WorldsFwd.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Effects/EffectId.h"

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
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    struct PHOENIX_RTS_API EffectFinalizeContext
    {
        EffectNodeId EffectNodeId;
        EffectComponent* EffectComponent = nullptr;
        FName EffectId;
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IEffectHandler : public IService
    {
        PHX_DECLARE_TYPE(IEffectHandler, IService)

    public:

        virtual FName GetEffectTypeId() const;

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual bool Execute(WorldRef world, const EffectExecuteContext& context) const;
        virtual bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const;

        virtual bool Finalize(WorldRef world, const EffectFinalizeContext& context) const;

    protected:

        std::shared_ptr<FeatureEffects> EffectsFeature;
    };
}
