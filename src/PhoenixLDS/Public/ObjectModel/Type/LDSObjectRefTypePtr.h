
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSObjectRefPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <IsObjectRefPtr TObjectRefPtr>
    struct TLDSObjectRefTypePtrBase : LDSObjectPtr
    {
        TLDSObjectRefTypePtrBase() = default;
        TLDSObjectRefTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefTypePtrBase(const LDSRecordPtr& other);

        TLDSValuePtr<FName> ReferenceType;
        TObjectRefPtr Default;

    private:
        void InitCommon();
    };

    using LDSObjectRefTypePtr = TLDSObjectRefTypePtrBase<LDSObjectRefPtr>;

    template <IsObjectRefPtr TObjectRefPtr>
    using TLDSObjectRefTypePtr = TLDSObjectRefTypePtrBase<TObjectRefPtr>;
}
