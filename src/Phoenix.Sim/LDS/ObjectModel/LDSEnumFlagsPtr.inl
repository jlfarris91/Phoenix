
#pragma once

#include "Phoenix.Sim/Flags.h"
#include "Phoenix.Sim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class TUnderlyingType>
    TUnderlyingType LDSEnumFlagsPtr::GetValue(const ILDSQueryContext& context, const TUnderlyingType& defaultValue) const
    {
        return context.QueryRecordValueAs<TUnderlyingType>(Path, defaultValue, Flags);
    }

    template <class TUnderlyingType>
    bool LDSEnumFlagsPtr::TryGetValue(const ILDSQueryContext& context, TUnderlyingType& outValue) const
    {
        return context.TryQueryRecordValueAs<TUnderlyingType>(Path, outValue, Flags);
    }

    template <class TUnderlyingType, class U>
    bool LDSEnumFlagsPtr::HasAnyFlags(const ILDSQueryContext& context, U value) const
    {
        return Phoenix::HasAnyFlags(GetValue<TUnderlyingType>(context), value);
    }

    template <class TUnderlyingType, class ... Us>
    bool LDSEnumFlagsPtr::HasAnyFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasAnyFlags(GetValue<TUnderlyingType>(context), std::forward<Us>(args)...);
    }

    template <class TUnderlyingType, class U>
    bool LDSEnumFlagsPtr::HasAllFlags(const ILDSQueryContext& context, U value)
    {
        return Phoenix::HasAllFlags(GetValue<TUnderlyingType>(context), value);
    }

    template <class TUnderlyingType, class ... Us>
    bool LDSEnumFlagsPtr::HasAllFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasAllFlags(GetValue<TUnderlyingType>(context), std::forward<Us>(args)...);
    }

    template <class TUnderlyingType, class U>
    bool LDSEnumFlagsPtr::HasNoneFlags(const ILDSQueryContext& context, U value)
    {
        return Phoenix::HasNoneFlags(GetValue<TUnderlyingType>(context), value);
    }

    template <class TUnderlyingType, class ... Us>
    bool LDSEnumFlagsPtr::HasNoneFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasNoneFlags(GetValue<TUnderlyingType>(context), std::forward<Us>(args)...);
    }

    template <class TUnderlyingType>
    TLDSEnumFlagsPtr<TUnderlyingType>::TLDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSEnumFlagsPtrBase(path, flags)
    {
    }

    template <class TUnderlyingType>
    TLDSEnumFlagsPtr<TUnderlyingType>::TLDSEnumFlagsPtr(const LDSEnumFlagsPtrBase& other)
        : LDSEnumFlagsPtrBase(other)
    {
    }

    template <class TUnderlyingType>
    TLDSEnumFlagsPtr<TUnderlyingType>::operator LDSEnumFlagsPtr() const
    {
        return LDSEnumFlagsPtr(Path, Flags);
    }

    template <class TUnderlyingType>
    TUnderlyingType TLDSEnumFlagsPtr<TUnderlyingType>::GetValue(const ILDSQueryContext& context, const TUnderlyingType& defaultValue) const
    {
        return context.QueryRecordValueAs<TUnderlyingType>(Path, defaultValue, Flags);
    }

    template <class TUnderlyingType>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::TryGetValue(const ILDSQueryContext& context, TUnderlyingType& outValue) const
    {
        return context.TryQueryRecordValueAs<TUnderlyingType>(Path, outValue, Flags);
    }

    template <class TUnderlyingType>
    template <class U>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasAnyFlags(const ILDSQueryContext& context, U value) const
    {
        return this->HasAnyFlags<TUnderlyingType>(context, value);
    }

    template <class TUnderlyingType>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasAnyFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return this->HasAnyFlags<TUnderlyingType>(context, std::forward<Us>(args)...);
    }

    template <class TUnderlyingType>
    template <class U>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasAllFlags(const ILDSQueryContext& context, U value)
    {
        return this->HasAllFlags<TUnderlyingType>(context, value);
    }

    template <class TUnderlyingType>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasAllFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return this->HasAllFlags<TUnderlyingType>(context, std::forward<Us>(args)...);
    }

    template <class TUnderlyingType>
    template <class U>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasNoneFlags(const ILDSQueryContext& context, U value)
    {
        return this->HasNoneFlags<TUnderlyingType>(context, value);
    }

    template <class TUnderlyingType>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<TUnderlyingType>::HasNoneFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return this->HasNoneFlags<TUnderlyingType>(context, std::forward<Us>(args)...);
    }
}
