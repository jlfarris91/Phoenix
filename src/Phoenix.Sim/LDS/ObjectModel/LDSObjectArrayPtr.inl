
#pragma once

#include "Phoenix.Sim/LDS/ObjectModel/LDSArrayUtil.h"
#include "Phoenix.Sim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <IsObjectPtr TObjectPtr>
    TObjectPtr LDSObjectArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TObjectPtr LDSObjectArrayPtr::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <IsObjectPtr TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
    const LDSObjectArrayPtr& LDSObjectArrayPtr::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsReadObject<TObject, TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsObjectPtr TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::GetItems(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::GetItems(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TContainer>
    uint32 LDSObjectArrayPtr::ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsReadObjects<TObject, TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TLDSObjectArrayPtrBase<TObject, TObjectPtr>::TLDSObjectArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectArrayPtrBase(path, flags)
    {
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TLDSObjectArrayPtrBase<TObject, TObjectPtr>::TLDSObjectArrayPtrBase(const LDSObjectArrayPtrBase& other)
        : LDSObjectArrayPtrBase(other)
    {
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TLDSObjectArrayPtrBase<TObject, TObjectPtr>::operator LDSObjectArrayPtr() const
    {
        return LDSObjectArrayPtr(Path, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TObjectPtr TLDSObjectArrayPtrBase<TObject, TObjectPtr>::Item(uint32 index) const
    {
        return LDSArrayUtil::GetItem<TObjectPtr>(Path, index, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    template <class TCallback>
    const TLDSObjectArrayPtrBase<TObject, TObjectPtr>& TLDSObjectArrayPtrBase<TObject, TObjectPtr>::ForEachItem(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItem<TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    template <class TCallback>
    const TLDSObjectArrayPtrBase<TObject, TObjectPtr>& TLDSObjectArrayPtrBase<TObject, TObjectPtr>::ForEachItemAsReadObject(
        const ILDSQueryContext& context,
        const TCallback& callback) const
    {
        LDSArrayUtil::ForEachItemAsReadObject<TObject, TObjectPtr>(context, Path, callback, Flags);
        return *this;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    template <class TContainer>
    uint32 TLDSObjectArrayPtrBase<TObject, TObjectPtr>::GetItems(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItems<TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    template <class TContainer>
    uint32 TLDSObjectArrayPtrBase<TObject, TObjectPtr>::ReadObjects(
        const ILDSQueryContext& context,
        TContainer& outObjects) const
    {
        return LDSArrayUtil::GetItemsAsReadObjects<TObject, TObjectPtr>(context, Path, outObjects, Flags);
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSObjectArrayPtrBase<typename TObjectPtr::ObjectT, TObjectPtr>(path, flags)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectArrayPtr(
        const LDSObjectArrayPtrBase& other)
        : TLDSObjectArrayPtrBase<typename TObjectPtr::ObjectT, TObjectPtr>(other)
    {
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectArrayPtr<TObject, EnableIfNotRecordPtr<TObject>>::TLDSObjectArrayPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSObjectArrayPtrBase<TObject, TLDSObjectPtr<TObject>>(path, flags)
    {
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectArrayPtr<TObject, EnableIfNotRecordPtr<TObject>>::TLDSObjectArrayPtr(
        const LDSObjectArrayPtrBase& other)
        : TLDSObjectArrayPtrBase<TObject, TLDSObjectPtr<TObject>>(other)
    {
    }
}
