
#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectPtr.h"
#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSArrayTypePtr.h"
#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSEnumTypePtr.h"
#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSNumericTypePtr.h"
#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSObjectRefTypePtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectTypePtr : LDSObjectPtr
    {
        LDSObjectTypePtr() = default;
        LDSObjectTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectTypePtr(const LDSRecordPtr& other);

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, size_t N>
        TObjectPtr Property(const char (&chars)[N]) const;

        template <size_t N>
        LDSObjectTypePtr ObjectProperty(const char (&chars)[N]) const;

        template <class TObjectRefTypePtr = LDSObjectRefTypePtr, size_t N>
        TObjectRefTypePtr ObjectRefProperty(const char (&chars)[N]) const;

        template <class TArrayTypePtr = LDSArrayTypePtr, size_t N>
        TArrayTypePtr ArrayProperty(const char (&chars)[N]) const;

        template <class TNumericTypePtr = LDSNumericTypePtr, size_t N>
        TNumericTypePtr NumericProperty(const char (&chars)[N]) const;

        template <class TEnumTypePtr = LDSEnumTypePtr, size_t N>
        TEnumTypePtr EnumProperty(const char (&chars)[N]) const;

        LDSObjectPtr Default;

    private:
        void InitCommon();
    };
}

#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSObjectTypePtr.inl"
