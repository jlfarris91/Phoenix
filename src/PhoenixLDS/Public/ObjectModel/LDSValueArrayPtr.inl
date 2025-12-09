
#pragma once

#include "LDSValueArrayPtr.h"

namespace Phoenix::LDS
{
    template <class TValuePtr>
    TValuePtr LDSValueArrayPtr::Item(uint32 index) const
    {
        return LDSArrayPtrBase::Item<TValuePtr>(index);
    }

    template <class TValue, class TValuePtr>
    TValue LDSValueArrayPtr::ItemValue(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue) const
    {
        return Item<TValuePtr>(index).GetValue(context, defaultValue);
    }

    template <class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSValuePtr, TValuePtr>)
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, Item<TValuePtr>(i));
        }
        return *this;
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSValuePtr, TValue>)
    {
        return ForEachItem<TValuePtr>(context, callback);
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, ItemValue<TValue, TValuePtr>(context, i));
        }
        return *this;
    }

    template <class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
        requires (std::is_base_of_v<LDSValuePtr, TValuePtr>)
    {
        uint32 count = 0;
        ForEachItem<TValuePtr>(context, [&count, &outItems](uint32, const TValuePtr& value)
        {
            outItems.Add(value);
            ++count;
        });
        return count;
    }

    template <class TValue, class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outValues) const
        requires (!std::is_base_of_v<LDSValuePtr, TValue>)
    {
        return GetItems<TValuePtr>(context, outValues);
    }

    template <class TValue, class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItemValues(
        const ILDSQueryContext& context,
        TContainer& outValues) const
    {
        uint32 count = 0;
        ForEachItemValue<TValue, TValuePtr>(context, [&count, &outValues](uint32, const TValue& value)
        {
            outValues.Add(value);
            ++count;
        });
        return count;
    }

    template <class T, class TValuePtr>
    TLDSValueArrayPtr<T, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSArrayPtrBase(path, flags)
    {
    }

    template <class T, class TValuePtr>
    TLDSValueArrayPtr<T, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPtr& other)
        : LDSArrayPtrBase(other)
    {
    }

    template <class T, class TValuePtr>
    TValuePtr TLDSValueArrayPtr<T, TValuePtr>::Item(uint32 index) const
    {
        return LDSArrayPtrBase::Item<T, TValuePtr>(index);
    }

    template <class T, class TValuePtr>
    T TLDSValueArrayPtr<T, TValuePtr>::ItemValue(
        const ILDSQueryContext& context,
        uint32 index,
        const T& defaultValue) const
    {
        return Item(index).GetValue(context, defaultValue);
    }

    template <class T, class TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<T, TValuePtr>& TLDSValueArrayPtr<T, TValuePtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, Item(i));
        }
        return *this;
    }

    template <class T, class TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<T, TValuePtr>& TLDSValueArrayPtr<T, TValuePtr>::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, ItemValue(i));
        }
        return *this;
    }

    template <class TValue, class TValuePtr>
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        uint32 count = 0;
        ForEachItem(context, [&count, &outItems](uint32, const TValuePtr& value)
        {
            outItems.Add(value);
            ++count;
        });
        return count;
    }

    template <class TValue, class TValuePtr>
    template <class TContainer>
    uint32 TLDSValueArrayPtr<TValue, TValuePtr>::GetItemValues(
        const ILDSQueryContext& context,
        TContainer& outValues) const
    {
        uint32 count = 0;
        ForEachItemValue(context, [&count, &outValues](uint32, const TValue& value)
        {
            outValues.Add(value);
            ++count;
        });
        return count;
    }
}
