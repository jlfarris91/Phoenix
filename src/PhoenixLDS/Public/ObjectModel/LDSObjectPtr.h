
#pragma once

#include "LDSArrayPtr.h"
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

        template <class T, class TValuePtr = TLDSValuePtr<T>, size_t N>
        TValuePtr Value(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSValuePtr, T> );

        template <class T, size_t N>
        TLDSObjectPtr<T> Object(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectPtr, T> );

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const  requires ( !std::is_base_of_v<LDSObjectPtr, T> );

        template <class T, size_t N>
        LDSObjectRefPtr ObjectRef(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectRefPtr, T> );

        template <class T, class TObjectRefPtr = TLDSObjectRefPtr<T>, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSObjectRefPtr, T> );

        template <size_t N>
        LDSArrayPtr Array(const char (&chars)[N]) const;

        template <class T, size_t N>
        LDSValueArrayPtr ValueArray(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSValueArrayPtr, T> );

        template <class T, class TValuePtr = TLDSValuePtr<T>, size_t N>
        TLDSValueArrayPtr<T, TValuePtr> ValueArray(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSValueArrayPtr, T> );

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, size_t N>
        TLDSObjectArrayPtr<T, TObjectPtr> ObjectArray(const char (&chars)[N]) const;

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
