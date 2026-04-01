
#pragma once

#include "PhoenixSim/FixedPoint/FixedPoint.h"

namespace Phoenix
{
    using Value = TFixed<12>;
    using InvValue = TInvFixed2<Value>;
    using Distance = TFixed<12>;
    using Time = TFixed<6>;
    using DeltaTime = TInvFixed2<Time>;
    using Speed = Fixed32_16;
    using Angle = TFixed<20>;

    static constexpr Angle QuartPi = static_cast<Angle::QT>(823549);
    static constexpr Angle HalfPi = static_cast<Angle::QT>(QuartPi.Value * 2);
    static constexpr Angle Pi = static_cast<Angle::QT>(QuartPi.Value * 4);
    static constexpr Angle TwoPi = static_cast<Angle::QT>(Pi.Value * 2);
    static constexpr Angle FourPi = static_cast<Angle::QT>(Pi.Value * 4);
    static constexpr TInvFixed2<Angle> InvPi = static_cast<Angle::QT>(Pi.Value);
    static constexpr TInvFixed2<Angle> InvTwoPi = static_cast<Angle::QT>(TwoPi.Value);

    static constexpr Angle Deg45 = static_cast<Angle::QT>(47185920);
    static constexpr Angle Deg90 = static_cast<Angle::QT>(Deg45.Value * 2);
    static constexpr Angle Deg135 = static_cast<Angle::QT>(Deg45.Value * 3);
    static constexpr Angle Deg180 = static_cast<Angle::QT>(Deg45.Value * 4);
    static constexpr Angle Deg225 = static_cast<Angle::QT>(Deg45.Value * 5);
    static constexpr Angle Deg270 = static_cast<Angle::QT>(Deg45.Value * 6);
    static constexpr Angle Deg315 = static_cast<Angle::QT>(Deg45.Value * 7);
    static constexpr Angle Deg360 = static_cast<Angle::QT>(Deg45.Value * 8);
    static constexpr TInvFixed2<Angle> InvDeg180 = static_cast<Angle::QT>(Deg180.Value);

    static constexpr Angle Rad45 = QuartPi;
    static constexpr Angle Rad90 = HalfPi;
    static constexpr Angle Rad135 = Rad90 + Rad45;
    static constexpr Angle Rad180 = Pi;
    static constexpr Angle Rad225 = Rad180 + Rad45;
    static constexpr Angle Rad270 = Rad180 + Rad90;
    static constexpr Angle Rad315 = Rad180 + Rad135;
    static constexpr Angle Rad360 = TwoPi;
    static constexpr TInvFixed2<Angle> InvRad180 = static_cast<Angle::QT>(Rad180.Value);

    constexpr Angle Deg2Rad(Angle d)
    {
        auto v = int64(d.Value) * Pi.Value;
        auto v2 = v / Deg180.Value;
        return Q64(v2);
    }

    constexpr Angle Rad2Deg(Angle r)
    {
        auto v = int64(r.Value) * Deg180.Value;
        auto v2 = v / Pi.Value;
        return Q64(v2);
    }
}

// ── Reflection type traits for TFixed domain types ───────────────────────────
//
// Register canonical names so TypeRegistry and GenericConverter can identify
// these types.  Value == Distance (same TFixed<12,int32> type) — Distance is
// the canonical name.  InvValue / DeltaTime are inverse types, not registered.

#include "PhoenixSim/Reflection/TypeName.h"
#include "PhoenixSim/Reflection/TypeDescriptorMetadataProvider.h"

namespace Phoenix
{
    // A specialization for TFixed to provide the number of fractional bits as metadata, which is useful for UI to
    // display the fixed-point value correctly.
    template <uint8 Tb, class T>
    struct TypeDescriptorMetadataProvider<TFixed<Tb, T>>
    {
        static std::unordered_map<std::string, std::string> GetMetadata()
        {
            return
            { 
                { "FractionalBits", std::to_string(Tb) },
                { "UnderlyingType", std::to_string(StaticTypeName<T>::TypeId) }
            };
        }
    };
}
