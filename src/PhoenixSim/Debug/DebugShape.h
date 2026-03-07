#pragma once

#include "PhoenixSim/Color.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/FixedPoint/FixedCircle.h"
#include "PhoenixSim/FixedPoint/FixedLine.h"
#include "PhoenixSim/FixedPoint/FixedBox.h"
#include "PhoenixSim/FixedPoint/FixedEllipse.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API DebugShape
    {
        enum class Type : uint8
        {
            Invalid,
            Line,
            Circle,
            Ellipse,
            Box,
            Text
        };

        struct TextShape
        {
            Vec2 Pos;
            FName Name;
        };

        union Shape
        {
            Line2 Line;
            Circle2 Circle;
            Ellipse2 Ellipse;
            Box2 Box;
            TextShape Text;
        };

        DebugShape();
        DebugShape(const Line2& line, const Color& color);
        DebugShape(const Circle2& circle, const Color& color);
        DebugShape(const Ellipse2& ellipse, const Color& color);
        DebugShape(const Box2& box, const Color& color);
        DebugShape(const TextShape& text, const Color& color);

        Shape Data;
        Type ShapeType;
        Color Color;
    };
}
