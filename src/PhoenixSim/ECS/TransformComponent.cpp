#include "PhoenixSim/ECS/TransformComponent.h"
#include "PhoenixSim/Reflection/Registration.h"

// Pull in Vec2/Transform2D/TFixed field registrations from FixedTypeRegistrations.cpp.
// That TU has no externally-referenced symbols so the linker would otherwise dead-strip it.
extern void Phoenix_EnsureFixedTypeRegistrations();
static const bool s_FixedTypesLoaded = (Phoenix_EnsureFixedTypeRegistrations(), true);

using namespace Phoenix;
using namespace Phoenix::ECS;

PHX_DEFINE_TYPE(TransformComponent)
{
    registration
        .Field("AttachParent", &TransformComponent::AttachParent)
        .Field("Transform",    &TransformComponent::Transform)
        .Field("ZCode",        &TransformComponent::ZCode);
}
