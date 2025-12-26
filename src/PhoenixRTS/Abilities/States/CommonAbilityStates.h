#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Units/UnitId.h"

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
        AbilityStateResult() = default;
        AbilityStateResult(EAbilityStateResult result, const FName& reason = FName::None)
            : Result(result)
            , Reason(reason)
        {
        }

        operator EAbilityStateResult() const { return Result; }

        EAbilityStateResult Result = EAbilityStateResult::Fail;
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
        static constexpr FName TargetOutOfAngle = "TargetOutOfAngle"_n;
        static constexpr FName TargetTooClose = "TargetTooClose"_n;
        static constexpr FName TargetInvalid = "TargetInvalid"_n;
        static constexpr FName NoLongerSeekingGoal = "NoLongerSeekingGoal"_n;
        static constexpr FName WeaponOutOfAmmo = "WeaponOutOfAmmo"_n;
        static constexpr FName WeaponCooldown = "WeaponCooldown"_n;
        static constexpr FName FailedToAcquireEntityScope = "FailedToAcquireEntityScope"_n;
        static constexpr FName FailedToExecuteEffect = "FailedToExecuteEffect"_n;
    };

    template <class TTarget>
    struct PHOENIX_RTS_API BaseState
    {
        TTarget Target;
    };

    using TargetEntityState = BaseState<ECS::EntityId>;
    using TargetLocationState = BaseState<Vec2>;

    struct PHOENIX_RTS_API MoveToEntityState : TargetEntityState
    {
        Distance Range;

        AbilityStateResult Enter(WorldRef world, const UnitId& unit, const ECS::EntityId& target, Distance range);
        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API MoveToLocationState : TargetLocationState
    {
        Distance Range;

        AbilityStateResult Enter(WorldRef world, const UnitId& unit, const Vec2& target, Distance range);
        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API FaceEntityState : TargetEntityState
    {
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const ECS::EntityId& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API FaceLocationState : TargetLocationState
    {
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult Enter(
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    struct PHOENIX_RTS_API FollowEntityState : TargetEntityState
    {
        Distance FollowRange;
        Time RangeCheckTime;

        enum class ESubState
        {
            Waiting,
            Moving
        } SubState;

        AbilityStateResult Enter(WorldRef world, const UnitId& unit, const ECS::EntityId& target, Distance range);
        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);

        AbilityStateResult SetSubState(WorldRef world, const UnitId& unit, ESubState subState);
    };
}
