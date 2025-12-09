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
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const requires (std::is_base_of_v<LDSValuePtr, TValuePtr>)
    {
        return Property<TValuePtr>(chars);
    }

    template <class T, class TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSValuePtr, T>)
    {
        return Property<TValuePtr>(chars);
    }

    template <class TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectPtr, TObjectPtr>)
    {
        return Property<TObjectPtr>(chars);
    }

    template <class T, class TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectPtr, T>)
    {
        return Property<TObjectPtr>(chars);
    }

    template <class TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectRefPtr, TObjectRefPtr>)
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <class T, class TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectRefPtr, T>)
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <class TArrayPtr, size_t N>
    TArrayPtr LDSObjectPtr::Array(const char(& chars)[N]) const
    {
        return Property<TArrayPtr>(chars);
    }

    template <class TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const requires (std::is_base_of_v<LDSValueArrayPtr, TValueArrayPtr>)
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <class T, class TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSValueArrayPtr, T>)
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <class TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectArrayPtr, TObjectArrayPtr>)
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <class T, class TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectArrayPtr, T>)
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <class TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectRefArrayPtr, TObjectRefArrayPtr>)
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <class T, class TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectRefArrayPtr, TObjectRefArrayPtr>)
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <class TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const requires (std::is_base_of_v<LDSEnumFlagsPtr, TEnumFlagsPtr>)
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <class T, class TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSEnumFlagsPtr, TEnumFlagsPtr>)
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <class T>
    TLDSObjectPtr<T>::TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
    }

    template <class T>
    TLDSObjectPtr<T>::TLDSObjectPtr(const LDSObjectPtr& other)
        : LDSObjectPtr(other)
    {
    }

    template <class T>
    T TLDSObjectPtr<T>::ReadObject(const ILDSQueryContext& context) const
    {
        return LDSObjectPtr::ReadObject<T>(context);
    }

    template <class T>
    bool TLDSObjectPtr<T>::TryReadObject(const ILDSQueryContext& context, T& outObject) const
    {
        return LDSObjectPtr::TryReadObject<T>(context, outObject);
    }
}
