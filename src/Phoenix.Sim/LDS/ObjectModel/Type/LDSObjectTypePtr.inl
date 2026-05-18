#pragma once

#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSObjectTypePtr.h"

namespace Phoenix::LDS
{
    template <IsObjectPtr TObjectPtr, size_t N>
    TObjectPtr LDSObjectTypePtr::Property(const char(& chars)[N]) const
    {
        return TObjectPtr(Path.Append(chars), Flags);
    }

    template <size_t N>
    LDSObjectTypePtr LDSObjectTypePtr::ObjectProperty(const char(& chars)[N]) const
    {
        return Property<LDSObjectTypePtr>(chars);
    }

    template <class TObjectRefTypePtr, size_t N>
    TObjectRefTypePtr LDSObjectTypePtr::ObjectRefProperty(const char(& chars)[N]) const
    {
        return Property<LDSObjectRefTypePtr>(chars);
    }

    template <class TArrayTypePtr, size_t N>
    TArrayTypePtr LDSObjectTypePtr::ArrayProperty(const char(& chars)[N]) const
    {
        return Property<LDSArrayPtr>(chars);
    }

    template <class TNumericTypePtr, size_t N>
    TNumericTypePtr LDSObjectTypePtr::NumericProperty(const char(& chars)[N]) const
    {
        return Property<TNumericTypePtr>(chars);
    }

    template <class TEnumTypePtr, size_t N>
    TEnumTypePtr LDSObjectTypePtr::EnumProperty(const char(& chars)[N]) const
    {
        return Property<TEnumTypePtr>(chars);
    }
}

#include "LDSObjectTypePtr.inl"