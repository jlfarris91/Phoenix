#include "Rect2D.h"

float Phoenix::Math::Rect2D::GetWidth() const
{
    return Max.x - Min.x;
}

float Phoenix::Math::Rect2D::GetHeight() const
{
    return Max.y - Min.y;
}
