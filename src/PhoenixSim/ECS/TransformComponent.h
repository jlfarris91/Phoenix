
#pragma once

#include "PhoenixSim/FixedPoint/FixedTransform.h"
#include "PhoenixSim/ECS/Component.h"

namespace Phoenix::ECS
{
    struct PHOENIX_SIM_API TransformComponent : IComponent
    {
        PHX_ECS_DECLARE_COMPONENT(TransformComponent)

    public:

        // The id of another entity that the owning entity is attached to.
        // Note that this cannot be the entity that owns the body component.
        EntityId AttachParent = EntityId::Invalid;

        // The relative transform of the entity.
        // Relative to the origin if not attached to another entity.
        Transform2D Transform;

        // Morton z-code used for spacial sorting of entities.
        uint64 ZCode = 0;
    };

    struct PHOENIX_SIM_API EntityTransform
    {
        EntityId EntityId;
        TransformComponent* TransformComponent;
        uint64 ZCode = 0;
    };
}