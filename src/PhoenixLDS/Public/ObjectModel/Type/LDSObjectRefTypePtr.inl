#pragma once

#include "LDSObjectRefTypePtr.h"

namespace Phoenix::LDS
{
    template <class TObjectPtr, class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefTypePtr<TObjectPtr, TObjectRefPtr>::TLDSObjectRefTypePtr(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefTypePtr<TObjectPtr, TObjectRefPtr>::TLDSObjectRefTypePtr(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class TObjectPtr, class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    void TLDSObjectRefTypePtr<TObjectPtr, TObjectRefPtr>::InitCommon()
    {
        ReferenceType = Value<FName>("type");
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefTypePtr<TObjectRefPtr>::TLDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : TLDSObjectRefTypePtr(path, flags)
    {
    }

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    TLDSObjectRefTypePtr<TObjectRefPtr>::TLDSObjectRefTypePtr(const LDSRecordPtr& other)
        : TLDSObjectRefTypePtr(other)
    {
    }
}
