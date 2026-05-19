#pragma once

#include <entt/entt.hpp>

#include <IEntitySyncHandler.h>

namespace Phoenix
{
    class EnTTEntitySyncHandler : public IEntitySyncHandler
    {
    public:
        explicit EnTTEntitySyncHandler(entt::registry& registry);

    protected:
        entt::registry& Registry;

        static EngineEntity Pack(entt::entity e)
        {
            return { static_cast<uint64_t>(entt::to_integral(e)) };
        }

        static entt::entity Unpack(EngineEntity e)
        {
            return static_cast<entt::entity>(e.Id);
        }
    };
}
