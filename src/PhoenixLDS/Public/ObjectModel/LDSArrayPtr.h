
#pragma once

#include "LDSRecordPtr.h"
#include "Containers/Array.h"

namespace Phoenix::LDS
{
    class LDSRecord;

    struct PHOENIX_LDS_API LDSArrayPtrBase : LDSRecordPtr
    {
        LDSArrayPtrBase() = default;
        LDSArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtrBase(const LDSRecordPtr& other);

        uint32 GetSize(const ILDSQueryContext& context) const;

        const LDSRecord* GetItemRecord(const ILDSQueryContext& context, uint32 index) const;

        // IE array.Item<LDSObjectPtr>()
        template <class TItemPtr = LDSRecordPtr>
        TItemPtr Item(uint32 index) const;
    };

    struct PHOENIX_LDS_API LDSArrayPtr : LDSArrayPtrBase
    {
        LDSArrayPtr() = default;
        LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtr(const LDSRecordPtr& other);

        // Ie array.ItemAsValue<uint32>()
        template <class T>
        TLDSValuePtr<T> ItemAsValue(uint32 index) const;

        // Ie array.ItemValueAs<uint32>()
        template <class T>
        T ItemValueAs(const ILDSQueryContext& context, uint32 index, const T& defaultValue = {}) const;

        // IE array.ItemAsObject<Foobar>()
        template <class T>
        TLDSObjectPtr<T> ItemAsObject(uint32 index) const;

        // IE array.ItemAsObjectRef<Foobar>()
        template <class T>
        TLDSObjectRefPtr<T> ItemAsObjectRef(uint32 index) const;

        // ie array.ResolveItemObject<FoobarPtr>()
        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr ResolveItemObject(
            const ILDSQueryContext& context,
            uint32 index) const
            requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>);

        // ie array.ResolveItemObject<Foobar>()
        template <class T, class TObjectPtr = TLDSObjectPtr<T>>
        TObjectPtr ResolveItemObject(
            const ILDSQueryContext& context,
            uint32 index) const
            requires (!std::is_base_of_v<LDSObjectPtr, T>);

        template <class TItemPtr = LDSRecordPtr, class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TValuePtr = LDSValuePtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( std::is_base_of_v<LDSValuePtr, TValuePtr> );

        template <class T, class TValuePtr = TLDSValuePtr<T>, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( !std::is_base_of_v<LDSValuePtr, T> );

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( std::is_base_of_v<LDSObjectPtr, TObjectPtr> );

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires ( !std::is_base_of_v<LDSObjectPtr, T> );

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectRefPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectRefPtr, TObjectRefPtr>);

        template <class T, class TObjectRefPtr = TLDSObjectRefPtr<T>, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (!std::is_base_of_v<LDSObjectRefPtr, T>);

        template <class TObjectPtr, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>);

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (!std::is_base_of_v<LDSObjectPtr, T>);
    };

    template <class T, class TObjectPtr>
    struct PHOENIX_LDS_API TLDSObjectArrayPtr : LDSArrayPtrBase
    {
        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSRecordPtr& other);

        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class U>
        uint32 GetObjects(const ILDSQueryContext& context, TArray2<U>& outObjects) const;

        uint32 ReadObjects(const ILDSQueryContext& context, TArray2<T>& outObjects) const;
    };

    template <class T, class TObjectPtr, class TObjectRefPtr>
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr : LDSArrayPtrBase
    {
        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSRecordPtr& other);

        TObjectRefPtr Item(uint32 index) const;

        TObjectPtr ResolvedItem(const ILDSQueryContext& context, uint32 index) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachResolvedItem(const ILDSQueryContext& context, const TCallback& callback) const;

        uint32 GetObjectRefs(const ILDSQueryContext& context, TArray2<TObjectRefPtr>& outObjectRefs) const;

        uint32 GetResolvedObjects(const ILDSQueryContext& context, TArray2<TObjectPtr>& outObjects) const;
    };

}
