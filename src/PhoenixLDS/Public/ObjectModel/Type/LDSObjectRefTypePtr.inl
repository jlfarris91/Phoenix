#pragma once

#include "LDSObjectRefTypePtr.h"

namespace Phoenix::LDS
{
    template <class T, class TObjectRefPtr>
    TLDSObjectRefTypePtr<T, TObjectRefPtr>::TLDSObjectRefTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class T, class TObjectRefPtr>
    TLDSObjectRefTypePtr<T, TObjectRefPtr>::TLDSObjectRefTypePtr(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class T, class TObjectRefPtr>
    void TLDSObjectRefTypePtr<T, TObjectRefPtr>::InitCommon()
    {
        ReferenceType = Value<ELDSValueType>("type");
    }
}
