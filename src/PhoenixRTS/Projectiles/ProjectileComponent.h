#pragma once

#include "PhoenixSim/ECS/Component.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"
#include "PhoenixRTS/Effects/EffectId.h"
#include "PhoenixSim/ECS/EntityId.h"

namespace Phoenix::RTS
{
    struct ProjectileId;
    struct ProjectileComponent;

    enum class PHOENIX_RTS_API EProjectileState : uint8
    {
        None,
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
        Distance ArrivalThreshold;

        AbilityStateResult Enter(
            WorldRef world,
            const ProjectileId& projectile,
            ProjectileComponent& comp,
            Distance arrivalThreshold);

        AbilityStateResult Update(
            WorldRef world,
            const ProjectileId& projectile,
            ProjectileComponent& comp);

        void Interrupt(
            WorldRef world,
            const ProjectileId& projectile,
            const ProjectileComponent& comp);

        void Exit(
            WorldRef world,
            const ProjectileId& projectile,
            const ProjectileComponent& comp);

        AbilityStateResult HandleActiveStateResult(
            WorldRef world,
            const ProjectileId& projectile, 
            ProjectileComponent& comp,
            const AbilityStateResult& result) const;
    };

    struct PHOENIX_RTS_API ProjectileComponent : ECS::IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(ProjectileComponent)

        ECS::EntityId Owner;
        FName ProjectileDataId;

        Vec2 LaunchPos;
        ECS::EntityId TargetEntity;
        Vec2 TargetPos;

        EffectNodeId EffectParent;
        FName ImpactEffectId;

        ProjectileState State;
    };
}
