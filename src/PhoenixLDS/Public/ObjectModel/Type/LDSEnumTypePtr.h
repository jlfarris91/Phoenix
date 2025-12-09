
#pragma once

#include "ObjectModel/LDSObjectArrayPtr.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSEnumTypeItemPtr : LDSObjectPtr
    {
        LDSEnumTypeItemPtr() = default;
        LDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypeItemPtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> Key;
        LDSValuePtr Value;

    private:
        void InitCommon();
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSEnumTypeItemPtr;

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSEnumTypeItemPtr<TValue, TValuePtr> : LDSObjectPtr
    {
        using ValueT = TValue;
        using ValuePtrT = TValuePtr;

        TLDSEnumTypeItemPtr() = default;
        TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypeItemPtr(const LDSEnumTypeItemPtr& other);

        TLDSValuePtr<FName> Key;
        TValuePtr Value;

    private:
        void InitCommon();
    };

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSEnumTypeItemPtr<TValuePtr> : TLDSEnumTypeItemPtr<typename TValuePtr::ValueT, TValuePtr>
    {
        TLDSEnumTypeItemPtr() = default;
        TLDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypeItemPtr(const LDSEnumTypeItemPtr& other);
    };

    struct PHOENIX_LDS_API LDSEnumTypePtrBase : LDSObjectPtr
    {
        LDSEnumTypePtrBase() = default;
        LDSEnumTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypePtrBase(const LDSEnumTypePtrBase& other);
    };

    struct PHOENIX_LDS_API LDSEnumTypePtr : LDSEnumTypePtrBase
    {
        LDSEnumTypePtr() = default;
        LDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypePtr(const LDSEnumTypePtrBase& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        TLDSObjectArrayPtr<LDSEnumTypeItemPtr> Items;
        TLDSValuePtr<FName> DefaultValue;

        LDSEnumTypeItemPtr GetEnumItem(const ILDSQueryContext& context, const FName& key) const;

        bool TryGetEnumItem(const ILDSQueryContext& context, const FName& key, LDSEnumTypeItemPtr& outItemPtr) const;

        template <class TUnderlyingValue>
        TUnderlyingValue GetEnumValue(const ILDSQueryContext& context, const FName& key) const;

        template <class TUnderlyingValue>
        bool TryGetEnumValue(const ILDSQueryContext& context, const FName& key, TUnderlyingValue& outValue) const;

    private:
        void InitCommon();
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSEnumTypePtr;

    template <class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    struct PHOENIX_LDS_API TLDSEnumTypePtr<TEnumTypeItemPtr> : LDSEnumTypePtrBase
    {
        using ValueT = typename TEnumTypeItemPtr::ValueT;
        using ValuePtrT = typename TEnumTypeItemPtr::ValuePtrT;
        using EnumTypeItemPtrT = TEnumTypeItemPtr;

        TLDSEnumTypePtr() = default;
        TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypePtr(const LDSEnumTypePtrBase& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        TLDSObjectArrayPtr<TEnumTypeItemPtr> Items;
        TLDSValuePtr<FName> DefaultValue;

        TEnumTypeItemPtr GetEnumItem(const ILDSQueryContext& context, const FName& key) const;

        bool TryGetEnumItem(const ILDSQueryContext& context, const FName& key, TEnumTypeItemPtr& outItemPtr) const;

        ValueT GetEnumValue(const ILDSQueryContext& context, const FName& key) const;

        bool TryGetEnumValue(const ILDSQueryContext& context, const FName& key, ValueT& outValue) const;

    private:
        void InitCommon();
    };

    template <class TValuePtr, class TEnumTypeItemPtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    struct PHOENIX_LDS_API TLDSEnumTypePtr<TValuePtr, TEnumTypeItemPtr> : TLDSEnumTypePtr<TEnumTypeItemPtr>
    {
        TLDSEnumTypePtr() = default;
        TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypePtr(const LDSEnumTypePtrBase& other);
    };

    template <class TValue, class TValuePtr, class TEnumTypeItemPtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && !std::is_base_of_v<LDSEnumTypeItemPtr, TValue> &&
              std::is_base_of_v<LDSValuePtrBase, TValuePtr> &&
              std::is_base_of_v<LDSEnumTypeItemPtr, TEnumTypeItemPtr>)
    struct PHOENIX_LDS_API TLDSEnumTypePtr<TValue, TValuePtr, TEnumTypeItemPtr> : TLDSEnumTypePtr<TEnumTypeItemPtr>
    {
        TLDSEnumTypePtr() = default;
        TLDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSEnumTypePtr(const LDSEnumTypePtrBase& other);
    };
}
