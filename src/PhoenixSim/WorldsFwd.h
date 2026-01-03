#pragma once

#include "Platform.h"

namespace Phoenix
{
    class World;

    typedef World* WorldPtr;
    typedef const World* WorldConstPtr;

    typedef World& WorldRef;
    typedef const World& WorldConstRef;

    typedef TSharedPtr<World> WorldSharedPtr;

    using PostWorldUpdateDelegate = TFunction<void(WorldConstRef world)>;
}
