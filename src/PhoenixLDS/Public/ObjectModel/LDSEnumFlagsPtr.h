
#pragma once

#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSEnumFlagsPtr : LDSRecordPtr
    {
        LDSEnumFlagsPtr() = default;
        LDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumFlagsPtr(const LDSRecordPtr& other);

        template <class T>
        T GetValue(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        template <class T>
        bool TryGetValue(const ILDSQueryContext& context, T& outValue) const;

        template <class T, class U>
        bool HasAnyFlags(const ILDSQueryContext& context, U value) const;

        template <class T, class ...Us>
        bool HasAnyFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class T, class U>
        bool HasAllFlags(const ILDSQueryContext& context, U value);

        template <class T, class ...Us>
        bool HasAllFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class T, class U>
        bool HasNoneFlags(const ILDSQueryContext& context, U value);

        template <class T, class ...Us>
        bool HasNoneFlags(const ILDSQueryContext& context, Us&& ...args);
    };

    template <class T>
    struct TLDSEnumFlagsPtr : LDSEnumFlagsPtr
    {
        TLDSEnumFlagsPtr() = default;
        TLDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumFlagsPtr(const LDSRecordPtr& other);

        T GetValue(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        bool TryGetValue(const ILDSQueryContext& context, T& outValue) const;

        template <class U>
        bool HasAnyFlags(const ILDSQueryContext& context, U value) const;

        template <class ...Us>
        bool HasAnyFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class U>
        bool HasAllFlags(const ILDSQueryContext& context, U value);

        template <class ...Us>
        bool HasAllFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class U>
        bool HasNoneFlags(const ILDSQueryContext& context, U value);

        template <class ...Us>
        bool HasNoneFlags(const ILDSQueryContext& context, Us&& ...args);
    };
}