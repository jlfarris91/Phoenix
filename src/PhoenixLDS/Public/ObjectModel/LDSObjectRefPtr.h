
#pragma once

#include "DLLExport.h"
#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectPtrBase;

    struct PHOENIX_LDS_API LDSObjectRefPtrBase : LDSRecordPtr
    {
        LDSObjectRefPtrBase() = default;
        LDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectRefPtr : LDSObjectRefPtrBase
    {
        LDSObjectRefPtr() = default;
        LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefPtr(const LDSObjectRefPtrBase& other);

        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr ResolveObject(const ILDSQueryContext& context) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>>
        TObjectPtr ResolveObject(const ILDSQueryContext& context) const
            requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectPtr = LDSObjectPtr>
        bool TryResolveObject(
            const ILDSQueryContext& context,
            TObjectPtr& outObjectPtr) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>>
        bool TryResolveObject(
            const ILDSQueryContext& context,
            TObjectPtr& outObjectPtr) const
            requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);
    };

    template <class TObjectPtr>
    struct PHOENIX_LDS_API TLDSObjectRefPtr : LDSObjectRefPtrBase
    {
        using ObjectPtrT = TObjectPtr;

        TLDSObjectRefPtr() = default;
        TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefPtr(const LDSObjectRefPtrBase& other);

        operator LDSObjectRefPtr() const;

        TObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        bool TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const;
    };
}
