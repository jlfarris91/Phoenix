
#include "Phoenix/Color.h"

using namespace Phoenix;

const Color Color::White = Color(255, 255, 255);
const Color Color::Red = Color(255, 0, 0);
const Color Color::Green = Color(0, 255, 0);
const Color Color::Blue = Color(0, 0, 255);
const Color Color::Yellow = Color(255, 255, 0);

Color Color::operator*(Value v) const
{
    Color color;
    color.R = uint8(Value(R) * v);
    color.G = uint8(Value(G) * v);
    color.B = uint8(Value(B) * v);
    color.A = uint8(Value(A) * v);
    return color;
}

Color Color::operator*(const Color& c) const
{
    Color color;
    color.R = uint8(Value(R) * Value(c.R) / 255.0);
    color.G = uint8(Value(G) * Value(c.G) / 255.0);
    color.B = uint8(Value(B) * Value(c.B) / 255.0);
    color.A = uint8(Value(A) * Value(c.A) / 255.0);
    return color;
}

Color& Color::operator*=(Value v)
{
    *this = this->operator*(v);
    return *this;
}

Color& Color::operator*=(const Color& c)
{
    *this = this->operator*(c);
    return *this;
}

Color Color::operator/(Value v) const
{
    Color color;
    color.R = uint8(Value(R) / v);
    color.G = uint8(Value(G) / v);
    color.B = uint8(Value(B) / v);
    color.A = uint8(Value(A) / v);
    return color;
}

Color& Color::operator/=(Value v)
{
    *this = this->operator/(v);
    return *this;
}

Color Color::FromHex(uint32 hex)
{
    Color color;
    if ((hex & 0xFF000000) != 0)
    {
        color.A = hex & 0xFF; hex >>= 8;
    }
    color.B = hex & 0xFF; hex >>= 8;
    color.G = hex & 0xFF; hex >>= 8;
    color.R = hex & 0xFF;
    return color;
}

Color Color::FromHex(const char* hex)
{
    if (hex[0] == '#') ++hex;
    uint32 code = std::stoul(hex, nullptr, 16);
    return FromHex(code);
}
