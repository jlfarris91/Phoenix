#pragma once

#include "LDSObjectRefPtr.h"

namespace Phoenix::LDS
{
    template <class T, class TObjectPtr>
    TLDSObjectRefPtr<T, TObjectPtr>::TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectRefPtr(path, flags)
    {
    }

    template <class T, class TObjectPtr>
    TLDSObjectRefPtr<T, TObjectPtr>::TLDSObjectRefPtr(const LDSRecordPtr& other)
        : LDSObjectRefPtr(other)
    {
    }

    template <class T, class TObjectPtr>
    TObjectPtr TLDSObjectRefPtr<T, TObjectPtr>::ResolveObject(const ILDSQueryContext& context) const
    {
        return LDSObjectRefPtr::ResolveObjectAs<T>(context);
    }

    template <class T, class TObjectPtr>
    bool TLDSObjectRefPtr<T, TObjectPtr>::TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObject) const
    {
        return LDSObjectRefPtr::TryResolveObjectAs<T>(context, outObject);
    }
}
