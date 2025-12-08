
#pragma once

#include "DLLExport.h"
#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectRefPtr : LDSRecordPtr
    {
        LDSObjectRefPtr() = default;
        LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefPtr(const LDSRecordPtr& other);

        template <class T>
        TLDSObjectPtr<T> As() const
        {
            return TLDSObjectPtr<T>(Path, Flags);
        }

        LDSObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        bool TryResolveObject(const ILDSQueryContext& context, LDSObjectPtr& outObjectPtr) const;

        template <class T>
        TLDSObjectPtr<T> ResolveObjectAs(const ILDSQueryContext& context) const
        {
            return ResolveObject(context);
        }

        template <class T>
        bool TryResolveObjectAs(const ILDSQueryContext& context, TLDSObjectPtr<T>& outObjectPtr) const
        {
            return TryResolveObject(context, outObjectPtr);
        }
    };

    template <class T, class TObjectPtr>
    struct TLDSObjectRefPtr : LDSObjectRefPtr
    {
        TLDSObjectRefPtr() = default;
        TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefPtr(const LDSRecordPtr& other);

        TObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        bool TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObject) const;
    };
}
