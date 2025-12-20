#pragma once

#include "PhoenixRTS/Effects/Responses.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API ResponseDamageHandler final : public ResponseHandlerBase
    {
    public:
        ResponseDamageHandler();
        bool Execute(WorldRef world, const ResponseContext& context) const override;
    };
}
