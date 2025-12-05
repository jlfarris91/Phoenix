
#pragma once

#include "LDSArrayPtr.h"
#include "LDSObjectPtr.h"
#include "LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct LDSEnumItemPtr : LDSObjectPtr
    {
        LDSEnumItemPtr() = default;
        LDSEnumItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumItemPtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> Key;
        LDSValuePtr Value;

    private:
        void InitCommon();
    };

    struct LDSEnumPtr : LDSObjectPtr
    {
        LDSEnumPtr() = default;
        LDSEnumPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumPtr(const LDSRecordPtr& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        TLDSObjectArrayPtr<LDSEnumItemPtr> Items;

    private:
        void InitCommon();
    };
}
