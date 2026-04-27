#pragma once

#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"
#include "PhoenixRTS/Effects/EffectId.h"

namespace Phoenix::RTS
{
    struct ProjectileId;

    enum class PHOENIX_RTS_API EProjectileState : uint8
    {
        Idle,
        MoveToEntity,
        MoveToLocation
    };

    struct PHOENIX_RTS_API ProjectileState
    {
        ProjectileState();

        union
        {
            MoveToEntityState MoveToEntity;
            MoveToLocationState MoveToLocation;
        } States;

        EProjectileState ActiveState;

        ECS::EntityId TargetEntity;
        Vec2 TargetPos;
        EffectNodeId EffectParent;
        FName ImpactEffectId;
        Distance ArrivalThreshold;

        AbilityStateResult Enter(WorldRef world, const ProjectileId& projectile);
        AbilityStateResult Update(WorldRef world, const ProjectileId& projectile);
        void Interrupt(WorldRef world, const ProjectileId& projectile);
        void Exit(WorldRef world, const ProjectileId& projectile);

        AbilityStateResult HandleActiveStateResult(
            WorldRef world,
            const ProjectileId& projectile,
            const AbilityStateResult& result) const;
    };
}
