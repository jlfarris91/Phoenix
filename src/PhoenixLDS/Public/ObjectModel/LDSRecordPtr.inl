#pragma once

#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    template <class T>
    T LDSRecordPtr::GetRecordValueAs(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return context.QueryObjectRecordValueAs<T>(Path, defaultValue, Flags);
    }

    template <class T>
    TLDSObjectPtr<T> LDSRecordPtr::AsObject() const
    {
        return TLDSObjectPtr<T>(Path, Flags);
    }

    template <class T>
    TLDSObjectRefPtr<T> LDSRecordPtr::AsObjectRef() const
    {
        return TLDSObjectRefPtr<T>(Path, Flags);
    }

    template <class T>
    TLDSValuePtr<T> LDSRecordPtr::AsValue() const
    {
        return TLDSValuePtr<T>(Path, Flags);
    }

    template <class T, class TValuePtr>
    TLDSValueArrayPtr<T> LDSRecordPtr::AsValueArray() const
    {
        return TLDSValueArrayPtr<T, TValuePtr>(Path, Flags);
    }

    template <class T>
    TLDSObjectArrayPtr<T> LDSRecordPtr::AsObjectArray() const
    {
        return TLDSObjectArrayPtr<T>(Path, Flags);
    }

    template <class T, class TObjectPtr>
    TLDSObjectRefArrayPtr<T, TObjectPtr> LDSRecordPtr::AsObjectRefArray() const
    {
        return TLDSObjectRefArrayPtr<T, TObjectPtr>(Path, Flags);
    }

    template <class T>
    TLDSEnumFlagsPtr<T> LDSRecordPtr::AsEnumFlags() const
    {
        return TLDSEnumFlagsPtr<T>(Path, Flags);
    }
}
