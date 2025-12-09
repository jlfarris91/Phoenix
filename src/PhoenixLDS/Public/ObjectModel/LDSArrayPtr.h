
#pragma once

#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct LDSValuePtrBase;
    struct LDSObjectPtrBase;
    struct LDSObjectRefPtrBase;
    class LDSRecord;

    struct PHOENIX_LDS_API LDSArrayPtrBase : LDSRecordPtr
    {
        LDSArrayPtrBase() = default;
        LDSArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtrBase(const LDSRecordPtr& other);

        uint32 GetSize(const ILDSQueryContext& context) const;

        const LDSRecord* GetItemRecord(const ILDSQueryContext& context, uint32 index) const;
    };

    struct PHOENIX_LDS_API LDSArrayPtr : LDSArrayPtrBase
    {
        LDSArrayPtr() = default;
        LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtr(const LDSRecordPtr& other);

        template <class TItemPtr = LDSRecordPtr>
        TItemPtr Item(uint32 index) const;

        template <class TValuePtr = LDSValuePtr>
        TValuePtr ItemAsValue(uint32 index) const;

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>>
        TValue ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <class T>
        TLDSObjectPtr<T> ItemAsObject(uint32 index) const;

        template <class TObjectRefPtr = LDSObjectRefPtr>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const
            requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectRef, class TObjectRefPtr = TLDSObjectRefPtr<TObjectRef>>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const
            requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectRef>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <class TItemPtr = LDSRecordPtr, class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TValuePtr = LDSValuePtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (!std::is_base_of_v<LDSValuePtrBase, TValue>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> );

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( !std::is_base_of_v<LDSObjectPtrBase, TObject> );

        template <class TObject, class TCallback>
        const LDSArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectRefPtr = LDSObjectRefPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(
            const ILDSQueryContext& context,
            const TCallback& callback) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);
    };
}
