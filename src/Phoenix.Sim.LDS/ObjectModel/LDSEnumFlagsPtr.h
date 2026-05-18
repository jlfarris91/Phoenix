
#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSRecordPtr.h"

// TODO (jfarris): do we even need this?
namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSEnumFlagsPtrBase : LDSRecordPtr
    {
        LDSEnumFlagsPtrBase() = default;
        LDSEnumFlagsPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumFlagsPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_SIM_API LDSEnumFlagsPtr : LDSEnumFlagsPtrBase
    {
        LDSEnumFlagsPtr() = default;
        LDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumFlagsPtr(const LDSEnumFlagsPtrBase& other);

        template <class TUnderlyingType>
        TUnderlyingType GetValue(const ILDSQueryContext& context, const TUnderlyingType& defaultValue = {}) const;

        template <class TUnderlyingType>
        bool TryGetValue(const ILDSQueryContext& context, TUnderlyingType& outValue) const;

        template <class TUnderlyingType, class U>
        bool HasAnyFlags(const ILDSQueryContext& context, U value) const;

        template <class TUnderlyingType, class ...Us>
        bool HasAnyFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class TUnderlyingType, class U>
        bool HasAllFlags(const ILDSQueryContext& context, U value);

        template <class TUnderlyingType, class ...Us>
        bool HasAllFlags(const ILDSQueryContext& context, Us&& ...args);

        template <class TUnderlyingType, class U>
        bool HasNoneFlags(const ILDSQueryContext& context, U value);

        template <class TUnderlyingType, class ...Us>
        bool HasNoneFlags(const ILDSQueryContext& context, Us&& ...args);
    };

    template <class TUnderlyingType>
    struct TLDSEnumFlagsPtr : LDSEnumFlagsPtrBase
    {
        TLDSEnumFlagsPtr() = default;
        TLDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumFlagsPtr(const LDSEnumFlagsPtrBase& other);

        operator LDSEnumFlagsPtr() const;

        TUnderlyingType GetValue(const ILDSQueryContext& context, const TUnderlyingType& defaultValue = {}) const;

        bool TryGetValue(const ILDSQueryContext& context, TUnderlyingType& outValue) const;

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

#include "Phoenix.Sim.LDS/ObjectModel/LDSEnumFlagsPtr.inl"