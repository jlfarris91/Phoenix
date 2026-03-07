#pragma once

#include "PhoenixSim/LDS/FeatureLDS.h"

namespace Phoenix::LDS
{
    template <IsNotRecordPtr TValue>
    TValue FeatureLDS::GetValue(WorldConstRef world, const LDSValuePtr& ptr, const TValue& defaultValue)
    {
        auto queryContext = StaticGetWorldQueryContext(world);
        return queryContext ? ptr.GetValue<TValue>(*queryContext, defaultValue) : defaultValue;
    }

    template <IsNotRecordPtr TValue>
    bool FeatureLDS::TryGetValue(WorldConstRef world, const LDSValuePtr& ptr, TValue& outValue)
    {
        auto queryContext = StaticGetWorldQueryContext(world);
        return queryContext && ptr.TryGetValue<TValue>(*queryContext, outValue);
    }

    template <IsNotRecordPtr TValue>
    TValue FeatureLDS::GetValue(WorldConstRef world, const TLDSValuePtr<TValue>& ptr, const TValue& defaultValue)
    {
        auto queryContext = StaticGetWorldQueryContext(world);
        return queryContext ? ptr.GetValue(*queryContext, defaultValue) : defaultValue;
    }

    template <IsNotRecordPtr TValue>
    bool FeatureLDS::TryGetValue(WorldConstRef world, const TLDSValuePtr<TValue>& ptr, TValue& outValue)
    {
        auto queryContext = StaticGetWorldQueryContext(world);
        return queryContext && ptr.TryGetValue(*queryContext, outValue);
    }
}
