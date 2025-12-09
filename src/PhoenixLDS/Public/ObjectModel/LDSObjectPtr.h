
#pragma once

#include "LDSArrayPtr.h"
#include "LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectRefArrayPtrBase;
    struct LDSObjectArrayPtrBase;
    struct LDSValueArrayPtrBase;
    struct LDSEnumFlagsPtrBase;

    struct PHOENIX_LDS_API LDSObjectPtrBase : LDSRecordPtr
    {
        LDSObjectPtrBase() = default;
        LDSObjectPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectPtrBase(const LDSRecordPtr& other);
    };

    struct PHOENIX_LDS_API LDSObjectPtr : LDSObjectPtrBase
    {
        LDSObjectPtr() = default;
        LDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectPtr(const LDSObjectPtrBase& other);

        template <class T>
        T ReadObject(const ILDSQueryContext& context) const;

        template <class T>
        bool TryReadObject(const ILDSQueryContext& context, T& outObject) const;

        template <class TRecordPtr = LDSRecordPtr, size_t N>
        TRecordPtr Property(const char (&chars)[N]) const;

        template <class TValuePtr = LDSValuePtr, size_t N>
        TValuePtr Value(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, size_t N>
        TValuePtr Value(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSValuePtrBase, TValue>);

        template <class TObjectPtr = LDSObjectPtr, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, size_t N>
        TObjectPtr Object(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>);

        template <class TObjectRefPtr, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TObjectPtr, class TObjectRefPtr = TLDSObjectRefPtr<TObjectPtr>, size_t N>
        TObjectRefPtr ObjectRef(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSObjectRefPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>);

        template <class TEnumFlagsPtr = LDSEnumFlagsPtr, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSEnumFlagsPtrBase, TEnumFlagsPtr>);

        template <class TUnderlyingValue, class TEnumFlagsPtr = TLDSEnumFlagsPtr<TUnderlyingValue>, size_t N>
        TEnumFlagsPtr EnumFlags(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSEnumFlagsPtrBase, TUnderlyingValue> && std::is_base_of_v<LDSEnumFlagsPtrBase, TEnumFlagsPtr>);

        template <class TArrayPtr = LDSArrayPtr, size_t N>
        TArrayPtr Array(const char (&chars)[N]) const;

        template <class TValueArrayPtr = LDSValueArrayPtr, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>);

        template <class TValuePtr = LDSValuePtr, class TValueArrayPtr = TLDSValueArrayPtr<TValuePtr>, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSValueArrayPtrBase, TValuePtr> && std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>);

        template <class TValue, class TValuePtr = TLDSValuePtr<TValue>, class TValueArrayPtr = TLDSValueArrayPtr<TValue, TValuePtr>, size_t N>
        TValueArrayPtr ValueArray(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSValueArrayPtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>);

        template <class TObjectArrayPtr = LDSObjectArrayPtr, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>);

        template <class TObjectPtr, class TObjectArrayPtr = TLDSObjectArrayPtr<TObjectPtr>, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>);

        template <class TObject, class TObjectPtr = TLDSObjectPtr<TObject>, class TObjectArrayPtr = TLDSObjectArrayPtr<TObject, TObjectPtr>, size_t N>
        TObjectArrayPtr ObjectArray(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && !std::is_base_of_v<LDSObjectArrayPtrBase, TObject> &&
                      std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> &&
                      std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>);

        template <class TObjectRefArrayPtr = LDSObjectRefArrayPtr, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const
            requires (std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr>);

        template <class TObjectPtr, class TObjectRefArrayPtr = TLDSObjectRefArrayPtr<TObjectPtr>, size_t N>
        TObjectRefArrayPtr ObjectRefArray(const char (&chars)[N]) const
            requires (!std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr> && std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr>);
    };

    template <class TObject>
    struct PHOENIX_LDS_API TLDSObjectPtr : LDSObjectPtrBase
    {
        using ObjectT = TObject;

        TLDSObjectPtr() = default;
        TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSObjectPtr(const LDSObjectPtrBase& other);

        operator LDSObjectPtr() const;

        TObject ReadObject(const ILDSQueryContext& context) const;

        bool TryReadObject(const ILDSQueryContext& context, TObject& outObject) const;
    };
}
