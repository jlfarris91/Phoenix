
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSValueArrayPtr : LDSArrayPtrBase
    {
        LDSValueArrayPtr() = default;
        LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValueArrayPtr(const LDSRecordPtr& other);

        template <class TValuePtr = LDSValuePtr>
        TValuePtr Item(uint32 index) const;

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>>
        TValue ItemValue(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <class TValuePtr = LDSValuePtr, class TCallback>
        const LDSValueArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSValuePtr, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (!std::is_base_of_v<LDSValuePtr, TValue>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItemValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TValuePtr = LDSValuePtr, class TContainer = TArray2<TValuePtr>>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outItems) const
            requires (std::is_base_of_v<LDSValuePtr, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TContainer = TArray2<TValuePtr>>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outValues) const
            requires (!std::is_base_of_v<LDSValuePtr, TValue>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TContainer = TArray2<TValue>>
        uint32 GetItemValues(const ILDSQueryContext& context, TContainer& outValues) const;
    };

    template <class TValue, class TValuePtr = TLDSValuePtr<TValue>>
    struct PHOENIX_LDS_API TLDSValueArrayPtr : LDSArrayPtrBase
    {
        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSRecordPtr& other);

        TValuePtr Item(uint32 index) const;

        TValue ItemValue(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItemValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer = TArray2<TValuePtr>>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer = TArray2<TValue>>
        uint32 GetItemValues(const ILDSQueryContext& context, TContainer& outValues) const;
    };
}
