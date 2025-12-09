
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSObjectRefPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectRefTypePtr : LDSObjectPtr
    {
        LDSObjectRefTypePtr() = default;
        LDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> ReferenceType;
        LDSObjectRefPtr Default;

    private:
        void InitCommon();
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSObjectRefTypePtr;

    template <class TObjectPtr, class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    struct PHOENIX_LDS_API TLDSObjectRefTypePtr<TObjectPtr, TObjectRefPtr> : LDSObjectPtr
    {
        TLDSObjectRefTypePtr() = default;
        TLDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> ReferenceType;
        TObjectRefPtr Default;

    private:
        void InitCommon();
    };

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    struct TLDSObjectRefTypePtr<TObjectRefPtr> : TLDSObjectRefTypePtr<typename TObjectRefPtr::ObjectT, TObjectRefPtr>
    {
        TLDSObjectRefTypePtr() = default;
        TLDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefTypePtr(const LDSRecordPtr& other);
    };
}
