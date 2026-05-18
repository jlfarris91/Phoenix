#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Effects/Effects.h"

namespace Phoenix::RTS
{    
    class PHOENIX_RTS_API EffectLaunchProjectileHandler : public IEffectHandler
    {
        PHX_DECLARE_TYPE(EffectLaunchProjectileHandler, IEffectHandler)

    public:

        FName GetEffectTypeId() const override;

        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;
    };
}
