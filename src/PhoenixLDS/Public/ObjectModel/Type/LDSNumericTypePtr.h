
#pragma once

#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct LDSNumericTypePtr : LDSObjectPtr
    {
        LDSNumericTypePtr() = default;
        LDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSNumericTypePtr(const LDSRecordPtr& other);

        LDSValuePtr DefaultValue;
        LDSValuePtr MinValue;
        LDSValuePtr MaxValue;

    private:
        void InitCommon();
    };

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSNumericTypePtr;

    template <class TValue, class TValuePtr>
    requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSNumericTypePtr<TValue, TValuePtr> : LDSObjectPtr
    {
        using ValueT = TValue;
        using ValuePtrT = TValuePtr;

        TLDSNumericTypePtr() = default;
        TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSNumericTypePtr(const LDSNumericTypePtr& other);

        operator LDSNumericTypePtr() const;
        operator TLDSNumericTypePtr<TValuePtr>() const;

        TValuePtr DefaultValue;
        TValuePtr MinValue;
        TValuePtr MaxValue;

    private:
        void InitCommon();
    };

    template <class TValuePtr>
    requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    struct PHOENIX_LDS_API TLDSNumericTypePtr<TValuePtr> : TLDSNumericTypePtr<typename TValuePtr::ValueT, TValuePtr>
    {
        TLDSNumericTypePtr() = default;
        TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSNumericTypePtr(const LDSNumericTypePtr& other);
    };
}
