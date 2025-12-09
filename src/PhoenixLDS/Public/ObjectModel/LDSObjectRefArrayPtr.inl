
#pragma once

#include "LDSArrayUtil.h"
#include "LDSObjectRefArrayPtr.h"

namespace Phoenix::LDS
{
    template <class TObjectRefPtr>
    TObjectRefPtr LDSObjectRefArrayPtr::Item(uint32 index) const requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectRef, class TObjectRefPtr>
    TObjectRefPtr LDSObjectRefArrayPtr::Item(uint32 index) const requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectRef>)
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    TObjectPtr LDSObjectRefArrayPtr::ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <class TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr, class TCallback>
    const LDSObjectRefArrayPtr& LDSObjectRefArrayPtr::ForEachItemAsResolvedObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
        requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr, class TContainer>
    uint32 LDSObjectRefArrayPtr::GetResolvedObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return LDSArrayUtil::GetItemsAsResolvedObjects<TObjectPtr, TObjectRefPtr>(context, Path, outObjects, Flags);
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectRefPtr>::TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectRefArrayPtrBase(path, flags)
    {
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectRefPtr>::TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other)
        : LDSObjectRefArrayPtrBase(other)
    {
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectRefPtr>::operator LDSObjectRefArrayPtr() const
    {
        return LDSObjectRefArrayPtr(Path, Flags);
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TObjectRefPtr TLDSObjectRefArrayPtr<TObjectRefPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    typename TLDSObjectRefArrayPtr<TObjectRefPtr>::ObjectPtrT TLDSObjectRefArrayPtr<TObjectRefPtr>::ItemAsResolvedObject(
        const ILDSQueryContext& context,
        uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<ObjectPtrT, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TCallback>
    const TLDSObjectRefArrayPtr<TObjectRefPtr>& TLDSObjectRefArrayPtr<TObjectRefPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TCallback>
    const TLDSObjectRefArrayPtr<TObjectRefPtr>& TLDSObjectRefArrayPtr<TObjectRefPtr>::ForEachResolvedItemObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<ObjectPtrT, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtr<TObjectRefPtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtr<TObjectRefPtr>::GetResolvedObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsResolvedObjects<ObjectPtrT, TObjectRefPtr>(context, Path, outObjects, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSObjectRefArrayPtrBase(path, flags)
    {
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other)
        : LDSObjectRefArrayPtrBase(other)
    {
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::operator LDSObjectRefArrayPtr() const
    {
        return LDSObjectRefArrayPtr(Path, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TObjectRefPtr TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectRefPtr>(Path, index, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TObjectPtr TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::ItemAsResolvedObject(
        const ILDSQueryContext& context,
        uint32 index) const
    {
        return LDSArrayUtil::GetResolvedItemObject<TObjectPtr, TObjectRefPtr>(context, Path, index, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TCallback>
    const TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>&
        TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TCallback>
    const TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>&
        TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::ForEachResolvedItemObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsResolvedObject<TObjectPtr, TObjectRefPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outItems) const
    {
        return LDSArrayUtil::GetItems<TObjectRefPtr>(context, Path, outItems, Flags);
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    template <class TContainer>
    uint32 TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr>::GetResolvedObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsResolvedObjects<TObjectPtr, TObjectRefPtr>(context, Path, outObjects, Flags);
    }
}
