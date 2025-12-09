
#pragma once

#include "LDSArrayPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSValueArrayPtrBase : LDSArrayPtrBase
    {
        LDSValueArrayPtrBase() = default;
        LDSValueArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValueArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSValueArrayPtr : LDSValueArrayPtrBase
    {
        LDSValueArrayPtr() = default;
        LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValueArrayPtr(const LDSValueArrayPtrBase& other);

        template <class TValuePtr = LDSValuePtr>
        TValuePtr Item(uint32 index) const;

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>>
        TValue ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <class TValuePtr = LDSValuePtr, class TCallback>
        const LDSValueArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItem(
            const ILDSQueryContext& context,
            const TCallback& callback) const
            requires (!std::is_base_of_v<LDSValuePtrBase, TValue>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TValuePtr = LDSValuePtr, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outItems) const
            requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TContainer>
        uint32 GetItems(
            const ILDSQueryContext& context,
            TContainer& outItems) const
            requires (!std::is_base_of_v<LDSValuePtrBase, TValue>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TContainer>
        uint32 GetItemValuesAs(const ILDSQueryContext& context, TContainer& outValues) const;
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSValueArrayPtr;

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSValueArrayPtr<TValuePtr> : LDSValueArrayPtrBase
    {
        using ValueT = typename TValuePtr::ValueT;
        using ValuePtrT = TValuePtr;

        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSValueArrayPtrBase& other);

        operator LDSValueArrayPtr() const;

        operator TLDSValueArrayPtr<ValueT, TValuePtr>() const;

        TValuePtr Item(uint32 index) const;

        ValueT ItemValue(const ILDSQueryContext& context, uint32 index, const ValueT& defaultValue = {}) const;

        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;
    
        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItemValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer>
        uint32 GetItemValues(const ILDSQueryContext& context, TContainer& outValues) const;
    };

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSValueArrayPtr<TValue, TValuePtr> : LDSValueArrayPtrBase
    {
        using ValueT = TValue;
        using ValuePtrT = TValuePtr;

        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSValueArrayPtrBase& other);

        operator LDSValueArrayPtr() const;

        operator TLDSValueArrayPtr<TValuePtr>() const;

        TValuePtr Item(uint32 index) const;

        TValue ItemValue(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TCallback>
        const TLDSValueArrayPtr& ForEachItemValue(const ILDSQueryContext& context, const TCallback& callback) const;

        template <class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <class TContainer>
        uint32 GetItemValues(const ILDSQueryContext& context, TContainer& outValues) const;
    };
}
