#pragma once

#include "LDSEnumTypePtr.h"

namespace Phoenix::LDS
{
    template <class T, class TValuePtr>
    TLDSEnumTypeItemPtr<T, TValuePtr>::TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    TLDSEnumTypeItemPtr<T, TValuePtr>::TLDSEnumTypeItemPtr(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    void TLDSEnumTypeItemPtr<T, TValuePtr>::InitCommon()
    {
        Key = LDSObjectPtr::Value<FName>("key");
        Value = LDSObjectPtr::Value("value");
    }

    template <class T, class TValuePtr>
    TLDSEnumTypePtr<T, TValuePtr>::TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    TLDSEnumTypePtr<T, TValuePtr>::TLDSEnumTypePtr(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    void TLDSEnumTypePtr<T, TValuePtr>::InitCommon()
    {
        UnderlyingType = Value<ELDSValueType>("underlying_type");
        Items = ValueArray<T, TValuePtr>("items");
        DefaultValue = Value<T>("default");
    }
}
