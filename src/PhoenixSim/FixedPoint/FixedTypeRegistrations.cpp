#include "PhoenixSim/FixedPoint/FixedTransform.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;

// ── Vec2 field registration ───────────────────────────────────────────────────

static const bool s_Vec2Registered = []()
{
    TypeDescriptorBuilder<Vec2>()
        .Field("X", &Vec2::X)
        .Field("Y", &Vec2::Y);
    return true;
}();

// ── Transform2D field registration ───────────────────────────────────────────

static const bool s_Transform2DRegistered = []()
{
    TypeDescriptorBuilder<Transform2D>()
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