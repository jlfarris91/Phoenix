#include "PhoenixSim/FixedPoint/FixedTransform.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;

// Externally-visible no-op that prevents this TU from being dead-stripped by
// the linker.  TransformComponent.cpp calls it to pull in Vec2/Transform2D/
// TFixed metadata registrations.  Without this, static-only TUs are silently
// omitted from static library links that do not use /WHOLEARCHIVE.
void Phoenix_EnsureFixedTypeRegistrations() {}

// ── Vec2 field registration ───────────────────────────────────────────────────

static const bool s_Vec2Registered = []()
{
    using Vec2AddMember = Vec2 (Vec2::*)(const Vec2&) const;
    using Vec2AddComponentMember = Vec2 (Vec2::*)(const Vec2::ComponentT&) const;
    using Vec2NegMember = Vec2 (Vec2::*)() const;

    TypeDescriptorBuilder<Vec2>()
        .Namespace("Phoenix.Vec2")
        .Field("X", &Vec2::X)
        .Field("Y", &Vec2::Y)
        .StaticField("Zero", &Vec2::Zero)
        .StaticField("One", &Vec2::One)
        .StaticField("XAxis", &Vec2::XAxis)
        .StaticField("YAxis", &Vec2::YAxis)
        .StaticField("Min", &Vec2::Min)
        .StaticField("Max", &Vec2::Max)
        .Method("Add", static_cast<Vec2AddMember>(&Vec2::operator+))
        .Method("Add", static_cast<Vec2AddComponentMember>(&Vec2::operator+))
        .Method("Sub", static_cast<Vec2AddMember>(&Vec2::operator-))
        .Method("Sub", static_cast<Vec2AddComponentMember>(&Vec2::operator-))
        .Method("Mult", static_cast<Vec2AddMember>(&Vec2::operator*))
        .Method("Mult", static_cast<Vec2AddComponentMember>(&Vec2::operator*))
        .Method("Div", static_cast<Vec2AddMember>(&Vec2::operator/))
        .Method("Div", static_cast<Vec2AddComponentMember>(&Vec2::operator/))
        .Method("Neg", static_cast<Vec2NegMember>(&Vec2::operator-))
        .Method("AsRadians", &Vec2::AsRadians)
        .Method("AsDegrees", &Vec2::AsDegrees)
        .Method("Length", &Vec2::Length)
        .Method("Normalized", &Vec2::Normalized)
        .Method("Rotated", &Vec2::Rotate)
        .StaticMethod("Equals", &Vec2::Equals)
        // .StaticMethod("Dot", &Vec2::Dot)
        .StaticMethod("Distance", &Vec2::Distance)
        .StaticMethod("Project", &Vec2::Project)
        .StaticMethod("Reflect", &Vec2::Reflect)
        // .StaticMethod("Cross", &Vec2::Cross)
        // .StaticMethod("Intersects", &Vec2::Intersects)
        .StaticMethod("Midpoint", &Vec2::Midpoint)
        .StaticMethod("FromPolar", &Vec2::FromPolar);
        // .StaticMethod("Perpendicular", &Vec2::Perpendicular);
    return true;
}();

// ── Transform2D field registration ───────────────────────────────────────────

static const bool s_Transform2DRegistered = []()
{
    TypeDescriptorBuilder<Transform2D>()
        .Namespace("Phoenix.Transform2D")
        .Field("Position", &Transform2D::Position)
        .Field("Rotation", &Transform2D::Rotation)
        .Field("Scale",    &Transform2D::Scale);
    return true;
}();

// ── TFixed domain type registrations ─────────────────────────────────────────
//
// Each type stores its fractional bit count as type-level metadata so that any
// consumer holding a GenericValue (or a PropertyDescriptor) can recover the
// scaling factor without knowing the concrete TFixed<N> specialization.
//
// Value == Distance (both are TFixed<12,int32>) — Distance is the canonical name.

#define REGISTER_FIXED_TYPE(TypeName) \
    static const bool s_##TypeName##Registered = []() \
    { \
        TypeDescriptorBuilder<TypeName>() \
            .Metadata("FractionalBits", TypeName::B); \
        return true; \
    }();

REGISTER_FIXED_TYPE(Distance)
REGISTER_FIXED_TYPE(Time)
REGISTER_FIXED_TYPE(Speed)
REGISTER_FIXED_TYPE(Angle)

#undef REGISTER_FIXED_TYPE