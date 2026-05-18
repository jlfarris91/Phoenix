#pragma once

#include "Phoenix/Platform.h"

namespace Phoenix
{
    class World;

    typedef World* WorldPtr;
    typedef const World* WorldConstPtr;

    typedef World& WorldRef;
    typedef const World& WorldConstRef;

    typedef std::shared_ptr<World> WorldSharedPtr;

    using PostWorldUpdateDelegate = std::function<void(WorldConstRef world)>;
}
