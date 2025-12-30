
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API Color
    {
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
