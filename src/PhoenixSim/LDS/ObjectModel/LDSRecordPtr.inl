#pragma once

#include "PhoenixSim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class T>
    T LDSRecordPtr::GetRecordValueAs(const ILDSQueryContext& context, const T& defaultValue) const
    {
        return context.QueryRecordValueAs<T>(Path, defaultValue, Flags);
    }

    template <class TItemPtr> requires (IsRecordPtr<TItemPtr>)
    TItemPtr LDSRecordPtr::As() const
    {
        return TItemPtr(Path, Flags);
    }
}
