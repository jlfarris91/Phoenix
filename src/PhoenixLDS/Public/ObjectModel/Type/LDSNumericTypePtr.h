
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <IsValuePtr TValuePtr>
    struct TLDSNumericTypePtrBase : LDSObjectPtr
    {
        TLDSNumericTypePtrBase() = default;
        TLDSNumericTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSNumericTypePtrBase(const LDSRecordPtr& other);

        TValuePtr DefaultValue;
        TValuePtr MinValue;
        TValuePtr MaxValue;

    private:
        void InitCommon();
    };

    using LDSNumericTypePtr = TLDSNumericTypePtrBase<LDSValuePtr>;

    template <IsValuePtr TValuePtr>
    using TLDSNumericTypePtr = TLDSNumericTypePtrBase<TValuePtr>;
}

#include "LDSNumericTypePtr.inl"