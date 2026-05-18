#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectRefPtr.h"

namespace Phoenix::LDS
{
    template <IsObjectPtr TObjectPtr>
    TObjectPtr LDSObjectRefPtr::ResolveObject(const ILDSQueryContext& context) const
    {
        TObjectPtr objectPtr;
        (void)TryResolveObject<TObjectPtr>(context, objectPtr);
        return objectPtr;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    TObjectPtr LDSObjectRefPtr::ResolveObject(const ILDSQueryContext& context) const
    {
        return ResolveObject<TObjectPtr>(context);
    }

    template <IsObjectPtr TObjectPtr>
    bool LDSObjectRefPtr::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const
    {
        if (const LDSRecord* record = context.QueryRecord(Path, Flags))
        {
            FName objectId = record->GetValueAs<FName>();
            outObjectPtr = TObjectPtr(LDSRecordPath(objectId), Flags);
            return true;
        }
        return false;
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr>
    bool LDSObjectRefPtr::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const
    {
        return TryResolveObject<TObjectPtr>(context, outObjectPtr);
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefPtrBase<TObjectPtr>::TLDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectRefPtrBase(path, flags)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefPtrBase<TObjectPtr>::TLDSObjectRefPtrBase(const LDSObjectRefPtrBase& other)
        : LDSObjectRefPtrBase(other)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefPtrBase<TObjectPtr>::operator LDSObjectRefPtr() const
    {
        return LDSObjectRefPtr(Path, Flags);
    }

    template <IsObjectPtr TObjectPtr>
    TObjectPtr TLDSObjectRefPtrBase<TObjectPtr>::ResolveObject(const ILDSQueryContext& context) const
    {
        TObjectPtr objectPtr;
        (void)TryResolveObject(context, objectPtr);
        return objectPtr;
    }

    template <IsObjectPtr TObjectPtr>
    bool TLDSObjectRefPtrBase<TObjectPtr>::TryResolveObject(
        const ILDSQueryContext& context,
        TObjectPtr& outObjectPtr) const
    {
        if (const LDSRecord* record = context.QueryRecord(Path, Flags))
        {
            FName objectId = record->GetValueAs<FName>();
            outObjectPtr = TObjectPtr(LDSRecordPath(objectId), Flags);
            return true;
        }
        return false;
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectRefPtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : TLDSObjectRefPtrBase<TObjectPtr>(path, flags)
    {
    }

    template <IsObjectPtr TObjectPtr>
    TLDSObjectRefPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>>::TLDSObjectRefPtr(
        const LDSObjectRefPtrBase& other)
        : TLDSObjectRefPtrBase<TObjectPtr>(other)
    {
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectRefPtr<TObject, EnableIfNotRecordPtr<TObject>>::TLDSObjectRefPtr(
        const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : TLDSObjectRefPtrBase<TLDSObjectPtr<TObject>>(path, flags)
    {
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectRefPtr<TObject, EnableIfNotRecordPtr<TObject>>::TLDSObjectRefPtr(
        const LDSObjectRefPtrBase& other)
        : TLDSObjectRefPtrBase<TLDSObjectPtr<TObject>>(other)
    {
    }
}
