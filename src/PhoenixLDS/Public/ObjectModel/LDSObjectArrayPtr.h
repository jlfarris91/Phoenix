
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectArrayPtr : LDSArrayPtrBase
    {
        LDSObjectArrayPtr() = default;
        LDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectArrayPtr(const LDSRecordPtr& other);

        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const LDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectPtr, class TContainer = TArray2<TObjectPtr>>
        uint32 GetObjects(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <class TObjectPtr, class TContainer = TArray2<TObjectPtr>>
        uint32 GetObjects(const ILDSQueryContext& context, TContainer& outObjects) const;

        uint32 ReadObjects(const ILDSQueryContext& context, TArray2<T>& outObjects) const;
    };

    template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>>
    struct PHOENIX_LDS_API TLDSObjectArrayPtr : LDSArrayPtrBase
    {
        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSRecordPtr& other);

        TObjectPtr Item(uint32 index) const;

        TObject ItemObject(const ILDSQueryContext& context, uint32 index, const TObject& defaultObject = {}) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItemObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer = TArray2<TObjectPtr>>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer = TArray2<TObject>>
        uint32 GetItemObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };
}
