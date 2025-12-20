#pragma once

#include "PhoenixSim/LDS/ObjectModel/LDSObjectSerializer.h"

namespace Phoenix::LDS
{
    template <class T>
    T ILDSQueryContext::QueryRecordValueAs(
        const LDSRecordPath& path,
        const T& defaultValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryRecord(path, flags))
        {
            return record->GetValueAs<T>();
        }
        return defaultValue;
    }

    template <class T>
    bool ILDSQueryContext::TryQueryRecordValueAs(const LDSRecordPath& path, T& outValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryRecord(path, flags))
        {
            outValue = record->GetValueAs<T>();
            return true;
        }
        return false;
    }

    template <class T>
    T ILDSQueryContext::ReadObject(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
    {
        T object;
        (void)TryReadObject(path, object, flags);
        return object;
    }

    template <class T>
    bool ILDSQueryContext::TryReadObject(
        const LDSRecordPath& path,
        T& outObject,
        ELDSRecordQueryFlags flags) const
    {
        LDSReadObjectArgs readContext(*this, path, flags);
        if constexpr (requires { T::Read(readContext, outObject); })
        {
            return T::Read(readContext, outObject);
        }
        else if constexpr (requires { LDSObjectSerializer<T>::Read(readContext, outObject); })
        {
            return LDSObjectSerializer<T>::Read(readContext, outObject);
        }
        return false;
    }
}
