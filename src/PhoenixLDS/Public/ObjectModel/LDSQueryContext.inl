#pragma once

#include "LDSQueryContext.h"
#include "LDSObjectSerializer.h"

namespace Phoenix::LDS
{
    template <class T>
    T ILDSQueryContext::QueryObjectRecordValueAs(
        const LDSRecordPath& path,
        const T& defaultValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryObjectRecord(path, flags))
        {
            return record->GetValueAs<T>();
        }
        return defaultValue;
    }

    template <class T>
    bool ILDSQueryContext::TryQueryObjectRecordValueAs(const LDSRecordPath& path, T& outValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryObjectRecord(path, flags))
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
        LDSReadObjectContext readContext;
        readContext.Query = shared_from_this();
        readContext.Path = path;
        readContext.Flags = flags;

        if constexpr (requires { T::Read(readContext, outObject); })
        {
            return T::Read(readContext, outObject);
        }
        else if constexpr (requires { LDSObjectSerializer<T>::Read(LDSReadObjectContext{}, outObject); })
        {
            return LDSObjectSerializer<T>::Read(readContext, outObject);
        }

        return false;
    }

    template <class T>
    T ILDSQueryContext::QueryTypeRecordValueAs(
        const LDSRecordPath& path,
        const T& defaultValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryTypeRecord(path, flags))
        {
            return record->GetValueAs<T>();
        }
        return defaultValue;
    }

    template <class T>
    bool ILDSQueryContext::TryQueryTypeRecordValueAs(
        const LDSRecordPath& path,
        T& outValue,
        ELDSRecordQueryFlags flags) const
    {
        if (const LDSRecord* record = QueryTypeRecord(path, flags))
        {
            outValue = record->GetValueAs<T>();
            return true;
        }
        return false;
    }
}
