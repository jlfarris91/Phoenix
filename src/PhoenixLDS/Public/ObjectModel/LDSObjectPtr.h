
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

        template <class TRecordPtr = LDSRecordPtr, size_t N>
        TRecordPtr Property(const char (&chars)[N]) const;

        template <class TValuePtr = LDSValuePtr, size_t N>
        TValuePtr Value(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSValuePtr, TValuePtr> );

        template <class T, class TValuePtr = TLDSValuePtr<T>, size_t N>
        TValuePtr Value(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSValuePtr, T> );

        template <class TObjectPtr, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectPtr, TObjectPtr> );

        template <class T, class TObjectPtr = TLDSObjectPtr<T>, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const  requires ( !std::is_base_of_v<LDSObjectPtr, T> );

        template <class TObjectRefPtr, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectRefPtr, TObjectRefPtr> );

        template <class T, class TObjectRefPtr = TLDSObjectRefPtr<T>, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSObjectRefPtr, T> );

        template <class TEnumFlagsPtr = LDSEnumFlagsPtr, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSEnumFlagsPtr, TEnumFlagsPtr> );

        template <class T, class TEnumFlagsPtr = TLDSEnumFlagsPtr<T>, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSEnumFlagsPtr, TEnumFlagsPtr> );

        template <class TArrayPtr = LDSArrayPtr, size_t N>
        TArrayPtr Array(const char (&chars)[N]) const;

        template <class TValueArrayPtr = LDSValueArrayPtr, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSValueArrayPtr, TValueArrayPtr> );

        template <class T, class TValueArrayPtr = TLDSValueArrayPtr<T>, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSValueArrayPtr, T> );

        template <class TObjectArrayPtr = LDSObjectArrayPtr, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectArrayPtr, TObjectArrayPtr> );

        template <class T, class TObjectArrayPtr = TLDSObjectArrayPtr<T>, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSObjectArrayPtr, T> );

        template <class TObjectRefArrayPtr = LDSObjectRefArrayPtr, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const requires ( std::is_base_of_v<LDSObjectRefArrayPtr, TObjectRefArrayPtr> );

        template <class T, class TObjectRefArrayPtr = TLDSObjectRefArrayPtr<T>, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const requires ( !std::is_base_of_v<LDSObjectRefArrayPtr, TObjectRefArrayPtr> );
    };

    template <class T>
    struct PHOENIX_LDS_API TLDSObjectPtr : LDSObjectPtr
    {
        TLDSObjectPtr() = default;
        TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectPtr(const LDSObjectPtr& other);

        T ReadObject(const ILDSQueryContext& context) const;

        bool TryReadObject(const ILDSQueryContext& context, T& outObject) const;
    };
}
