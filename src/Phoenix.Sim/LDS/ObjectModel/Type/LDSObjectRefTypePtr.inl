#pragma once

#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSObjectRefTypePtr.h"

namespace Phoenix::LDS
{
    template <IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefTypePtrBase<TObjectRefPtr>::TLDSObjectRefTypePtrBase(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <IsObjectRefPtr TObjectRefPtr>
    TLDSObjectRefTypePtrBase<TObjectRefPtr>::TLDSObjectRefTypePtrBase(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <IsObjectRefPtr TObjectRefPtr>
    void TLDSObjectRefTypePtrBase<TObjectRefPtr>::InitCommon()
    {
        ReferenceType = Value<FName>("type");
        Default = ObjectRef<TObjectRefPtr>("default");
    }
}