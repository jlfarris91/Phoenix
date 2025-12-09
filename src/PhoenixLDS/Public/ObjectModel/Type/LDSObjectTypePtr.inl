#pragma once

#include "LDSObjectTypePtr.h"

namespace Phoenix::LDS
{
    template <class TObjectPtr, size_t N>
    TObjectPtr LDSObjectTypePtr::Property(const char(& chars)[N]) const
    {
        return TObjectPtr(Path.Append(chars), Flags);
    }

    template <size_t N>
    LDSObjectTypePtr LDSObjectTypePtr::ObjectProperty(const char(& chars)[N]) const
    {
        return Property<LDSObjectTypePtr>(chars);
    }

    template <size_t N>
    LDSObjectRefTypePtr LDSObjectTypePtr::ObjectRefProperty(const char(& chars)[N]) const
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
        requires (std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>)
    {
        return Property<TNumericTypePtr>(chars);
    }

    template <class TValue, class TNumericTypePtr, size_t N>
    TNumericTypePtr LDSObjectTypePtr::NumericProperty(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSValuePtrBase, TValue> && std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>)
    {
        return Property<TNumericTypePtr>(chars);
    }

    template <class TValue, class TValuePtr, class TNumericTypePtr, size_t N>
    TNumericTypePtr LDSObjectTypePtr::NumericProperty(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && !std::is_base_of_v<LDSNumericTypePtr, TValue> &&
                  std::is_base_of_v<LDSValuePtrBase, TValuePtr> &&
                  std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>)
    {
        return Property<TNumericTypePtr>(chars);
    }

    template <class TEnumTypePtr, size_t N>
    TEnumTypePtr LDSObjectTypePtr::EnumProperty(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSEnumTypePtr, TEnumTypePtr>)
    {
        return Property<TEnumTypePtr>(chars);
    }

    template <class TUnderlyingType, class TEnumTypePtr, size_t N>
    TEnumTypePtr LDSObjectTypePtr::EnumProperty(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSEnumTypePtr, TUnderlyingType> && std::is_base_of_v<LDSEnumTypePtr, TEnumTypePtr>)
    {
        return Property<TEnumTypePtr>(chars);
    }
}
