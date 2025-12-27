#pragma once

#include "PhoenixRTS/Effects/Responses.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API ResponseDamageHandler final : public ResponseHandlerBase
    {
        PHX_DECLARE_TYPE_WITH_BASE(ResponseDamageHandler, ResponseHandlerBase)
    public:
        ResponseDamageHandler();
        bool Execute(WorldRef world, const ResponseContext& context) const override;
    };
}
