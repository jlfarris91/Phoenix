#pragma once

namespace Phoenix
{
    class World;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS::Data
{
    struct TagFilter;
    struct TargetFilter;
}

namespace Phoenix::RTS
{
    struct TargetFiltering
    {
        static bool PassesTagFilter(
            const World& world,
            const Data::TagFilter& filter,
            const ECS::EntityId& target);
        
        static bool PassesTargetFilter(
            const World& world,
            const Data::TargetFilter& filter,
            const ECS::EntityId& source,
            const ECS::EntityId& target);
    };
}