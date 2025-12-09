#pragma once

#include "LDSEnumTypePtr.h"

namespace Phoenix::LDS
{
    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSEnumTypeItemPtr<TValue, TValuePtr>::TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSEnumTypeItemPtr<TValue, TValuePtr>::TLDSEnumTypeItemPtr(const LDSEnumTypeItemPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    void TLDSEnumTypeItemPtr<TValue, TValuePtr>::InitCommon()
    {
        Key = LDSObjectPtr::Value<FName>("key");
        Value = LDSObjectPtr::Value<TValue, TValuePtr>("value");
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSEnumTypeItemPtr<TValuePtr>::TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : TLDSEnumTypeItemPtr(path, flags)
    {
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSEnumTypeItemPtr<TValuePtr>::TLDSEnumTypeItemPtr(const LDSEnumTypeItemPtr& other)
        : TLDSEnumTypeItemPtr(other)
    {
    }

    template <class TUnderlyingValue>
    TUnderlyingValue LDSEnumTypePtr::GetEnumValue(const ILDSQueryContext& context, const FName& key) const
    {
        return GetEnumItem(context, key).Value.GetValue(context, TUnderlyingValue{});
    }

    template <class TUnderlyingValue>
    bool LDSEnumTypePtr::TryGetEnumValue(
        const ILDSQueryContext& context,
        const FName& key,
        TUnderlyingValue& outValue) const
    {
        return GetEnumItem(context, key).Value.TryGetValue(context, outValue);
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TEnumTypeItemPtr>::TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSEnumTypePtrBase(path, flags)
    {
        InitCommon();
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TEnumTypeItemPtr>::TLDSEnumTypePtr(const LDSEnumTypePtrBase& other)
        : LDSEnumTypePtrBase(other)
    {
        InitCommon();
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TEnumTypeItemPtr TLDSEnumTypePtr<TEnumTypeItemPtr>::GetEnumItem(
        const ILDSQueryContext& context,
        const FName& key) const
    {
        TEnumTypeItemPtr itemPtr;
        (void)TryGetEnumItem(context, key, itemPtr);
        return itemPtr;
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    bool TLDSEnumTypePtr<TEnumTypeItemPtr>::TryGetEnumItem(
        const ILDSQueryContext& context,
        const FName& key,
        TEnumTypeItemPtr& outItemPtr) const
    {
        bool foundItem = false;
        Items.ForEachItem(context, [&](const TEnumTypeItemPtr& itemPtr)
        {
            if (itemPtr.Key.GetValue(context, FName::None) == key)
            {
                foundItem = true;
                outItemPtr = itemPtr;
            }
        });
        return foundItem;
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    typename TLDSEnumTypePtr<TEnumTypeItemPtr>::ValueT TLDSEnumTypePtr<TEnumTypeItemPtr>::GetEnumValue(
        const ILDSQueryContext& context,
        const FName& key) const
    {
        return GetEnumItem(context, key).Value.GetValue(context, ValueT{});
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    bool TLDSEnumTypePtr<TEnumTypeItemPtr>::TryGetEnumValue(
        const ILDSQueryContext& context,
        const FName& key,
        ValueT& outValue) const
    {
        return GetEnumItem(context, key).Value.TryGetValue(context, outValue);
    }

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    void TLDSEnumTypePtr<TEnumTypeItemPtr>::InitCommon()
    {
        UnderlyingType = Value<ELDSValueType>("underlying_type");
        Items = ValueArray<TEnumTypeItemPtr>("items");
        DefaultValue = Value<FName>("default");
    }

    template <class TValuePtr, class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TValuePtr, TEnumTypeItemPtr>::TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSEnumTypePtrBase(path, flags)
    {
    }

    template <class TValuePtr, class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TValuePtr, TEnumTypeItemPtr>::TLDSEnumTypePtr(const LDSEnumTypePtrBase& other)
        : LDSEnumTypePtrBase(other)
    {
    }

    template <class TValue, class TValuePtr, class TEnumTypeItemPtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && !std::is_base_of_v<LDSEnumTypeItemPtr, TValue> &&
              std::is_base_of_v<LDSValuePtrBase, TValuePtr> &&
              std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TValue, TValuePtr, TEnumTypeItemPtr>::TLDSEnumTypePtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSEnumTypePtrBase(path, flags)
    {
    }

    template <class TValue, class TValuePtr, class TEnumTypeItemPtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && !std::is_base_of_v<LDSEnumTypeItemPtr, TValue> &&
              std::is_base_of_v<LDSValuePtrBase, TValuePtr> &&
              std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    TLDSEnumTypePtr<TValue, TValuePtr, TEnumTypeItemPtr>::TLDSEnumTypePtr(const LDSEnumTypePtrBase& other)
        : LDSEnumTypePtrBase(other)
    {
    }
}
