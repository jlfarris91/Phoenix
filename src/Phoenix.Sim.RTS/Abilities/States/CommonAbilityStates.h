#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim/FixedPoint/FixedTypes.h"
#include "Phoenix.Sim/ECS/FeatureECS.h"
#include "Phoenix.Sim/Containers/Optional.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
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

    struct PHOENIX_RTS_API TargetEntityState
    {
        ECS::EntityId Target;
        Vec2 LastKnownPosition = Vec2::Max;
    };

    struct PHOENIX_RTS_API TargetLocationState
    {
        Vec2 Target;
    };

    struct PHOENIX_RTS_API MoveToEntityState : TargetEntityState
    {
        Distance Range;

        AbilityStateResult Enter(WorldRef world, const ECS::EntityId& entity, const ECS::EntityId& target, Distance range);
        AbilityStateResult Update(WorldRef world, const ECS::EntityId& entity);
        void Interrupt(WorldRef world, const ECS::EntityId& entity);
        void Exit(WorldRef world, const ECS::EntityId& entity);
    };

    struct PHOENIX_RTS_API MoveToLocationState : TargetLocationState
    {
        Distance Range;

        AbilityStateResult Enter(WorldRef world, const ECS::EntityId& entity, const Vec2& target, Distance range);
        AbilityStateResult Update(WorldRef world, const ECS::EntityId& entity);
        void Interrupt(WorldRef world, const ECS::EntityId& entity);
        void Exit(WorldRef world, const ECS::EntityId& entity);
    };

    struct PHOENIX_RTS_API FaceEntityState : TargetEntityState
    {
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult Enter(
            WorldRef world,
            const ECS::EntityId& entity,
            const ECS::EntityId& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult Update(WorldRef world, const ECS::EntityId& entity);
        void Interrupt(WorldRef world, const ECS::EntityId& entity);
        void Exit(WorldRef world, const ECS::EntityId& entity);
    };

    struct PHOENIX_RTS_API FaceLocationState : TargetLocationState
    {
        Distance MaxRange;
        Angle Threshold;

        AbilityStateResult Enter(
            WorldRef world,
            const ECS::EntityId& entity,
            const Vec2& target,
            Distance range,
            Angle threshold = 5.0);

        AbilityStateResult Update(WorldRef world, const ECS::EntityId& entity);
        void Interrupt(WorldRef world, const ECS::EntityId& entity);
        void Exit(WorldRef world, const ECS::EntityId& entity);
    };

    struct PHOENIX_RTS_API FollowEntityState : TargetEntityState
    {
        Distance FollowRange;
        Time RangeCheckTime;

        enum class ESubState : uint8
        {
            Waiting,
            Moving
        } SubState;

        AbilityStateResult Enter(WorldRef world, const ECS::EntityId& entity, const ECS::EntityId& target, Distance range);
        AbilityStateResult Update(WorldRef world, const ECS::EntityId& entity);
        void Interrupt(WorldRef world, const ECS::EntityId& entity);
        void Exit(WorldRef world, const ECS::EntityId& entity);

        AbilityStateResult SetSubState(WorldRef world, const ECS::EntityId& unit, ESubState subState);
    };
}
