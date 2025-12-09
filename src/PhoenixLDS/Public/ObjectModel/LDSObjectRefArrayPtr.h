
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectRefArrayPtrBase : LDSArrayPtrBase
    {
        LDSObjectRefArrayPtrBase() = default;
        LDSObjectRefArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        LDSObjectRefArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectRefArrayPtr : LDSObjectRefArrayPtrBase
    {
        LDSObjectRefArrayPtr() = default;
        LDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);

        template <class TObjectRefPtr = LDSObjectRefPtr>
        TObjectRefPtr Item(uint32 index) const requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectRef, class TObjectRefPtr = TLDSObjectRefPtr<TObjectRef>>
        TObjectRefPtr Item(uint32 index) const requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectRef>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>>
        TObjectPtr ItemAsResolvedObject(
            const ILDSQueryContext& context,
            uint32 index) const;

        template <class TObjectRefPtr = LDSObjectRefPtr, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TCallback>
        const LDSObjectRefArrayPtr& ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectRefPtr = LDSObjectRefPtr, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outItems) const
            requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outItems) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectPtr = LDSObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, class TContainer>
        uint32 GetResolvedObjects(
            const ILDSQueryContext& context,
            TContainer& outObjects) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr;

    template <class TObjectRefPtr>
    requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr<TObjectRefPtr> : LDSObjectRefArrayPtrBase
    {
        using ObjectPtrT = typename TObjectRefPtr::ObjectPtrT;
        using ObjectRefPtrT = TObjectRefPtr;

        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);

        operator LDSObjectRefArrayPtr() const;

        TObjectRefPtr Item(uint32 index) const;

        ObjectPtrT ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachResolvedItemObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer>
        uint32 GetResolvedObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };

    template <class TObjectPtr, class TObjectRefPtr>
    requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr<TObjectPtr, TObjectRefPtr> : LDSObjectRefArrayPtrBase
    {
        using ObjectPtrT = TObjectPtr;
        using ObjectRefPtrT = TObjectRefPtr;

        TLDSObjectRefArrayPtr() = default;
        TLDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other);

        operator LDSObjectRefArrayPtr() const;

        TObjectRefPtr Item(uint32 index) const;

        TObjectPtr ItemAsResolvedObject(const ILDSQueryContext& context, uint32 index) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSObjectRefArrayPtr& ForEachResolvedItemObject(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer>
        uint32 GetResolvedObjects(const ILDSQueryContext& context, TContainer& outObjects) const;
    };
}