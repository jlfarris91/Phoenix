
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectArrayPtrBase : LDSArrayPtrBase
    {
        LDSObjectArrayPtrBase() = default;
        LDSObjectArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectArrayPtr : LDSObjectArrayPtrBase
    {
        LDSObjectArrayPtr() = default;
        LDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);

        template <class TObjectPtr = LDSObjectPtr>
        TObjectPtr Item(uint32 index) const requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>>
        TObjectPtr Item(uint32 index) const requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>);

        template <class TObjectPtr = LDSObjectPtr, class TCallback>
        const LDSObjectArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSObjectArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TCallback>
        const LDSObjectArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TObjectPtr, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outObjects) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outObjects) const
            requires (!std::is_base_of_v<LDSObjectPtrBase, TObject>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TContainer>
        uint32 ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSObjectArrayPtr;

    template <class TObjectPtr>
    requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    struct PHOENIX_LDS_API TLDSObjectArrayPtr<TObjectPtr> : LDSObjectArrayPtrBase
    {
        using ObjectPtrT = TObjectPtr;

        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);

        operator LDSObjectArrayPtr() const;

        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        requires (TObjectPtr::ObjectT)
        const TLDSObjectArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <class TContainer>
        requires (TObjectPtr::ObjectT)
        uint32 ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <class TObject, class TObjectPtr>
    requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    struct PHOENIX_LDS_API TLDSObjectArrayPtr<TObject, TObjectPtr> : LDSObjectArrayPtrBase
    {
        using ObjectT = TObject;
        using ObjectPtrT = TObjectPtr;

        TLDSObjectArrayPtr() = default;
        TLDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectArrayPtr(const LDSObjectArrayPtrBase& other);

        operator LDSObjectArrayPtr() const;

        TObjectPtr Item(uint32 index) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectArrayPtr& ForEachItemAsReadObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outObjects) const;

        template <class TContainer>
        uint32 ReadObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };
}
