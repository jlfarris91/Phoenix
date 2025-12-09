#pragma once

#include "LDSObjectPtr.h"
#include "LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <class T>
    T LDSObjectPtr::ReadObject(const ILDSQueryContext& context) const
    {
        PHX_ASSERT(FName::IsNoneOrEmpty(Path.Path));
        return context.ReadObject<T>(Path, Flags);
    }

    template <class T>
    bool LDSObjectPtr::TryReadObject(const ILDSQueryContext& context, T& outObject) const
    {
        PHX_ASSERT(FName::IsNoneOrEmpty(Path.Path));
        return context.TryReadObject<T>(Path, outObject, Flags);
    }

    template <class TRecordPtr, size_t N>
    TRecordPtr LDSObjectPtr::Property(const char(& chars)[N]) const
    {
        return TRecordPtr(Path.Append(chars), Flags);
    }

    template <class TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSValuePtrBase, TValuePtr>)
    {
        return Property<TValuePtr>(chars);
    }

    template <class T, class TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSValuePtrBase, T>)
    {
        return Property<TValuePtr>(chars);
    }

    template <class TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return Property<TObjectPtr>(chars);
    }

    template <class T, class TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, T> && std::is_base_of_v<LDSObjectPtrBase, TObjectPtr>)
    {
        return Property<TObjectPtr>(chars);
    }

    template <class TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <class T, class TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSObjectRefPtrBase, T> && std::is_base_of_v<LDSObjectRefPtrBase, TObjectRefPtr>)
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <class TArrayPtr, size_t N>
    TArrayPtr LDSObjectPtr::Array(const char(& chars)[N]) const
    {
        return Property<TArrayPtr>(chars);
    }

    template <class TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>)
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <class TValuePtr, class TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSValueArrayPtrBase, TValuePtr> && std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>)
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <class TValue, class TValuePtr, class TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
            requires (!std::is_base_of_v<LDSValueArrayPtrBase, TValue> && std::is_base_of_v<LDSValuePtrBase, TValuePtr> && std::is_base_of_v<LDSValueArrayPtrBase, TValueArrayPtr>)
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <class TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>)
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <class TObjectPtr, class TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> && std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>)
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <class TObject, class TObjectPtr, class TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSObjectPtrBase, TObject> && !std::is_base_of_v<LDSObjectArrayPtrBase, TObject> &&
                  std::is_base_of_v<LDSObjectPtrBase, TObjectPtr> &&
                  std::is_base_of_v<LDSObjectArrayPtrBase, TObjectArrayPtr>)
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <class TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr>)
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <class T, class TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr> && std::is_base_of_v<LDSObjectRefArrayPtrBase, TObjectRefArrayPtr>)
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <class TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const
        requires (std::is_base_of_v<LDSEnumFlagsPtrBase, TEnumFlagsPtr>)
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <class TUnderlyingValue, class TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const
        requires (!std::is_base_of_v<LDSEnumFlagsPtrBase, TUnderlyingValue> && std::is_base_of_v<LDSEnumFlagsPtrBase, TEnumFlagsPtr>)
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <class TObject>
    TLDSObjectPtr<TObject>::TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtrBase(path, flags)
    {
    }

    template <class TObject>
    TLDSObjectPtr<TObject>::TLDSObjectPtr(const LDSObjectPtrBase& other)
        : LDSObjectPtrBase(other)
    {
    }

    template <class TObject>
    TLDSObjectPtr<TObject>::operator LDSObjectPtr() const
    {
        return LDSObjectPtr(Path, Flags);
    }

    template <class TObject>
    TObject TLDSObjectPtr<TObject>::ReadObject(const ILDSQueryContext& context) const
    {
        return LDSObjectPtr::ReadObject<TObject>(context);
    }

    template <class TObject>
    bool TLDSObjectPtr<TObject>::TryReadObject(const ILDSQueryContext& context, TObject& outObject) const
    {
        return LDSObjectPtr::TryReadObject<TObject>(context, outObject);
    }
}
