
#pragma once

#include "LDSEnumFlagsPtr.h"
#include "Flags.h"

namespace Phoenix::LDS
{
    template <class T>
    T LDSEnumFlagsPtr::GetValue(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return context.QueryRecordValueAs<T>(Path, defaultValue, Flags);
    }

    template <class T>
    bool LDSEnumFlagsPtr::TryGetValue(const ILDSQueryContext& context, T& outValue) const
    {
        return context.TryQueryRecordValueAs<T>(Path, outValue, Flags);
    }

    template <class T, class U>
    bool LDSEnumFlagsPtr::HasAnyFlags(const ILDSQueryContext& context, U value) const
    {
        return Phoenix::HasAnyFlags(GetValue<T>(context), value);
    }

    template <class T, class ... Us>
    bool LDSEnumFlagsPtr::HasAnyFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasAnyFlags(GetValue<T>(context), std::forward<Us>(args)...);
    }

    template <class T, class U>
    bool LDSEnumFlagsPtr::HasAllFlags(const ILDSQueryContext& context, U value)
    {
        return Phoenix::HasAllFlags(GetValue<T>(context), value);
    }

    template <class T, class ... Us>
    bool LDSEnumFlagsPtr::HasAllFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasAllFlags(GetValue<T>(context), std::forward<Us>(args)...);
    }

    template <class T, class U>
    bool LDSEnumFlagsPtr::HasNoneFlags(const ILDSQueryContext& context, U value)
    {
        return Phoenix::HasNoneFlags(GetValue<T>(context), value);
    }

    template <class T, class ... Us>
    bool LDSEnumFlagsPtr::HasNoneFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return Phoenix::HasNoneFlags(GetValue<T>(context), std::forward<Us>(args)...);
    }

    template <class T>
    TLDSEnumFlagsPtr<T>::TLDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags): LDSEnumFlagsPtr(path, flags)
    {
    }

    template <class T>
    TLDSEnumFlagsPtr<T>::TLDSEnumFlagsPtr(const LDSRecordPtr& other): LDSEnumFlagsPtr(other)
    {
    }

    template <class T>
    T TLDSEnumFlagsPtr<T>::GetValue(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return LDSEnumFlagsPtr::GetValue<T>(context, defaultValue);
    }

    template <class T>
    bool TLDSEnumFlagsPtr<T>::TryGetValue(const ILDSQueryContext& context, T& outValue) const
    {
        return LDSEnumFlagsPtr::TryGetValue<T>(context, outValue);
    }

    template <class T>
    template <class U>
    bool TLDSEnumFlagsPtr<T>::HasAnyFlags(const ILDSQueryContext& context, U value) const
    {
        return LDSEnumFlagsPtr::HasAnyFlags<T>(context, value);
    }

    template <class T>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<T>::HasAnyFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return LDSEnumFlagsPtr::HasAnyFlags<T>(context, std::forward<Us>(args)...);
    }

    template <class T>
    template <class U>
    bool TLDSEnumFlagsPtr<T>::HasAllFlags(const ILDSQueryContext& context, U value)
    {
        return LDSEnumFlagsPtr::HasAllFlags<T>(context, value);
    }

    template <class T>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<T>::HasAllFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return LDSEnumFlagsPtr::HasAllFlags<T>(context, std::forward<Us>(args)...);
    }

    template <class T>
    template <class U>
    bool TLDSEnumFlagsPtr<T>::HasNoneFlags(const ILDSQueryContext& context, U value)
    {
        return LDSEnumFlagsPtr::HasNoneFlags<T>(context, value);
    }

    template <class T>
    template <class ... Us>
    bool TLDSEnumFlagsPtr<T>::HasNoneFlags(const ILDSQueryContext& context, Us&&... args)
    {
        return LDSEnumFlagsPtr::HasNoneFlags<T>(context, std::forward<Us>(args)...);
    }
}
