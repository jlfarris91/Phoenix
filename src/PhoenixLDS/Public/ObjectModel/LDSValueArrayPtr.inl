
#pragma once

#include "LDSArrayUtil.h"

namespace Phoenix::LDS
{
    template <IsValuePtr TValuePtr>
    TValuePtr LDSValueArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TValue LDSValueArrayPtr::ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <IsValuePtr TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItemValueAs(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsValuePtr TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(const ILDSQueryContext& context, TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(const ILDSQueryContext& context, TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItemValuesAs(const ILDSQueryContext& context, TContainer& outValues) const
    {
        return LDSArrayUtil::GetItemValuesAs<TValue, TValuePtr>(context, Path, outValues, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValue, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSValueArrayPtrBase(path, flags)
    {
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValue, TValuePtr>::TLDSValueArrayPtr(const LDSValueArrayPtrBase& other)
        : LDSValueArrayPtrBase(other)
    {
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValue, TValuePtr>::operator LDSValueArrayPtr() const
    {
        return LDSValueArrayPtr(Path, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValue, TValuePtr>::operator TLDSValueArrayPtr<TValuePtr>() const
    {
        return TLDSValueArrayPtr<TValuePtr>(Path, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TValuePtr TLDSValueArrayPtr<TValue, TValuePtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TValue TLDSValueArrayPtr<TValue, TValuePtr>::ItemValue(
        const ILDSQueryContext& context,
        uint32 index,
        const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<TValue, TValuePtr>& TLDSValueArrayPtr<TValue, TValuePtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<TValue, TValuePtr>& TLDSValueArrayPtr<TValue, TValuePtr>::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TValuePtr>(context, Path, outItems, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItemValues(
        const ILDSQueryContext& context,
        TContainer& outValues) const
    {
        return LDSArrayUtil::GetItemValuesAs<TValue, TValuePtr>(context, Path, outValues, Flags);
    }

    template <IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValuePtr, EnableIfValuePtr<TValuePtr>>::TLDSValueArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSValueArrayPtr<typename TValuePtr::ValueT, TValuePtr>(path, flags)
    {
    }

    template <IsValuePtr TValuePtr>
    TLDSValueArrayPtr<TValuePtr, EnableIfValuePtr<TValuePtr>>::TLDSValueArrayPtr(const LDSValueArrayPtrBase& other)
        : TLDSValueArrayPtr<typename TValuePtr::ValueT, TValuePtr>(other)
    {
    }

    template <IsNotRecordPtr TValue>
    TLDSValueArrayPtr<TValue, EnableIfNotRecordPtr<TValue>>::TLDSValueArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSValueArrayPtr<TValue, TLDSValuePtr<TValue>>(path, flags)
    {
    }

    template <IsNotRecordPtr TValue>
    TLDSValueArrayPtr<TValue, EnableIfNotRecordPtr<TValue>>::TLDSValueArrayPtr(const LDSValueArrayPtrBase& other)
        : TLDSValueArrayPtr<TValue, TLDSValuePtr<TValue>>(other)
    {
    }
}
