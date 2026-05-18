#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Effects/Effects.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API EffectDamageHandler : public IEffectHandler
    {
        PHX_DECLARE_TYPE(EffectDamageHandler, IEffectHandler)

    public:

        FName GetEffectTypeId() const override;

        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;
        bool Finalize(WorldRef world, const EffectFinalizeContext& context) const override;
    };
}
