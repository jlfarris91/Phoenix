
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"

namespace Phoenix
{
    template <class TVec, class TRot, class TScale>
    struct TTransform
    {
        TVec Position = TZero<TVec>::Value;
        TRot Rotation = TZero<TRot>::Value;
        TScale Scale = TOne<TScale>::Value;
        TVec RotateVector(const TVec& vec) const;
        TVec TransformPoint(const TVec& pt) const;
    };

    typedef TTransform<Vec2, Angle, Value> Transform2D;

    template <>
    inline Vec2 TTransform<Vec2, Angle, Value>::RotateVector(const Vec2& vec) const
    {
        return vec.Rotate(Rotation);
    }

    template <class TVec, class TRot, class TScale>
    TVec TTransform<TVec, TRot, TScale>::TransformPoint(const TVec& pt) const
    {
        return RotateVector(pt) * Scale + Position;
    }
}
