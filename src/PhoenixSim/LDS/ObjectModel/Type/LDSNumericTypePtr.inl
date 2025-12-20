#pragma once

#include "PhoenixSim/LDS/ObjectModel/Type/LDSNumericTypePtr.h"

namespace Phoenix::LDS
{
    template <IsValuePtr TValuePtr>
    TLDSNumericTypePtrBase<TValuePtr>::TLDSNumericTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
    }

    template <IsValuePtr TValuePtr>
    TLDSNumericTypePtrBase<TValuePtr>::TLDSNumericTypePtrBase(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
    }

    template <IsValuePtr TValuePtr>
    void TLDSNumericTypePtrBase<TValuePtr>::InitCommon()
    {
        DefaultValue = Value<TValuePtr>("default");
        MinValue = Value<TValuePtr>("min");
        MaxValue = Value<TValuePtr>("max");
    }
}
