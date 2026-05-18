
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"

namespace Phoenix
{
    // Represents a line between 2 points.
    template <class T = TVec2<>>
    struct TLine
    {
        constexpr TLine() = default;
        constexpr TLine(const T& start, const T& end) : Start(start), End(end) {}
        
        T Start;
        T End;

        // Linearly interpolates between Start and End by the parameter t.
        constexpr T Lerp(Value t)
        {
            return Start + (End - Start) * t;
        }

        // Returns the vector between Start and End.
        constexpr T GetVector() const
        {
            return End - Start;
        }

        // Returns the normalized vector between Start and End.
        constexpr T GetDirection() const
        {
            return GetVector().Normalized();
        }
    };

    using Line2 = TLine<Vec2>;
}

PHX_DEFINE_TYPE(Phoenix::Line2)
{
    registration
        .Alias("Line2")
        .Namespace("Phoenix.Line2")
        .Constructor<const Vec2&, const Vec2&>()
        .Field("Start", &Line2::Start)
        .Field("End", &Line2::End)
        .Method("Lerp", &Line2::Lerp)
        .Method("GetVector", &Line2::GetVector)
        .Method("GetDirection", &Line2::GetDirection);
}