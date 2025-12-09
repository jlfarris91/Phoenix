
#pragma once

#include "LDSRecordPath.h"
#include "LDSRecordQueryFlags.h"
#include "Platform.h"

namespace Phoenix::LDS
{
    struct LDSObjectPtr;
    struct LDSRecordPath;
    struct LDSRecordPtr;
    class ILDSQueryContext;

    struct PHOENIX_LDS_API LDSArrayUtil final
    {
        static uint32 GetSize(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TItemPtr>
        static TItemPtr GetItem(
            const LDSRecordPath& path,
            uint32 index,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TValue, class TValuePtr>
        static TValue GetItemValueAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            uint32 index,
            const TValue& defaultValue = {},
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TObjectPtr, class TObjectRefPtr>
        static TObjectPtr GetResolvedItemObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            uint32 index,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TItemPtr, class TCallback>
        static void ForEachItem(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TValue, class TValuePtr, class TCallback>
        static void ForEachItemValueAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TObject, class TObjectPtr, class TCallback>
        static void ForEachItemAsReadObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TObjectPtr, class TObjectRefPtr, class TCallback>
        static void ForEachItemAsResolvedObject(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            const TCallback& callback,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TItemPtr, class TContainer>
        static uint32 GetItems(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outItems,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TValue, class TValuePtr, class TContainer>
        static uint32 GetItemValuesAs(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outValues,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TObjectPtr, class TObjectRefPtr, class TContainer>
        static uint32 GetItemsAsResolvedObjects(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outObjects,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);

        template <class TObject, class TObjectPtr, class TContainer>
        static uint32 GetItemsAsReadObjects(
            const ILDSQueryContext& context,
            const LDSRecordPath& path,
            TContainer& outObjects,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
    };
}
