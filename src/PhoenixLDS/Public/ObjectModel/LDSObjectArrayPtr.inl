
#pragma once

#include "LDSArrayUtil.h"
#include "LDSObjectArrayPtr.h"
#include "LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class TObjectPtr>
    TObjectPtr LDSObjectArrayPtr::Item(uint32 index) const requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <class TObject, class TObjectPtr>
    TObjectPtr LDSObjectArrayPtr::Item(uint32 index) const requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>)
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <class TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObject, class TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>)
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObject, class TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsReadObject<TObject, TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObject, class TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::GetItems(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>)
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObject, class TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsReadObjects<TObject, TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObjectPtr>::TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectArrayPtrBase(path, flags)
    {
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObjectPtr>::TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other)
        : LDSObjectArrayPtrBase(other)
    {
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObjectPtr>::operator LDSObjectArrayPtr() const
    {
        return LDSObjectArrayPtr(Path, Flags);
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TObjectPtr TLDSObjectArrayPtr<TObjectPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TCallback>
    const TLDSObjectArrayPtr<TObjectPtr>& TLDSObjectArrayPtr<TObjectPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TCallback>
    requires (TObjectPtr::ObjectT)
    const TLDSObjectArrayPtr<TObjectPtr>& TLDSObjectArrayPtr<TObjectPtr>::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsReadObject<typename TObjectPtr::ObjectT, TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TContainer>
    uint32 TLDSObjectArrayPtr<TObjectPtr>::GetItems(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TContainer>
    requires (TObjectPtr::ObjectT)
    uint32 TLDSObjectArrayPtr<TObjectPtr>::ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsReadObjects<typename TObjectPtr::ObjectT, TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObject, TObjectPtr>::TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectArrayPtrBase(path, flags)
    {
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObject, TObjectPtr>::TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other)
        : LDSObjectArrayPtrBase(other)
    {
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TLDSObjectArrayPtr<TObject, TObjectPtr>::operator LDSObjectArrayPtr() const
    {
        return LDSObjectArrayPtr(Path, Flags);
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    TObjectPtr TLDSObjectArrayPtr<TObject, TObjectPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TCallback>
    const TLDSObjectArrayPtr<TObject, TObjectPtr>& TLDSObjectArrayPtr<TObject, TObjectPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TCallback>
    const TLDSObjectArrayPtr<TObject, TObjectPtr>& TLDSObjectArrayPtr<TObject, TObjectPtr>::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsReadObject<TObject, TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TContainer>
    uint32 TLDSObjectArrayPtr<TObject, TObjectPtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    template <class TContainer>
    uint32 TLDSObjectArrayPtr<TObject, TObjectPtr>::ReadObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsReadObjects<TObject, TObjectPtr>(context, Path, outObjects, Flags);
    }
}
