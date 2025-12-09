#pragma once

#include "LDSObjectRefPtr.h"

namespace Phoenix::LDS
{
    template <class TObjectPtr>
    TObjectPtr LDSObjectRefPtr::ResolveObject(const ILDSQueryContext& context) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        TObjectPtr objectPtr;
        (void)TryResolveObject<TObjectPtr>(context, objectPtr);
        return objectPtr;
    }

    template <class TObject, class TObjectPtr>
    TObjectPtr LDSObjectRefPtr::ResolveObject(const ILDSQueryContext& context) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return ResolveObject<TObjectPtr>(context);
    }

    template <class TObjectPtr>
    bool LDSObjectRefPtr::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        if (const LDSRecord* record = context.QueryRecord(Path, Flags))
        {
            FName objectId = record->GetValueAs<FName>();
            outObjectPtr = TObjectPtr(LDSRecordPath(objectId), Flags);
            return true;
        }
        return false;
    }

    template <class TObject, class TObjectPtr>
    bool LDSObjectRefPtr::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return TryResolveObject<TObjectPtr>(context, outObjectPtr);
    }

    template <class TObjectPtr>
    TLDSObjectRefPtr<TObjectPtr>::TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectRefPtrBase(path, flags)
    {
    }

    template <class TObjectPtr>
    TLDSObjectRefPtr<TObjectPtr>::TLDSObjectRefPtr(const LDSObjectRefPtrBase& other)
        : LDSObjectRefPtrBase(other)
    {
    }

    template <class TObjectPtr>
    TLDSObjectRefPtr<TObjectPtr>::operator LDSObjectRefPtr() const
    {
        return LDSObjectRefPtr(Path, Flags);
    }

    template <class TObjectPtr>
    TObjectPtr TLDSObjectRefPtr<TObjectPtr>::ResolveObject(const ILDSQueryContext& context) const
    {
        TObjectPtr objectPtr;
        (void)TryResolveObject(context, objectPtr);
        return objectPtr;
    }

    template <class TObjectPtr>
    bool TLDSObjectRefPtr<TObjectPtr>::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const
    {
        if (const LDSRecord* record = context.QueryRecord(Path, Flags))
        {
            FName objectId = record->GetValueAs<FName>();
            outObjectPtr = TObjectPtr(LDSRecordPath(objectId), Flags);
            return true;
        }
        return false;
    }
}
