
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/TargetFiltering/TargetScanLevel.h"
#include "PhoenixRTS/Teams/Teams.h"
#include "PhoenixSim/Containers/Array.h"

namespace Phoenix::ECS
{
    class ISystem;
}

namespace Phoenix::RTS
{
    struct Damage;
    class IAbilityHandler;

    namespace Data
    {
        struct UnitPtr;
    }

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

    enum class EUnitQueryFlags
    {
        None,
        Alive = 1,
        Dead = 2,
        Cargo = 4,
        Hidden = 8,
        AliveOrDead = Alive | Dead
    };

    struct UnitRangeQueryArgs
    {
        EUnitQueryFlags Flags = EUnitQueryFlags::AliveOrDead;
        TeamMask TeamMask = Teams::All;
        TArray<UnitId> Exclude;
        uint32 MaxNum = 64;
    };

    class PHOENIX_RTS_API FeatureUnit : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureUnit)

    public:

        FeatureUnit();

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

        static Data::UnitPtr GetUnitData(WorldConstRef world, UnitId unit);

        static uint8 GetOwningPlayer(WorldConstRef world, UnitId unit);

        static uint8 GetOwningTeam(WorldConstRef world, UnitId unit);

        static bool UnitCanMove(WorldConstRef world, UnitId unit);

        static bool UnitCanTurn(WorldConstRef world, UnitId unit);

        static bool UnitIsImmobilized(WorldConstRef world, UnitId unit);

        static bool UnitIsAlive(WorldConstRef world, UnitId unit);

        static bool UnitIsDead(WorldConstRef world, UnitId unit);

        static bool UnitIsHidden(WorldConstRef world, UnitId unit);

        static bool UnitIsDetected(WorldConstRef world, UnitId unit, UnitId target);

        static bool UnitIsCargo(WorldConstRef world, UnitId unit);

        static bool UnitIsDormant(WorldConstRef world, UnitId unit);

        static bool UnitCanReceiveCommands(WorldConstRef world, UnitId unit);

        static ETargetScanLevel GetTargetScanLevel(WorldConstRef world, UnitId unit);
        
        static bool SetTargetScanLevel(WorldRef world, UnitId unit, ETargetScanLevel scanLevel);

        static bool ResetTargetScanLevel(WorldRef world, UnitId unit);

        static int32 GetAttackTargetPriority(WorldConstRef world, UnitId unit);

        // Returns the time that the unit will expire.
        static Time GetExpirationTime(WorldRef world, UnitId unit);

        // Sets a timer for a unit to expire.
        static bool SetExpirationTimer(WorldRef world, UnitId unit, Time expirationTime);

        // Clears an active time for a unit to expire.
        static bool ClearExpirationTimer(WorldRef world, UnitId unit);

        // Returns true if the unit had an expiration timer set and the timer has expired.
        static bool HasExpired(WorldRef world, UnitId unit);

        static uint32 QueryUnitsInRange(
            WorldConstRef world,
            const Vec2& pos,
            Distance range,
            TArray2<UnitId>& outUnits,
            const UnitRangeQueryArgs& args);

    protected:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args) override;

        TSharedPtr<ECS::ISystem> UnitSystem;
    };
}
