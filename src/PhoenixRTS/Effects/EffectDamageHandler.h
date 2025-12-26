#pragma once

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/Effects.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API EffectDamageHandler : public IEffectHandler
    {
        PHX_DECLARE_TYPE_BEGIN(IEffectHandler)
            PHX_REGISTER_BASE(IEffectHandler)
        PHX_DECLARE_TYPE_END()

    public:

        FName GetEffectId() const override;

        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;
        bool Finalize(WorldRef world, const EffectFinalizeContext& context) const override;
    };
}
