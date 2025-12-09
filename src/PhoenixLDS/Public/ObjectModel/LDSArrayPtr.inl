
#pragma once

#include "LDSArrayUtil.h"

namespace Phoenix::LDS
{
    template <IsRecordPtr TItemPtr>
    TItemPtr LDSArrayPtr::Item(uint32 index) const
    {
        return TItemPtr(Path.Append(index), Flags);
    }

    template <IsValuePtr TValuePtr>
    TValuePtr LDSArrayPtr::ItemAsValue(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TValuePtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TValue LDSArrayPtr::ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue) const
    {
        return LDSArrayUtil::GetItemValueAs<TValue, TValuePtr>(context, Path, index, defaultValue, Flags);
    }

    template <IsObjectPtr TObjectPtr>
    TObjectPtr LDSArrayPtr::ItemAsObject(uint32 index) const
    {
        return Item<TObjectPtr>(index);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TObjectPtr LDSArrayPtr::ItemAsObject(uint32 index) const
    {
        return Item<TObjectPtr>(index);
    }

    template <IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr LDSArrayPtr::ItemAsObjectRef(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsObjectPtr TObjectRef, IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr LDSArrayPtr::ItemAsObjectRef(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr LDSArrayPtr::ItemAsObjectRef(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectPtr LDSArrayPtr::ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectPtr LDSArrayPtr::ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <IsRecordPtr TItemPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, Item<TItemPtr>(i));
        }
        return *this;
    }

    template <IsValuePtr TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsValue(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const
    {
        return ForEachItem<TObjectPtr>(context, callback);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const
    {
        return ForEachItem<TObjectPtr>(context, callback);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            TObjectPtr objectPtr = Item<TObjectPtr>(i);
            TObject object;
            if (objectPtr.TryReadObject(context, object))
            {
                callback(i, object);
            }
        }
        return *this;
    }

    template <IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }
}
