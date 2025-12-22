#pragma once

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/Effects.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API EffectDamageHandler final : public EffectHandlerBase
    {
    public:
        EffectDamageHandler();
        bool Execute(WorldRef world, const EffectExecuteContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectExecuteContext& context) const override;
        bool Finalize(WorldRef world, const EffectFinalizeContext& context) const override;
    };
}
