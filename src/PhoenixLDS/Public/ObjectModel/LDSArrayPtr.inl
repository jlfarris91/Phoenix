
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    template <class TItemPtr>
    TItemPtr LDSArrayPtrBase::Item(uint32 index) const
    {
        return TItemPtr(Path.Append(index), Flags);
    }

    template <class T>
    TLDSValuePtr<T> LDSArrayPtr::ItemAsValue(uint32 index) const
    {
        return Item<TLDSValuePtr<T>>(index);
    }

    template <class T>
    T LDSArrayPtr::ItemValueAs(const ILDSQueryContext& context, uint32 index, const T& defaultValue) const
    {
        return Item<LDSValuePtr>(index).GetValue<T>(context, defaultValue);
    }

    template <class T>
    TLDSObjectPtr<T> LDSArrayPtr::ItemAsObject(uint32 index) const
    {
        return Item<TLDSObjectPtr<T>>(index);
    }

    template <class T>
    TLDSObjectRefPtr<T> LDSArrayPtr::ItemAsObjectRef(uint32 index) const
    {
        return Item<TLDSObjectRefPtr<T>>(index);
    }

    template <class TObjectPtr>
    TObjectPtr LDSArrayPtr::ResolveItemObject(const ILDSQueryContext& context, uint32 index) const requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>)
    {
        return Item<LDSObjectRefPtr>(index).ResolveObject(context);
    }

    template <class T, class TObjectPtr>
    TObjectPtr LDSArrayPtr::ResolveItemObject(const ILDSQueryContext& context, uint32 index) const requires (!std::is_base_of_v<LDSObjectPtr, T>)
    {
        return Item<TLDSObjectRefPtr<T, TObjectPtr>>(index).ResolveObject(context, index);
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
        requires (std::is_base_of_v<LDSValuePtr, TValuePtr>)
    {
        return ForEachItem<TValuePtr>(context, callback);
    }

    template <class T, class TValuePtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSValuePtr, T>)
    {
        return ForEachItem<TValuePtr>(context, callback);
    }

    template <class T, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, ItemValueAs<T>(context, i));
        }
        return *this;
    }

    template <class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>)
    {
        return ForEachItem<TObjectPtr>(context, callback);
    }

    template <class T, class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectPtr, T>)
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
        requires (std::is_base_of_v<LDSObjectRefPtr, TObjectRefPtr>)
    {
        return ForEachItem<TObjectRefPtr>(context, callback);
    }

    template <class T, class TObjectRefPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsObjectRef(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectRefPtr, T>)
    {
        return ForEachItem<TObjectRefPtr>(context, callback);
    }

    template <class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>)
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, ResolveItemObject<TObjectPtr>(context, i));
        }
        return *this;
    }

    template <class T, class TObjectPtr, class TCallback>
    const LDSArrayPtr& LDSArrayPtr::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectPtr, T>)
    {
        uint32 count = GetSize(context);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, ResolveItemObject<TObjectPtr>(context, i));
        }
        return *this;
    }

    template <class T>
    T LDSValueArrayPtr::ItemValue(const ILDSQueryContext& context, uint32 index, const T& defaultValue) const
    {
        return LDSArrayPtr::ItemValueAs<T>(context, index, defaultValue);
    }

    template <class TValuePtr, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemAsValue<TValuePtr>(context, callback);
    }

    template <class T, class TCallback>
    const LDSValueArrayPtr& LDSValueArrayPtr::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemValueAs<T>(context, callback);
    }

    template <class TValuePtr, class TContainer>
    uint32 LDSValueArrayPtr::GetItems(const ILDSQueryContext& context, TContainer& outItems) const
    {
        uint32 count = 0;
        ForEachItemAsValue<TValuePtr>()(context, [&count, &outItems](uint32, const TValuePtr& value)
        {
            outItems.Add(value);
            ++count;
        });
        return count;
    }

    template <class TValuePtr>
    uint32 LDSValueArrayPtr::GetItems(const ILDSQueryContext& context, TArray2<TValuePtr>& outItems) const
    {
    }

    template <class T, class TValuePtr>
    TLDSValueArrayPtr<T, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSArrayPtr(path, flags)
    {
    }

    template <class T, class TValuePtr>
    TLDSValueArrayPtr<T, TValuePtr>::TLDSValueArrayPtr(const LDSRecordPtr& other)
        : LDSArrayPtr(other)
    {
    }

    template <class T, class TValuePtr>
    TValuePtr TLDSValueArrayPtr<T, TValuePtr>::Item(uint32 index) const
    {
        return LDSArrayPtr::ItemAsValue<T>(index);
    }

    template <class T, class TValuePtr>
    T TLDSValueArrayPtr<T, TValuePtr>::ItemValue(
        const ILDSQueryContext& context,
        uint32 index,
        const T& defaultValue) const
    {
        return LDSArrayPtr::ItemValueAs<T>(context, index, defaultValue);
    }

    template <class T, class TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<T, TValuePtr>& TLDSValueArrayPtr<T, TValuePtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItem<T>(context, callback);
    }

    template <class T, class TValuePtr>
    template <class TCallback>
    const TLDSValueArrayPtr<T, TValuePtr>& TLDSValueArrayPtr<T, TValuePtr>::ForEachItemValue(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemValueAs<T>(context, callback);
    }

    template <class T, class TValuePtr>
    template <class U>
    uint32 TLDSValueArrayPtr<T, TValuePtr>::GetItems(const ILDSQueryContext& context, TArray2<U>& outItems) const
    {
        uint32 count = 0;
        ForEachItem(context, [&count, &outItems](uint32, const TLDSValuePtr<T>& value)
        {
            outItems.Add(value);
            ++count;
        });
        return count;
    }

    template <class T, class TValuePtr>
    template <class U>
    uint32 TLDSValueArrayPtr<T, TValuePtr>::GetItemValues(const ILDSQueryContext& context, TArray2<U>& outValues) const
    {
        uint32 count = 0;
        ForEachItemValue(context, [&count, &outValues](uint32, const T& value)
        {
            outValues.Add(value);
            ++count;
        });
        return count;
    }

    template <class T, class TObjectPtr>
    TLDSObjectArrayPtr<T, TObjectPtr>::TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSArrayPtr(path, flags)
    {
    }

    template <class T, class TObjectPtr>
    TLDSObjectArrayPtr<T, TObjectPtr>::TLDSObjectArrayPtr(const LDSRecordPtr& other)
        : LDSArrayPtr(other)
    {
    }

    template <class T, class TObjectPtr>
    TObjectPtr TLDSObjectArrayPtr<T, TObjectPtr>::Item(uint32 index) const
    {
        return LDSArrayPtr::ItemAsObject(index);
    }

    template <class T, class TObjectPtr>
    template <class TCallback>
    const TLDSObjectArrayPtr<T, TObjectPtr>& TLDSObjectArrayPtr<T, TObjectPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemAsObject<TObjectPtr>(context, callback);
    }

    template <class T, class TObjectPtr>
    template <class U>
    uint32 TLDSObjectArrayPtr<T, TObjectPtr>::GetObjects(const ILDSQueryContext& context, TArray2<U>& outObjects) const
    {
        uint32 count = 0;
        LDSArrayPtr::ForEachItem(context, [&count, &outObjects](uint32, const TLDSObjectPtr<T>& item)
        {
            outObjects.Add(item);
            ++count;
        });
        return count;
    }

    template <class T, class TObjectPtr>
    uint32 TLDSObjectArrayPtr<T, TObjectPtr>::ReadObjects(const ILDSQueryContext& context, TArray2<T>& outObjects) const
    {
        uint32 count = 0;
        LDSArrayPtr::ForEachItemAsReadObject<T>(context, [&count, &outObjects](uint32, const T& object)
        {
            outObjects.Add(object);
            ++count;
        });
        return count;
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSArrayPtr(path, flags)
    {
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtr(const LDSRecordPtr& other)
        : LDSArrayPtr(other)
    {
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    TObjectRefPtr TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::Item(uint32 index) const
    {
        return LDSArrayPtr::ItemAsObjectRef<T>(index);
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    TObjectPtr TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::ResolvedItem(
        const ILDSQueryContext& context,
        uint32 index) const
    {
        return ResolveItemObject(context, index);
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    template <class TCallback>
    const TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>& TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemAsObjectRef<T>(context, callback);
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    template <class TCallback>
    const TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>& TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::ForEachResolvedItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        return LDSArrayPtr::ForEachItemAsResolvedObject<T>(context, callback);
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    uint32 TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::GetObjectRefs(
        const ILDSQueryContext& context,
        TArray2<TObjectRefPtr>& outObjectRefs) const
    {
        uint32 count = 0;
        TLDSObjectRefArrayPtr::ForEachItem(context, [&count, &outObjectRefs](uint32, const TLDSObjectRefPtr<T>& value)
        {
            outObjectRefs.Add(value);
            ++count;
        });
        return count;
    }

    template <class T, class TObjectPtr, class TObjectRefPtr>
    uint32 TLDSObjectRefArrayPtr<T, TObjectPtr, TObjectRefPtr>::GetResolvedObjects(
        const ILDSQueryContext& context,
        TArray2<TObjectPtr>& outObjects) const
    {
        uint32 count = 0;
        TLDSObjectRefArrayPtr::ForEachResolvedItem(context, [&count, &outObjects](uint32, const TLDSObjectPtr<T>& value)
        {
            outObjects.Add(value);
            ++count;
        });
        return count;
    }
}
