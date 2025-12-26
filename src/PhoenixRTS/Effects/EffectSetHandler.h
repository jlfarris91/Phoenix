#pragma once

#include "PhoenixRTS/Effects/Effects.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API EffectSetHandler final : public IEffectHandler
    {
        PHX_DECLARE_TYPE_BEGIN(EffectSetHandler)
            PHX_REGISTER_BASE(IEffectHandler)
        PHX_DECLARE_TYPE_END()
        
    public:

        FName GetEffectId() const override;

        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;
    };
}
