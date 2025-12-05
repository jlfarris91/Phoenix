
#pragma once

#include "DLLExport.h"
#include "LDSQueryContext.h"
#include "LDSRecordPath.h"
#include "LDSRecordPtr.h"
#include "LDSRecordQueryFlags.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSValuePtr : LDSRecordPtr
    {
        LDSValuePtr() = default;
        LDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValuePtr(const LDSRecordPtr& other);

        template <class T>
        T GetValue(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        template <class T>
        bool TryGetValue(const ILDSQueryContext& context, T& outValue) const;
    };

    template <class T>
    struct PHOENIX_LDS_API TLDSValuePtr : LDSValuePtr
    {
        TLDSValuePtr() = default;
        TLDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValuePtr(const LDSRecordPtr& other);

        T GetValue(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        bool TryGetValue(const ILDSQueryContext& context, T& outValue) const;
    };
}