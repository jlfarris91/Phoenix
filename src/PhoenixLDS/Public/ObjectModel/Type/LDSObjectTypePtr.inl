#pragma once

#include "LDSObjectTypePtr.h"

namespace Phoenix::LDS
{
    template <size_t N>
    LDSObjectPtr LDSObjectTypePtr::Property(const char(& chars)[N]) const
    {
        return LDSObjectPtr(Path.Append(chars), Flags);
    }

    template <size_t N>
    LDSObjectTypePtr LDSObjectTypePtr::ObjectProperty(const char(& chars)[N]) const
    {
        return LDSObjectTypePtr(Path.Append(chars), Flags);
    }

    template <size_t N>
    LDSObjectRefTypePtr LDSObjectTypePtr::ObjectRefProperty(const char(& chars)[N]) const
    {
        return LDSObjectRefTypePtr(Path.Append(chars), Flags);
    }

    template <class T, size_t N>
    TLDSNumericTypePtr<T> LDSObjectTypePtr::NumericProperty(const char(& chars)[N]) const
    {
        return TLDSNumericTypePtr<T>(Path.Append(chars), Flags);
    }

    template <class T, size_t N>
    TLDSEnumTypePtr<T> LDSObjectTypePtr::EnumProperty(const char(& chars)[N]) const
    {
        return TLDSEnumTypePtr<T>(Path.Append(chars), Flags);
    }
}
