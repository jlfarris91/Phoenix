
#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/LDSArrayPtr.h"
#include "Phoenix.Sim.LDS/ObjectModel/LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSObjectPtrBase : LDSRecordPtr
    {
        LDSObjectPtrBase() = default;
        LDSObjectPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_SIM_API LDSObjectPtr : LDSObjectPtrBase
    {
        LDSObjectPtr() = default;
        LDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectPtr(const LDSObjectPtrBase& other);

        template <class T>
        T ReadObject(const ILDSQueryContext& context) const;

        template <class T>
        bool TryReadObject(const ILDSQueryContext& context, T& outObject) const;

        template <IsRecordPtr TRecordPtr = LDSRecordPtr, size_t N>
        TRecordPtr Property(const char (&chars)[N]) const;

        template <IsValuePtr TValuePtr = LDSValuePtr, size_t N>
        TValuePtr Value(const char (&chars)[N]) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, size_t N>
        TValuePtr Value(const char (&chars)[N]) const;

        template <IsObjectPtr TObjectPtr = LDSObjectPtr, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const;

        template <IsObjectRefPtr TObjectRefPtr = LDSObjectRefPtr, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const;

        template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefPtr TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const;

        template <IsEnumFlagsPtr TEnumFlagsPtr = LDSEnumFlagsPtr, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const;

        template <IsNotEnumFlagsPtr TUnderlyingValue, IsEnumFlagsPtr TEnumFlagsPtr = TLDSEnumFlagsPtr<TUnderlyingValue>, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const;

        template <IsArrayPtr TArrayPtr = LDSArrayPtr, size_t N>
        TArrayPtr Array(const char (&chars)[N]) const;

        template <IsValueArrayPtr TValueArrayPtr = LDSValueArrayPtr, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const;

        template <IsValuePtr TValuePtr = LDSValuePtr, IsValueArrayPtr TValueArrayPtr = TLDSValueArrayPtr<TValuePtr>, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const;

        template <IsNotRecordPtr TValue, IsValuePtr TValuePtr = TLDSValuePtr<TValue>, IsValueArrayPtr TValueArrayPtr = TLDSValueArrayPtr<TValuePtr>, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const;

        template <IsObjectArrayPtr TObjectArrayPtr = LDSObjectArrayPtr, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const;

        template <IsObjectPtr TObjectPtr, IsObjectArrayPtr TObjectArrayPtr = TLDSObjectArrayPtr<TObjectPtr>, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectArrayPtr TObjectArrayPtr = TLDSObjectArrayPtr<TObjectPtr>, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const;

        template <IsObjectRefArrayPtr TObjectRefArrayPtr = LDSObjectRefArrayPtr, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const;

        template <IsObjectPtr TObjectPtr, IsObjectRefArrayPtr TObjectRefArrayPtr = TLDSObjectRefArrayPtr<TObjectPtr>, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const;

        template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr = TLDSObjectPtr<TObject>, IsObjectRefArrayPtr TObjectRefArrayPtr = TLDSObjectRefArrayPtr<TObjectPtr>, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const;
    };

    template <IsNotRecordPtr TObject>
    struct TLDSObjectPtr : LDSObjectPtr
    {
        using ObjectT = TObject;

        TLDSObjectPtr() = default;
        TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectPtr(const LDSObjectPtrBase& other);

        TObject ReadObject(const ILDSQueryContext& context) const;

        bool TryReadObject(const ILDSQueryContext& context, TObject& outObject) const;
    };
}

#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectPtr.inl"