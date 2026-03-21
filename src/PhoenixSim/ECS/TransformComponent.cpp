#include "PhoenixSim/ECS/TransformComponent.h"
#include "PhoenixSim/Reflection/TypeRegistrationBuilder.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

PHX_TYPE_REGISTRATION(TransformComponent)
{
    registration
        .Field("AttachParent", &TransformComponent::AttachParent)
        .Field("Transform",    &TransformComponent::Transform)
        .Field("ZCode",        &TransformComponent::ZCode);
}
