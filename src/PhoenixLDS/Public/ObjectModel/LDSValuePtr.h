
#pragma once

#include "LDSRecordPtr.h"
#include "LDSRecordQueryFlags.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSValuePtrBase : LDSRecordPtr
    {
        LDSValuePtrBase() = default;
        LDSValuePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValuePtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSValuePtr : LDSValuePtrBase
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
}

#include "LDSValuePtr.inl"