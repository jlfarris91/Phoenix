
#pragma once

#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Reflection/Reflection.h"

namespace Phoenix
{
    template <class TVec, class TRot, class TScale>

    struct TTransform
    {
        TVec Position = TZero<TVec>::Value;
        TRot Rotation = TZero<TRot>::Value;
        TScale Scale = TOne<TScale>::Value;

        // Rotates a vector by Rotation
        TVec RotateVector(const TVec& vec) const;

        // Rotates a vector by -Rotation
        Vec2 RotateVectorInverse(const Vec2& vec);

        TVec TransformPoint(const TVec& pt) const;

        // Combine two transforms: Result = A * B (B applied first, then A)
        static TTransform Combine(const TTransform& a, const TTransform& b);

        // Get relative transform: solve for B such that C = Combine(A, B)
        static TTransform GetRelativeTransform(const TTransform& a, const TTransform& c);
    };

    typedef TTransform<Vec2, Angle, Value> Transform2D;

    template <>
    inline Vec2 TTransform<Vec2, Angle, Value>::RotateVector(const Vec2& vec) const
    {
        return vec.Rotate(Rotation);
    }

    template <class TVec, class TRot, class TScale>
    Vec2 TTransform<TVec, TRot, TScale>::RotateVectorInverse(const Vec2& vec)
    {
        return vec.Rotate(-Rotation);
    }

    template <class TVec, class TRot, class TScale>
    TVec TTransform<TVec, TRot, TScale>::TransformPoint(const TVec& pt) const
    {
        return RotateVector(pt) * Scale + Position;
    }

    template <class TVec, class TRot, class TScale>
    TTransform<TVec, TRot, TScale> TTransform<TVec, TRot, TScale>::Combine(
        const TTransform& a,
        const TTransform& b)
    {
        TTransform Result;
        Result.Scale = a.Scale * b.Scale;
        Result.Rotation = a.Rotation + b.Rotation;
        Result.Position = a.Position + a.RotateVector(b.Position * a.Scale);
        return Result;
    }

    template <class TVec, class TRot, class TScale>
    TTransform<TVec, TRot, TScale> TTransform<TVec, TRot, TScale>::GetRelativeTransform(
        const TTransform& a,
        const TTransform& c)
    {
        TTransform Result;
        Result.Scale = c.Scale / a.Scale;
        Result.Rotation = c.Rotation - a.Rotation;
        // (C.Position - A.Position) / A.Scale, then rotate by -A.Rotation
        TVec local = (c.Position - a.Position) / a.Scale;
        Result.Position = a.RotateVectorInverse(local);
        return Result;
    }
}

// ── External type registration ────────────────────────────────────────────────
//
// Register Transform2D with the Phoenix reflection system.
// Any translation unit that needs to reflect on these types must include
// this header (or another header that includes it).

PHX_REGISTER_EXTERNAL_TYPE(Transform2D)
