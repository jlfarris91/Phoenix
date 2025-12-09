
#pragma once

#include "DLLExport.h"
#include "LDSQueryContext.h"
#include "LDSRecordPath.h"
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

        template <class T>
        T GetValue(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        template <class T>
        bool TryGetValue(const ILDSQueryContext& context, T& outValue) const;
    };

    template <class TValue>
    struct PHOENIX_LDS_API TLDSValuePtr : LDSValuePtrBase
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