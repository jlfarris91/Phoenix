
#pragma once

#include "LDSArrayPtr.h"
#include "LDSObjectPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectArrayPtrBase : LDSArrayPtrBase
    {
        LDSObjectArrayPtrBase() = default;
        LDSObjectArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectArrayPtr : LDSObjectArrayPtrBase
    {
        LDSObjectArrayPtr() = default;
        LDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);

        template <IsObjectPtr TObjectPtr = LDSObjectPtr>
        TObjectPtr Item(uint32 index) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>>
        TObjectPtr Item(uint32 index) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, class TCallback>
        const LDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSObjectArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TContainer>
        uint32 ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    struct TLDSObjectArrayPtrBase : LDSObjectArrayPtrBase
    {
        using ObjectT = TObject;
        using ObjectPtrT = TObjectPtr;

        TLDSObjectArrayPtrBase() = default;
        TLDSObjectArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtrBase(const LDSObjectArrayPtrBase& other);

        operator LDSObjectArrayPtr() const;

        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const TLDSObjectArrayPtrBase& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectArrayPtrBase& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <class TContainer>
        uint32 ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <IsObjectPtr TObjectPtr>
    struct TLDSObjectArrayPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>> : TLDSObjectArrayPtrBase<typename TObjectPtr::ObjectT, TObjectPtr>
    {
        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);
    };

    template <IsNotRecordPtr TObject>
    struct TLDSObjectArrayPtr<TObject, EnableIfNotRecordPtr<TObject>> : TLDSObjectArrayPtrBase<TObject, TLDSObjectPtr<TObject>>
    {
        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);
    };
}

#include "LDSObjectArrayPtr.inl"