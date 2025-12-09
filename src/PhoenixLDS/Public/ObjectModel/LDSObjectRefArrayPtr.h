
#pragma once

#include "LDSArrayPtr.h"
#include "LDSObjectPtr.h"
#include "LDSObjectRefPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectRefArrayPtrBase : LDSArrayPtrBase
    {
        LDSObjectRefArrayPtrBase() = default;
        LDSObjectRefArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        LDSObjectRefArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectRefArrayPtr : LDSObjectRefArrayPtrBase
    {
        LDSObjectRefArrayPtr() = default;
        LDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr>
        TObjectRefPtr Item(uint32 index) const;

        template <IsNotRecordPtr TObject, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObject>>
        TObjectRefPtr Item(uint32 index) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TContainer>
        uint32 GetResolvedObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
    struct TLDSObjectRefArrayPtrBase : LDSObjectRefArrayPtrBase
    {
        using ObjectPtrT = TObjectPtr;
        using ObjectRefPtrT = TObjectRefPtr;

        TLDSObjectRefArrayPtrBase() = default;
        TLDSObjectRefArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtrBase(const LDSObjectRefArrayPtrBase& other);

        operator LDSObjectRefArrayPtr() const;

        TObjectRefPtr Item(uint32 index) const;

        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtrBase& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtrBase& ForEachResolvedItemObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer>
        uint32 GetResolvedObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <IsObjectRefPtr TObjectRefPtr>
    struct TLDSObjectRefArrayPtr<TObjectRefPtr, EnableIfObjectRefPtr<TObjectRefPtr>> : TLDSObjectRefArrayPtrBase<typename TObjectRefPtr::ObjectPtrT, TObjectRefPtr>
    {
        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);
    };

    template <IsObjectPtr TObjectPtr>
    struct TLDSObjectRefArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>> : TLDSObjectRefArrayPtrBase<TObjectPtr, TLDSObjectRefPtr<TObjectPtr>>
    {
        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);
    };
}

#include "LDSObjectRefArrayPtr.inl"