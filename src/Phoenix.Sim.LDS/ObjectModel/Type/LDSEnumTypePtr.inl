#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSEnumTypePtr.h"

namespace Phoenix::LDS
{
    template <class TUnderlyingValue>
    TUnderlyingValue LDSEnumTypePtr::GetEnumValue(const ILDSQueryContext& context, const FName& key) const
    {
        return GetEnumItem(context, key).Value.GetValue(context, TUnderlyingValue{});
    }

    template <class TUnderlyingValue>
    bool LDSEnumTypePtr::TryGetEnumValue(
        const ILDSQueryContext& context,
        const FName& key,
        TUnderlyingValue& outValue) const
    {
        return GetEnumItem(context, key).Value.TryGetValue(context, outValue);
    }
}
