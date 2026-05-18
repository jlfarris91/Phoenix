
#pragma once

#include "Phoenix.Sim/LDS/ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <IsNotRecordPtr TValue>
    TValue LDSValuePtr::GetValue(const ILDSQueryContext& context, const TValue& defaultValue) const
    {
        return context.QueryRecordValueAs<TValue>(Path, defaultValue, Flags);
    }

    template <IsNotRecordPtr TValue>
    bool LDSValuePtr::TryGetValue(const ILDSQueryContext& context, TValue& outValue) const
    {
        return context.TryQueryRecordValueAs<TValue>(Path, outValue, Flags);
    }

    template <IsNotRecordPtr TValue>
    TLDSValuePtr<TValue>::TLDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSValuePtrBase(path, flags)
    {
    }

    template <IsNotRecordPtr TValue>
    TLDSValuePtr<TValue>::TLDSValuePtr(const LDSValuePtrBase& other)
        : LDSValuePtrBase(other)
    {
    }

    template <IsNotRecordPtr TValue>
    TLDSValuePtr<TValue>::operator LDSValuePtr() const
    {
        return LDSValuePtr(Path, Flags);
    }

    template <IsNotRecordPtr TValue>
    TValue TLDSValuePtr<TValue>::GetValue(const ILDSQueryContext& context, const TValue& defaultValue) const
    {
        return context.QueryRecordValueAs<TValue>(Path, defaultValue, Flags);
    }

    template <IsNotRecordPtr TValue>
    bool TLDSValuePtr<TValue>::TryGetValue(const ILDSQueryContext& context, TValue& outValue) const
    {
        return context.TryQueryRecordValueAs<TValue>(Path, outValue, Flags);
    }
}
