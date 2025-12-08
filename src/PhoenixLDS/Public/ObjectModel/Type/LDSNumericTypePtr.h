
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct LDSNumericTypePtr : LDSObjectPtr
    {
        LDSNumericTypePtr() = default;
        LDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSNumericTypePtr(const LDSRecordPtr& other);

        LDSValuePtr DefaultValue;
        LDSValuePtr MinValue;
        LDSValuePtr MaxValue;

    private:
        void InitCommon();
    };

    template <class T, class TValuePtr = TLDSValuePtr<T>>
    struct TLDSNumericTypePtr : LDSObjectPtr
    {
        TLDSNumericTypePtr() = default;
        TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSNumericTypePtr(const LDSRecordPtr& other);

        TValuePtr DefaultValue;
        TValuePtr MinValue;
        TValuePtr MaxValue;

    private:
        void InitCommon();
    };
}
