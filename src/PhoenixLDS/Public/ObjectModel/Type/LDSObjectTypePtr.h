
#pragma once

#include "LDSArrayTypePtr.h"
#include "LDSEnumTypePtr.h"
#include "LDSNumericTypePtr.h"
#include "LDSObjectRefTypePtr.h"
#include "ObjectModel/LDSObjectPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectTypePtr : LDSObjectPtr
    {
        LDSObjectTypePtr() = default;
        LDSObjectTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectTypePtr(const LDSRecordPtr& other);

        template <class TObjectPtr = LDSObjectPtr, size_t N>
        TObjectPtr Property(const char (&chars)[N]) const;

        template <size_t N>
        LDSObjectTypePtr ObjectProperty(const char (&chars)[N]) const;

        template <size_t N>
        LDSObjectRefTypePtr ObjectRefProperty(const char (&chars)[N]) const;

        template <class TArrayTypePtr = LDSArrayTypePtr, size_t N>
        TArrayTypePtr ArrayProperty(const char (&chars)[N]) const;

        template <class TNumericTypePtr = LDSNumericTypePtr, size_t N>
        TNumericTypePtr NumericProperty(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>);

        template <class TValuePtr, class TNumericTypePtr = TLDSNumericTypePtr<TValuePtr>, size_t N>
        TNumericTypePtr NumericProperty(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TNumericTypePtr = TLDSNumericTypePtr<TValue, TValuePtr>, size_t N>
        TNumericTypePtr NumericProperty(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSValuePtrBase, TValue> && !std::is_base_of_v<LDSNumericTypePtr, TValue> &&
                      std::is_base_of_v<LDSValuePtrBase, TValuePtr> &&
                      std::is_base_of_v<LDSNumericTypePtr, TNumericTypePtr>);

        template <class TEnumTypePtr = LDSEnumTypePtr, size_t N>
        TEnumTypePtr EnumProperty(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSEnumTypePtr, TEnumTypePtr>);

        template <class TUnderlyingType, class TEnumTypePtr = TLDSEnumTypePtr<TUnderlyingType>, size_t N>
        TEnumTypePtr EnumProperty(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSEnumTypePtr, TUnderlyingType> && std::is_base_of_v<LDSEnumTypePtr, TEnumTypePtr>);

        LDSObjectPtr Default;

    private:
        void InitCommon();
    };
}
