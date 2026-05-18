
#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectPtrBase;

    struct PHOENIX_SIM_API LDSObjectRefPtrBase : LDSRecordPtr
    {
        LDSObjectRefPtrBase() = default;
        LDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefPtrBase(const LDSRecordPtr& other);

        FName GetReferenceId(const ILDSQueryContext& context) const;
    };
    
    struct PHOENIX_SIM_API LDSObjectRefPtr : LDSObjectRefPtrBase
    {
        LDSObjectRefPtr() = default;
        LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefPtr(const LDSObjectRefPtrBase& other);

        template <IsObjectPtr TObjectPtr = LDSObjectPtr>
        TObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>>
        TObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr>
        bool TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>>
        bool TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const;
    };

    template <IsObjectPtr TObjectPtr>
    struct TLDSObjectRefPtrBase : LDSObjectRefPtrBase
    {
        using ObjectT = typename TObjectPtr::ObjectT;
        using ObjectPtrT = TObjectPtr;

        TLDSObjectRefPtrBase() = default;
        TLDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefPtrBase(const LDSObjectRefPtrBase& other);

        operator LDSObjectRefPtr() const;

        TObjectPtr ResolveObject(const ILDSQueryContext& context) const;

        bool TryResolveObject(const ILDSQueryContext& context, TObjectPtr& outObjectPtr) const;
    };

    template <IsObjectPtr TObjectPtr>
    struct TLDSObjectRefPtr<TObjectPtr, EnableIfObjectPtr<TObjectPtr>> : TLDSObjectRefPtrBase<TObjectPtr>
    {
        TLDSObjectRefPtr() = default;
        TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefPtr(const LDSObjectRefPtrBase& other);
    };

    template <IsNotRecordPtr TObject>
    struct TLDSObjectRefPtr<TObject, EnableIfNotRecordPtr<TObject>> : TLDSObjectRefPtrBase<TLDSObjectPtr<TObject>>
    {
        TLDSObjectRefPtr() = default;
        TLDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefPtr(const LDSObjectRefPtrBase& other);
    };
}

#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectRefPtr.inl"
