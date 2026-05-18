
#pragma once

#include "PhoenixSim/LDS/ObjectModel/LDSObjectArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSObjectPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSEnumTypeItemPtr : LDSObjectPtr
    {
        LDSEnumTypeItemPtr() = default;
        LDSEnumTypeItemPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypeItemPtr(const LDSRecordPtr& other);

        TLDSValuePtr<FName> Key;
        LDSValuePtr Value;

    private:
        void InitCommon();
    };

    struct PHOENIX_SIM_API LDSEnumTypePtr : LDSObjectPtr
    {
        LDSEnumTypePtr() = default;
        LDSEnumTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSEnumTypePtr(const LDSObjectPtr& other);

        TLDSValuePtr<ELDSValueType> UnderlyingType;
        LDSObjectArrayPtr Items;
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
}

#include "PhoenixSim/LDS/ObjectModel/Type/LDSEnumTypePtr.inl"
