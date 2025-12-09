
#pragma once

#include "LDSArrayPtr.h"
#include "LDSArrayUtil.h"

namespace Phoenix::LDS
{
    template <class TItemPtr>
    TItemPtr LDSArrayPtr::Item(uint32 index) const
    {
        return TItemPtr(Path.Append(index), Flags);
    }

    template <class TValuePtr>
    TValuePtr LDSArrayPtr::ItemAsValue(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <class TValue, class TValuePtr>
    TValue LDSArrayPtr::ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <class T>
    TLDSObjectPtr<T> LDSArrayPtr::ItemAsObject(uint32 index) const
    {
        return Item<TLDSObjectPtr<T>>(index);
    }

    template <class TObjectRefPtr>
    TObjectRefPtr LDSArrayPtr::ItemAsObjectRef(uint32 index) const
        requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectRef, class TObjectRefPtr>
    TObjectRefPtr LDSArrayPtr::ItemAsObjectRef(uint32 index) const
        requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectRef>)
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    TObjectPtr LDSArrayPtr::ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <class TItemPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, Item<TItemPtr>(i));
        }
        return *this;
    }

    template <class TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSValuePtrBase, TValue>)
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TValue, class TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return ForEachItem<TObjectPtr>(context, callback);
    }

    template <class T, class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, T>)
    {
        return ForEachItem<TObjectPtr>(context, callback);
    }

    template <class T, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            TLDSObjectPtr<T> objectPtr = Item<TLDSObjectPtr<T>>(i);
            T object;
            if (objectPtr.TryReadObject(context, object))
            {
                callback(i, object);
            }
        }
        return *this;
    }

    template <class TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }
}
