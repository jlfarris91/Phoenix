
#pragma once

#include "PhoenixSim/Color.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/LDS/LDSRecordQueryFlags.h"
#include "PhoenixSim/LDS/ObjectModel/LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSValuePtrBase : LDSRecordPtr
    {
        LDSValuePtrBase() = default;
        LDSValuePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValuePtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_SIM_API LDSValuePtr : LDSValuePtrBase
    {
        LDSValuePtr() = default;
        LDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValuePtr(const LDSValuePtrBase& other);

        template <IsNotRecordPtr TValue>
        TValue GetValue(const ILDSQueryContext& context, const TValue& defaultValue = {}) const;

        template <IsNotRecordPtr TValue>
        bool TryGetValue(const ILDSQueryContext& context, TValue& outValue) const;
    };

    template <IsNotRecordPtr TValue>
    struct TLDSValuePtr : LDSValuePtrBase
    {
        using ValueT = TValue;

        TLDSValuePtr() = default;
        TLDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValuePtr(const LDSValuePtrBase& other);

        operator LDSValuePtr() const;

        TValue GetValue(const ILDSQueryContext& context, const TValue& defaultValue = {}) const;

        bool TryGetValue(const ILDSQueryContext& context, TValue& outValue) const;
    };

    using BoolPtr = TLDSValuePtr<bool>;
    using Int32Ptr = TLDSValuePtr<int32>;
    using UInt32Ptr = TLDSValuePtr<uint32>;
    using ValuePtr = TLDSValuePtr<Value>;
    using DistancePtr = TLDSValuePtr<Distance>;
    using AnglePtr = TLDSValuePtr<Angle>;
    using SpeedPtr = TLDSValuePtr<Speed>;
    using TimePtr = TLDSValuePtr<Time>;
    using NamePtr = TLDSValuePtr<FName>;
    using Vec2Ptr = TLDSValuePtr<Vec2>;
    using Vec3Ptr = TLDSValuePtr<Vec3>;
    using ColorPtr = TLDSValuePtr<Color>;
}

#include "PhoenixSim/LDS/ObjectModel/LDSValuePtr.inl"