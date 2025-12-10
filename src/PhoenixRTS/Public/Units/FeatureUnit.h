
#pragma once

#include "Component.h"
#include "Entity.h"
#include "Features.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::RTS
{
    struct Unit : ECS::EntityId { };

    struct UnitComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(UnitComponent)
        PHX_ECS_DECLARE_COMPONENT_END()
    };

    enum class ESpawnUnitFlags : uint8
    {
        None = 0,
        SkipBirth = 1,
        IgnoreCollision = 2
    };

    struct SpawnUnitArgs
    {
        ESpawnUnitFlags Flags = ESpawnUnitFlags::None;

        // The maximum range that the unit can spawn from the spawn position.
        // Collision can cause the unit to spawn in a different location than desired. If that range would be further
        // than this distance, the spawn will fail.
        Distance MaxRange = 16;
    };

    class FeatureUnit : public IFeature
    {
    public:

        PHX_FEATURE_BEGIN(FeatureUnit)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
        PHX_FEATURE_END()

        static Unit SpawnUnit(
            WorldRef world,
            const FName& unitData,
            uint32 owner,
            const Vec2& pos,
            Angle facing,
            const SpawnUnitArgs& args = {});

        static uint32 SpawnUnits(
            WorldRef world,
            uint32 num,
            const FName& unitData,
            uint32 owner,
            const Vec2& pos,
            Angle facing,
            const SpawnUnitArgs& args = {});

        // Returns the current health of a unit.
        static Value GetHealth(WorldConstRef world, Unit unit);

    protected:

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
    };
}
