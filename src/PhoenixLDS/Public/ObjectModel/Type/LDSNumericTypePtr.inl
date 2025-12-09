#pragma once

#include "LDSNumericTypePtr.h"

namespace Phoenix::LDS
{
    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValue, TValuePtr>::TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValue, TValuePtr>::operator LDSNumericTypePtr() const
    {
        return LDSNumericTypePtr(Path, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValue, TValuePtr>::operator TLDSNumericTypePtr<TValuePtr>() const
    {
        return TLDSNumericTypePtr<TValuePtr>(Path, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValue, TValuePtr>::TLDSNumericTypePtr(const LDSNumericTypePtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    void TLDSNumericTypePtr<TValue, TValuePtr>::InitCommon()
    {
        DefaultValue = Value<TValue, TValuePtr>("default");
        MinValue = Value<TValue, TValuePtr>("min");
        MaxValue = Value<TValue, TValuePtr>("max");
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValuePtr>::TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : TLDSNumericTypePtr(path, flags)
    {
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSNumericTypePtr<TValuePtr>::TLDSNumericTypePtr(const LDSNumericTypePtr& other)
        : TLDSNumericTypePtr(other)
    {
    }
}
