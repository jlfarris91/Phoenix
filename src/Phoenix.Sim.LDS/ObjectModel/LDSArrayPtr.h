
#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSArrayPtrBase : LDSRecordPtr
    {
        LDSArrayPtrBase() = default;
        LDSArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtrBase(const LDSRecordPtr& other);

        uint32 GetSize(const ILDSQueryContext& context) const;

        const LDSRecord* GetItemRecord(const ILDSQueryContext& context, uint32 index) const;
    };

    struct PHOENIX_SIM_API LDSArrayPtr : LDSArrayPtrBase
    {
        LDSArrayPtr() = default;
        LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtr(const LDSRecordPtr& other);

        template <IsRecordPtr TItemPtr = LDSRecordPtr>
        TItemPtr Item(uint32 index) const;

        template <IsValuePtr TValuePtr = LDSValuePtr>
        TValuePtr ItemAsValue(uint32 index) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>>
        TValue ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr>
        TObjectPtr ItemAsObject(uint32 index) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>>
        TObjectPtr ItemAsObject(uint32 index) const;

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <IsRecordPtr TItemPtr = LDSRecordPtr, class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsValuePtr TValuePtr = LDSValuePtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const;
    };
}

#include "Phoenix.Sim.LDS/ObjectModel/LDSArrayPtr.inl"