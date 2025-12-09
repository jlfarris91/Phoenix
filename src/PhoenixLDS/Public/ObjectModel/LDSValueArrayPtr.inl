
#pragma once

#include "LDSArrayUtil.h"
#include "LDSValueArrayPtr.h"

namespace Phoenix::LDS
{
    template <class TValuePtr>
    TValuePtr LDSValueArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <class TValue, class TValuePtr>
    TValue LDSValueArrayPtr::ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSValuePtrBase, TValue>)
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItemValueAs(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
        requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <class TValue, class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
        requires (!std::is_base_of_v<LDSValuePtrBase, TValue>)
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <class TValue, class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItemValuesAs(
        const ILDSQueryContext& context,
        TContainer& outValues) const
    {
        return LDSArrayUtil::GetItemValuesAs<TValue, TValuePtr>(context, Path, outValues, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValuePtr>::TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSValueArrayPtrBase(path, flags)
    {
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValuePtr>::TLDSValueArrayPtr(const LDSValueArrayPtrBase& other)
        : LDSValueArrayPtrBase(other)
    {
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValuePtr>::operator LDSValueArrayPtr() const
    {
        return LDSValueArrayPtr(Path, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValuePtr>::operator TLDSValueArrayPtr<typename TValuePtr::ValueT, TValuePtr>() const
    {
        return TLDSValueArrayPtr<typename TValuePtr::ValueT, TValuePtr>(Path, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TValuePtr TLDSValueArrayPtr<TValuePtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    typename TLDSValueArrayPtr<TValuePtr>::ValueT TLDSValueArrayPtr<TValuePtr>::ItemValue(
        const ILDSQueryContext& context,
        uint32 index,
        const ValueT& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<ValueT, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TCallback>
    const TLDSValueArrayPtr<TValuePtr>& TLDSValueArrayPtr<TValuePtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TCallback>
    const TLDSValueArrayPtr<TValuePtr>& TLDSValueArrayPtr<TValuePtr>::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<ValueT, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValuePtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValuePtr>::GetItemValues(const ILDSQueryContext& context, TContainer& outValues) const
    {
        return LDSArrayUtil::GetItemValuesAs<ValueT, TValuePtr>(context, Path, outValues, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValue, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSValueArrayPtrBase(path, flags)
    {
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValue, TValuePtr>::TLDSValueArrayPtr(const LDSValueArrayPtrBase& other)
        : LDSValueArrayPtrBase(other)
    {
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValue, TValuePtr>::operator LDSValueArrayPtr() const
    {
        return LDSValueArrayPtr(Path, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TLDSValueArrayPtr<TValue, TValuePtr>::operator TLDSValueArrayPtr<TValuePtr>() const
    {
        return TLDSValueArrayPtr<TValuePtr>(Path, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TValuePtr TLDSValueArrayPtr<TValue, TValuePtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    TValue TLDSValueArrayPtr<TValue, TValuePtr>::ItemValue(
        const ILDSQueryContext& context,
        uint32 index,
        const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TCallback>
    const TLDSValueArrayPtr<TValue, TValuePtr>& TLDSValueArrayPtr<TValue, TValuePtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TCallback>
    const TLDSValueArrayPtr<TValue, TValuePtr>& TLDSValueArrayPtr<TValue, TValuePtr>::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItemValues(
        const ILDSQueryContext& context,
        TContainer& outValues) const
    {
        return LDSArrayUtil::GetItemValuesAs<TValue, TValuePtr>(context, Path, outValues, Flags);
    }
}
