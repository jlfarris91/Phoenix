#pragma once

#include "LDSNumericTypePtr.h"

namespace Phoenix::LDS
{
    template <class T, class TValuePtr>
    TLDSNumericTypePtr<T, TValuePtr>::TLDSNumericTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    TLDSNumericTypePtr<T, TValuePtr>::TLDSNumericTypePtr(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <class T, class TValuePtr>
    void TLDSNumericTypePtr<T, TValuePtr>::InitCommon()
    {
        DefaultValue = Value<T, TValuePtr>("default");
        MinValue = Value<T, TValuePtr>("min");
        MaxValue = Value<T, TValuePtr>("max");
    }
}
