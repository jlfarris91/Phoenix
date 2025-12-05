
#pragma once

#include "LDSRecordPtr.h"
#include "Containers/Array.h"

namespace Phoenix::LDS
{
    class LDSRecord;

    struct PHOENIX_LDS_API LDSArrayPtr : LDSRecordPtr
    {
        LDSArrayPtr() = default;
        LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSArrayPtr(const LDSRecordPtr& other);

        uint32 GetSize(const ILDSQueryContext& context) const;

        const LDSRecord* GetItemRecord(const ILDSQueryContext& context, uint32 index) const;

        LDSRecordPtr Item(uint32 index) const;

        // ie array.ItemAsValue<MyCustomValuePtr>().GetValue() == "Foobar"_n
        template <class TValuePtr = LDSValuePtr>
        TValuePtr ItemAsValue(uint32 index) const requires ( std::is_base_of_v<LDSValuePtr, TValuePtr> );

        // ie array.ItemAsValue<uint32>().GetValue() == 123
        template <class T, class TObjectPtr = TLDSObjectPtr<T>>
        TObjectPtr ItemAsValue(uint32 index) const requires ( !std::is_base_of_v<LDSValuePtr, T> );

        template <class T>
        T ItemValueAs(const ILDSQueryContext& context, uint32 index, const T& defaultValue = {}) const;

        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr ItemAsObject(uint32 index) const requires ( std::is_base_of_v<LDSObjectPtr, TObjectPtr> );

        template <class T, class TObjectPtr = TLDSObjectPtr<T>>
        TObjectPtr ItemAsObject(uint32 index) const requires ( !std::is_base_of_v<LDSObjectPtr, T> );

        // ie array.ItemAsObjectRef<FoobarPtr>()
        template <class TObjectRefPtr = LDSObjectRefPtr>
        TObjectRefPtr ItemAsObjectRef(uint32 index) const requires ( std::is_base_of_v<LDSObjectRefPtr, TObjectRefPtr> );

        // ie array.ItemAsObjectRef<Foobar>()
        template
        <
            class T,
            class TObjectPtr = TLDSObjectPtr<T>,
            class TObjectRefPtr = TLDSObjectRefPtr<T, TObjectPtr>
        >
        TObjectRefPtr ItemAsObjectRef(uint32 index) const requires ( !std::is_base_of_v<LDSObjectRefPtr, T> );

        // ie array.ItemAsResolvedObject<FoobarPtr>()
        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const requires ( std::is_base_of_v<LDSObjectPtr, TObjectPtr> );

        // ie array.ItemAsResolvedObject<Foobar>()
        template <class T, class TObjectPtr = TLDSObjectPtr<T>>
        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const requires ( !std::is_base_of_v<LDSObjectPtr, T> );

        template <class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsObjectRef(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class T, class TCallback>
        const LDSArrayPtr& ForEachItemAsResolvedObject(const ILDSQueryContext& context, const TCallback& callback) const;
    };

    template <class T, class TValuePtr>
    struct PHOENIX_LDS_API TLDSValueArrayPtr : LDSArrayPtr
    {
        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSRecordPtr& other);

        TValuePtr Item(uint32 index) const;

        T ItemValue(const ILDSQueryContext& context, uint32 index, const T& defaultValue = {}) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItemValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class U>
        uint32 GetItems(const ILDSQueryContext& context, TArray2<U>& outItems) const;

        template <class U>
        uint32 GetItemValues(const ILDSQueryContext& context, TArray2<U>& outValues) const;
    };

    template <class T, class TObjectPtr>
    struct PHOENIX_LDS_API TLDSObjectArrayPtr : LDSArrayPtr
    {
        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSRecordPtr& other);

        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class U>
        uint32 GetObjects(const ILDSQueryContext& context, TArray2<U>& outObjects) const;

        uint32 ReadObjects(const ILDSQueryContext& context, TArray2<T>& outObjects) const;
    };

    template <class T, class TObjectPtr, class TObjectRefPtr>
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr : LDSArrayPtr
    {
        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSRecordPtr& other);

        TObjectRefPtr Item(uint32 index) const;

        TObjectPtr ResolvedItem(const ILDSQueryContext& context, uint32 index) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const LDSArrayPtr& ForEachResolvedItem(const ILDSQueryContext& context, const TCallback& callback) const;

        uint32 GetObjectRefs(const ILDSQueryContext& context, TArray2<TObjectRefPtr>& outObjectRefs) const;

        uint32 GetResolvedObjects(const ILDSQueryContext& context, TArray2<TObjectPtr>& outObjects) const;
    };

}
