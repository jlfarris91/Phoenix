
#pragma once

#include "PhoenixSim/LDS/ObjectModel/LDSArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSValueArrayPtrBase : LDSArrayPtrBase
    {
        LDSValueArrayPtrBase() = default;
        LDSValueArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValueArrayPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_SIM_API LDSValueArrayPtr : LDSValueArrayPtrBase
    {
        LDSValueArrayPtr() = default;
        LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSValueArrayPtr(const LDSValueArrayPtrBase& other);

        template <IsValuePtr TValuePtr = LDSValuePtr>
        TValuePtr Item(uint32 index) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>>
        TValue ItemValueAs(const ILDSQueryContext& context, uint32 index, const TValue& defaultValue = {}) const;

        template <IsValuePtr TValuePtr = LDSValuePtr, class TCallback>
        const LDSValueArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItem(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, class TCallback>
        const LDSValueArrayPtr& ForEachItemValueAs(const ILDSQueryContext& context, const TCallback& callback) const;

        template <IsValuePtr TValuePtr = LDSValuePtr, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, class TContainer>
        uint32 GetItems(const ILDSQueryContext& context, TContainer& outItems) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, class TContainer>
        uint32 GetItemValuesAs(const ILDSQueryContext& context, TContainer& outValues) const;
    };

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr>
    struct TLDSValueArrayPtr<TValue, TValuePtr> : LDSValueArrayPtrBase
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

    template <IsValuePtr TValuePtr>
    struct TLDSValueArrayPtr<TValuePtr, EnableIfValuePtr<TValuePtr>> : TLDSValueArrayPtr<typename TValuePtr::ValueT, TValuePtr>
    {
        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSValueArrayPtrBase& other);
    };

    template <IsNotRecordPtr TValue>
    struct TLDSValueArrayPtr<TValue, EnableIfNotRecordPtr<TValue>> : TLDSValueArrayPtr<TValue, TLDSValuePtr<TValue>>
    {
        TLDSValueArrayPtr() = default;
        TLDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSValueArrayPtr(const LDSValueArrayPtrBase& other);
    };
}

#include "PhoenixSim/LDS/ObjectModel/LDSValueArrayPtr.inl"