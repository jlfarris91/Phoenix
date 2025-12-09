
#pragma once

#include "LDSArrayUtil.h"

namespace Phoenix::LDS
{
    template <IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr LDSObjectRefArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr LDSObjectRefArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectPtr LDSObjectRefArrayPtr::ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectRefPtr TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetResolvedObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsResolvedObjects<TObjectPtr, TObjectRefPtr>(context, Path, outObjects, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtrBase(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSObjectRefArrayPtrBase(path, flags)
    {
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtrBase(const LDSObjectRefArrayPtrBase& other)
        : LDSObjectRefArrayPtrBase(other)
    {
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::operator LDSObjectRefArrayPtr() const
    {
        return LDSObjectRefArrayPtr(Path, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectRefPtr TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    TObjectPtr TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::ItemAsResolvedObject(
        const ILDSQueryContext& context,
        uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    template <class TCallback>
    const TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>&
        TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    template <class TCallback>
    const TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>&
        TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::ForEachResolvedItemObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtrBase<TObjectPtr, TObjectRefPtr>::GetResolvedObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsResolvedObjects<TObjectPtr, TObjectRefPtr>(context, Path, outObjects, Flags);
    }

    template <IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefArrayPtr<TObjectRefPtr, EnableIfObjectRefPtr<TObjectRefPtr>>::TLDSObjectRefArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSObjectRefArrayPtrBase<typename TObjectRefPtr::ObjectPtrT, TObjectRefPtr>(path, flags)
    {
    }

    template <IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefArrayPtr<TObjectRefPtr, EnableIfObjectRefPtr<TObjectRefPtr>>::TLDSObjectRefArrayPtr(
        const LDSObjectRefArrayPtrBase& other)
        : TLDSObjectRefArrayPtrBase<typename TObjectRefPtr::ObjectPtrT, TObjectRefPtr>(other)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectRefArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSObjectRefArrayPtrBase<TObjectPtr, TLDSObjectRefPtr<TObjectPtr>>(path, flags)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectRefArrayPtr(
        const LDSObjectRefArrayPtrBase& other)
        : TLDSObjectRefArrayPtrBase<TObjectPtr, TLDSObjectRefPtr<TObjectPtr>>(other)
    {
    }
}
