
#pragma once

#include "LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <class T>
    T LDSValuePtr::GetValue(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return context.QueryRecordValueAs<T>(Path, defaultValue, Flags);
    }

    template <class T>
    bool LDSValuePtr::TryGetValue(const ILDSQueryContext& context, T& outValue) const
    {
        return context.TryQueryRecordValueAs<T>(Path, outValue, Flags);
    }

    template <class T>
    TLDSValuePtr<T>::TLDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags): LDSValuePtr(path, flags)
    {
    }

    template <class T>
    TLDSValuePtr<T>::TLDSValuePtr(const LDSRecordPtr& other): LDSValuePtr(other)
    {
    }

    template <class T>
    T TLDSValuePtr<T>::GetValue(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return LDSValuePtr::GetValue<T>(context, defaultValue);
    }

    template <class T>
    bool TLDSValuePtr<T>::TryGetValue(const ILDSQueryContext& context, T& outValue) const
    {
        return LDSValuePtr::TryGetValue<T>(context, outValue);
    }
}
