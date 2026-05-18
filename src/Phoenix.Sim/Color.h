
#pragma once

#include "Phoenix.Sim/Platform.h"
#include "Phoenix.Sim/FixedPoint/FixedTypes.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API Color
    {
        PHX_DECLARE_TYPE(Color)

        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;

        constexpr Color() = default;
        constexpr Color(uint8 r, uint8 g, uint8 b, uint8 a = 255) : R(r), G(g), B(b), A(a) {}
        constexpr Color(const Color& other) = default;

        uint8 R = 0;
        uint8 G = 0;
        uint8 B = 0;
        uint8 A = 255;

        Color operator*(Value v) const;
        Color operator*(const Color& c) const;

        Color& operator*=(Value v);
        Color& operator*=(const Color& c);

        Color operator/(Value v) const;
        Color& operator/=(Value v);

        static Color FromHex(uint32 hex);
        static Color FromHex(const char* hex);
    };
}

PHX_DEFINE_TYPE(Phoenix::Color)
{
    registration
        .Constructor<uint8, uint8, uint8, uint8>()
        .Constructor<const Color&>()
        .Field("R", &Color::R)
        .Field("G", &Color::G)
        .Field("B", &Color::B)
        .Field("A", &Color::A)
        .StaticField("White", &Color::White)
        .StaticField("Red", &Color::Red)
        .StaticField("Green", &Color::Green)
        .StaticField("Blue", &Color::Blue)
        .StaticField("Yellow", &Color::Yellow)
        .StaticMethod("FromHex", static_cast<Color(*)(uint32)>(&Color::FromHex));
}