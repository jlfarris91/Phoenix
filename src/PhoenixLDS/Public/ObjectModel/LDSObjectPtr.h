
#pragma once

#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSObjectPtr : LDSRecordPtr
    {
        LDSObjectPtr() = default;
        LDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectPtr(const LDSRecordPtr& other);

        template <class T>
        T ReadObject(const ILDSQueryContext& context) const;

        template <class T>
        bool TryReadObject(const ILDSQueryContext& context, T& outObject) const;

        template <class T = LDSValuePtr, size_t N>
        LDSValuePtr Value(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSValuePtr, T> );

        template <class T, size_t N>
        TLDSValuePtr<T> Value(const char (&chars)[N]) const requires ( !std::is_same_v<LDSValuePtr, T> );

        template <class T, size_t N>
        TLDSObjectPtr<T> Object(const char (&chars)[N]) const;

        template <class T, size_t N>
        TLDSObjectRefPtr<T> ObjectRef(const char (&chars)[N]) const;

        template <class T, class TValuePtr = TLDSValuePtr<T>, size_t N>
        TLDSValueArrayPtr<T, TValuePtr> ValueArray(const char (&chars)[N]) const;

        template <class T, size_t N>
        TLDSObjectArrayPtr<T> ObjectArray(const char (&chars)[N]) const;

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, size_t N>
        TLDSObjectRefArrayPtr<T, TObjectPtr> ObjectRefArray(const char (&chars)[N]) const;

        template <class T, size_t N>
        TLDSEnumFlagsPtr<T> EnumFlags(const char (&chars)[N]) const;
    };

    template <class T>
    struct PHOENIX_LDS_API TLDSObjectPtr : LDSObjectPtr
    {
        TLDSObjectPtr() = default;
        TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectPtr(const LDSRecordPtr& other);

        T ReadObject(const ILDSQueryContext& context) const;

        bool TryReadObject(const ILDSQueryContext& context, T& outObject) const;
    };
}
