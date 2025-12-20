#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    struct UnitId;

    enum class PHOENIX_RTS_API EAbilityState : uint32
    {
        Approach = (uint32)"Approach"_n,
        Face = (uint32)"Face"_n
    };

    enum class PHOENIX_RTS_API EAbilityStateResult : uint8
    {
        Continue,
        Fail,
        Complete
    };

    struct PHOENIX_RTS_API AbilityStateResult
    {
        AbilityStateResult(EAbilityStateResult result, const FName& reason = FName::None)
            : Result(result)
            , Reason(reason)
        {
        }

        operator EAbilityStateResult() const { return Result; }

        EAbilityStateResult Result = EAbilityStateResult::Complete;
        FName Reason;
    };

    struct PHOENIX_RTS_API AbilityStateReasons
    {
        static constexpr FName NotInRange = "NotInRange"_n;
        static constexpr FName CannotMove = "CannotMove"_n;
        static constexpr FName CannotTurn = "CannotTurn"_n;
        static constexpr FName FailedToMove = "FailedToMove"_n;
        static constexpr FName FailedToTurn = "FailedToTurn"_n;
        static constexpr FName TargetLost = "TargetLost"_n;
        static constexpr FName TargetOutOfRange = "TargetOutOfRange"_n;
        static constexpr FName TargetTooClose = "TargetTooClose"_n;
        static constexpr FName TargetInvalid = "TargetInvalid"_n;
        static constexpr FName NoLongerSeekingGoal = "NoLongerSeekingGoal"_n;
        static constexpr FName WeaponOutOfAmmo = "WeaponOutOfAmmo"_n;
        static constexpr FName WeaponCooldown = "WeaponCooldown"_n;
        static constexpr FName FailedToAcquireEntityScope = "FailedToAcquireEntityScope"_n;
        static constexpr FName FailedToExecuteEffect = "FailedToExecuteEffect"_n;
    };

    struct PHOENIX_RTS_API MoveToEntityState
    {
        ECS::EntityId Target;
        Distance Range;

        AbilityStateResult OnEnter(WorldRef world, const UnitId& unit, const ECS::EntityId& target, Distance range);
        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API MoveToLocationState
    {
        Vec2 Target;
        Distance Range;

        AbilityStateResult OnEnter(WorldRef world, const UnitId& unit, const Vec2& target, Distance range);
        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API FaceEntityState
    {
        ECS::EntityId Target;
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_SIM_API FaceLocationState
    {
        Vec2 Target;
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult OnEnter(
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API FollowEntityState
    {
        ECS::EntityId Target;
        Distance FollowRange;
        Time RangeCheckTime;

        enum class ESubState
        {
            Waiting,
            Moving
        } SubState;

        AbilityStateResult OnEnter(WorldRef world, const UnitId& unit, const ECS::EntityId& target, Distance range);
        AbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);

        AbilityStateResult SetSubState(WorldRef world, const UnitId& unit, ESubState subState);
    };
}
