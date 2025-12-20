
#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Commands/Commands.h"

namespace Phoenix::RTS
{
    class IAbilityHandler;

    struct PHOENIX_RTS_API UnitComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(UnitComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

        uint8 OwningPlayer = 0;
        FName UnitData;
    };

    enum class PHOENIX_RTS_API ESpawnUnitFlags : uint8
    {
        None = 0,
        SkipBirth = 1,
        IgnoreCollision = 2
    };

    struct PHOENIX_RTS_API SpawnUnitArgs
    {
        ESpawnUnitFlags Flags = ESpawnUnitFlags::None;

        // The maximum range that the unit can spawn from the spawn position.
        // Collision can cause the unit to spawn in a different location than desired. If that range would be further
        // than this distance, the spawn will fail.
        Distance MaxRange = 16;
    };

    class PHOENIX_RTS_API FeatureUnit : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureUnit)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
        PHX_FEATURE_END()

    public:

        static UnitId SpawnUnit(
            WorldRef world,
            const FName& unitData,
            uint8 owner,
            const Vec2& pos,
            Angle facing,
            const SpawnUnitArgs& args = {});

        static uint32 SpawnUnits(
            WorldRef world,
            uint32 num,
            const FName& unitData,
            uint8 owner,
            const Vec2& pos,
            Angle facing,
            const SpawnUnitArgs& args = {});

        static FName GetUnitDataId(WorldConstRef world, UnitId unit);

        static uint8 GetOwningPlayer(WorldConstRef world, UnitId unit);

        // Returns the current health of a unit.
        static Value GetHealth(WorldConstRef world, UnitId unit);

        // Returns the current max health of a unit.
        static Value GetHealthMax(WorldConstRef world, UnitId unit);

        // Returns the current health regen of a unit.
        static Value GetHealthRegen(WorldConstRef world, UnitId unit);

        static bool UnitCanMove(WorldConstRef world, UnitId unit);

        static bool UnitCanTurn(WorldConstRef world, UnitId unit);

        static bool UnitIsImmobilized(WorldConstRef world, UnitId unit);

        static bool UnitIsAlive(WorldConstRef world, UnitId unit);

        static bool UnitIsDead(WorldConstRef world, UnitId unit);

    protected:

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args) override;
    };
}
