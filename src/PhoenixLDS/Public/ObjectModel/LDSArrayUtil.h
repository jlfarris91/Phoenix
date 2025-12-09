
#pragma once

#include "LDSForwardDecls.h"
#include "LDSRecordQueryFlags.h"
#include "Platform.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSArrayUtil final
    {
        static uint32 GetSize(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsRecordPtr TItemPtr>
        static TItemPtr GetItem(
            const LDSRecordPath& path,
            uint32 index,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
        static TValue GetItemValueAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            uint32 index,
            const TValue& defaultValue = {},
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr>
        static TObjectPtr GetResolvedItemObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            uint32 index,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsRecordPtr TItemPtr, class TCallback>
        static void ForEachItem(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TCallback>
        static void ForEachItemValueAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TCallback>
        static void ForEachItemAsReadObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TCallback>
        static void ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsRecordPtr TItemPtr, class TContainer>
        static uint32 GetItems(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outItems,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, class TContainer>
        static uint32 GetItemValuesAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outValues,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, class TContainer>
        static uint32 GetItemsAsResolvedObjects(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outObjects,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, class TContainer>
        static uint32 GetItemsAsReadObjects(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outObjects,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
    };
}

#include "LDSArrayUtil.inl"
