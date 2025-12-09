
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

    template <class TValue>
    TLDSValuePtr<TValue>::TLDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSValuePtrBase(path, flags)
    {
    }

    template <class TValue>
    TLDSValuePtr<TValue>::TLDSValuePtr(const LDSValuePtrBase& other)
        : LDSValuePtrBase(other)
    {
    }

    template <class TValue>
    TLDSValuePtr<TValue>::operator LDSValuePtr() const
    {
        return LDSValuePtr(Path, Flags);
    }

    template <class TValue>
    TValue TLDSValuePtr<TValue>::GetValue(const ILDSQueryContext& context, const TValue& defaultValue) const
    {
        return LDSValuePtr::GetValue<TValue>(context, defaultValue);
    }

    template <class TValue>
    bool TLDSValuePtr<TValue>::TryGetValue(const ILDSQueryContext& context, TValue& outValue) const
    {
        return LDSValuePtr::TryGetValue<TValue>(context, outValue);
    }
}
