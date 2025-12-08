
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSObjectRefPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectRefTypePtr : LDSObjectPtr
    {
        LDSObjectRefTypePtr() = default;
        LDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> ReferenceType;
        LDSObjectRefPtr Default;

    private:
        void InitCommon();
    };

    template <class T, class TObjectRefPtr = TLDSObjectRefPtr<T>>
    struct TLDSObjectRefTypePtr : LDSObjectPtr
    {
        TLDSObjectRefTypePtr() = default;
        TLDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> ReferenceType;
        TObjectRefPtr Default;

    private:
        void InitCommon();
    };
}
