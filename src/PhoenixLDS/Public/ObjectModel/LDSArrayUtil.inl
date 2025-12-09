
#pragma once

#include "LDSValuePtr.h"

namespace Phoenix::LDS
{
    inline uint32 LDSArrayUtil::GetSize(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
    {
        return context.QueryRecordValueAs<uint32>(path.Append("size"), 0, flags);
    }

    template <IsRecordPtr TItemPtr>
    TItemPtr LDSArrayUtil::GetItem(const LDSRecordPath& path, uint32 index, ELDSRecordQueryFlags flags)
    {
        return TItemPtr(path.Append(index), flags);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    TValue LDSArrayUtil::GetItemValueAs(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        uint32 index,
        const TValue& defaultValue,
        ELDSRecordQueryFlags flags)
    {
        return GetItem<TValuePtr>(path, index, flags).template GetValue<TValue>(context, defaultValue);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectPtr LDSArrayUtil::GetResolvedItemObject(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        uint32 index,
        ELDSRecordQueryFlags flags)
    {
        return GetItem<TObjectRefPtr>(path, index, flags).ResolveObject(context);
    }

    template <IsRecordPtr TItemPtr, class TCallback>
    void LDSArrayUtil::ForEachItem(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        const TCallback& callback,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = GetSize(context, path, flags);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, GetItem<TItemPtr>(path, i, flags));
        }
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TCallback>
    void LDSArrayUtil::ForEachItemValueAs(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        const TCallback& callback,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = GetSize(context, path, flags);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, GetItemValueAs<TValue, TValuePtr>(path, i, flags));
        }
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
    void LDSArrayUtil::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        const TCallback& callback,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = GetSize(context, path, flags);
        for (uint32 i = 0; i < count; ++i)
        {
            TObjectPtr objectPtr = GetItem<TObjectPtr>(path, i, flags);
            TObject object;
            if (objectPtr.TryReadObject(context, object))
            {
                callback(i, object);
            }
        }
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    void LDSArrayUtil::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        const TCallback& callback,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = GetSize(context, path, flags);
        for (uint32 i = 0; i < count; ++i)
        {
            callback(i, GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, path, i, flags));
        }
    }

    template <IsRecordPtr TItemPtr, class TContainer>
    uint32 LDSArrayUtil::GetItems(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        TContainer& outItems,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = 0;
        LDSArrayUtil::ForEachItem<TItemPtr>(context, path, [&count, &outItems](uint32, const TItemPtr& value)
        {
            outItems.Add(value);
            ++count;
        }, flags);
        return count;
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TContainer>
    uint32 LDSArrayUtil::GetItemValuesAs(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        TContainer& outValues,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = 0;
        LDSArrayUtil::ForEachItemValueAs<TValue, TValuePtr>(context, path, [&count, &outValues](uint32, const TValue& value)
        {
            outValues.Add(value);
            ++count;
        }, flags);
        return count;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TContainer>
    uint32 LDSArrayUtil::GetItemsAsResolvedObjects(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        TContainer& outObjects,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = 0;
        ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, path, [&count, &outObjects](uint32, const TObjectPtr& value)
        {
            outObjects.Add(value);
            ++count;
        }, flags);
        return count;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TContainer>
    uint32 LDSArrayUtil::GetItemsAsReadObjects(
        const ILDSQueryContext& context,
        const LDSRecordPath& path,
        TContainer& outObjects,
        ELDSRecordQueryFlags flags)
    {
        uint32 count = 0;
        ForEachItemAsReadObject<TObject, TObjectPtr>(context, path, [&count, &outObjects](uint32, const TObject& object)
        {
            outObjects.Add(object);
            ++count;
        }, flags);
        return count;
    }
}
