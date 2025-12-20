#pragma once

#include "PhoenixRTS/Effects/Effects.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API EffectSetHandler final : public EffectHandlerBase
    {
    public:
        EffectSetHandler();

        bool Execute(WorldRef world, const EffectContext& context) const override;
        bool CanExecute(WorldConstRef world, const EffectContext& context) const override;
    };
}
