#pragma once

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/Effects.h"
#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/ECS/System.h"

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API EffectLaunchProjectileComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(EffectLaunchProjectileComponent)
        Vec2 LaunchPos;
        Time NextPeriodic;
        FName PeriodicEffectId;
    };

    class PHOENIX_RTS_API EffectLaunchProjectileSystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM(EffectLaunchProjectileSystem);

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };
    
    class PHOENIX_RTS_API EffectLaunchProjectileHandler : public IEffectHandler
    {
        PHX_DECLARE_TYPE_WITH_BASE(EffectLaunchProjectileHandler, IEffectHandler)

    public:

        FName GetEffectTypeId() const override;

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;

        TSharedPtr<EffectLaunchProjectileSystem> System;
    };
}
