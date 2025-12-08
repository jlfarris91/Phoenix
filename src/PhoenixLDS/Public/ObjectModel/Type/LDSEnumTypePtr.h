
#pragma once

#include "ObjectModel/LDSArrayPtr.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct LDSEnumTypeItemPtr : LDSObjectPtr
    {
        LDSEnumTypeItemPtr() = default;
        LDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypeItemPtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> Key;
        LDSValuePtr Value;

    private:
        void InitCommon();
    };

    template <class T, class TValuePtr = TLDSValuePtr<T>>
    struct TLDSEnumTypeItemPtr : LDSObjectPtr
    {
        TLDSEnumTypeItemPtr() = default;
        TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypeItemPtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> Key;
        TValuePtr Value;

    private:
        void InitCommon();
    };

    struct LDSEnumTypePtr : LDSObjectPtr
    {
        LDSEnumTypePtr() = default;
        LDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        TLDSObjectArrayPtr<LDSEnumTypeItemPtr> Items;
        LDSValuePtr DefaultValue;

    private:
        void InitCommon();
    };

    template <class T, class TValuePtr = TLDSValuePtr<T>>
    struct TLDSEnumTypePtr : LDSObjectPtr
    {
        TLDSEnumTypePtr() = default;
        TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypePtr(const LDSRecordPtr& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        TLDSObjectArrayPtr<TLDSEnumTypeItemPtr<T, TValuePtr>> Items;
        TValuePtr DefaultValue;

    private:
        void InitCommon();
    };
}
