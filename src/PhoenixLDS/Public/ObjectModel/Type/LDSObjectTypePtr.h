
#pragma once

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

        template <size_t N>
        LDSObjectPtr Property(const char (&chars)[N]) const;

        template <size_t N>
        LDSObjectTypePtr ObjectProperty(const char (&chars)[N]) const;

        template <size_t N>
        LDSObjectRefTypePtr ObjectRefProperty(const char (&chars)[N]) const;

        template <size_t N>
        LDSArrayTypeProperty ArrayProperty(const char (&chars)[N]) const;

        template <class T, size_t N>
        TLDSNumericTypePtr<T> NumericProperty(const char (&chars)[N]) const;

        template <class T, size_t N>
        TLDSEnumTypePtr<T> EnumProperty(const char (&chars)[N]) const;
        
        LDSObjectPtr Default;

    private:
        void InitCommon();
    };
}
