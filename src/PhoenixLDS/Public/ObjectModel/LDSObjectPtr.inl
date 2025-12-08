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

    template <class T, size_t N>
    LDSValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const requires (std::is_base_of_v<LDSValuePtr, T>)
    {
        return LDSValuePtr(Path.Append(chars), Flags);
    }

    template <class T, class TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSValuePtr, T>)
    {
        return TValuePtr(Path.Append(chars), Flags);
    }

    template <class T, size_t N>
    TLDSObjectPtr<T> LDSObjectPtr::Object(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectPtr, T>)
    {
        return TLDSObjectPtr<T>(Path.Append(chars), Flags);
    }

    template <class T, class TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectPtr, T>)
    {
        return TObjectPtr(Path.Append(chars), Flags);
    }

    template <class T, size_t N>
    LDSObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const requires (std::is_base_of_v<LDSObjectRefPtr, T>)
    {
        return LDSObjectRefPtr(Path.Append(chars), Flags);
    }

    template <class T, class TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const requires (!std::is_base_of_v<LDSObjectRefPtr, T>)
    {
        return TObjectRefPtr(Path.Append(chars), Flags);
    }

    template <size_t N>
    LDSArrayPtr LDSObjectPtr::Array(const char(& chars)[N]) const
    {
        return LDSArrayPtr(Path.Append(chars), Flags);
    }

    template <class T, class TValuePtr, size_t N>
    TLDSValueArrayPtr<T, TValuePtr> LDSObjectPtr::ValueArray(const char(& chars)[N]) const
    {
        return TLDSValueArrayPtr<T, TValuePtr>(Path.Append(chars), Flags);
    }

    template <class T, class TObjectPtr, size_t N>
    TLDSObjectArrayPtr<T, TObjectPtr> LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
    {
        return TLDSObjectArrayPtr<T, TObjectPtr>(Path.Append(chars), Flags);
    }

    template <class T, class TObjectPtr, size_t N>
    TLDSObjectRefArrayPtr<T, TObjectPtr> LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
    {
        return TLDSObjectRefArrayPtr<T, TObjectPtr>(Path.Append(chars), Flags);
    }

    template <class T, size_t N>
    TLDSEnumFlagsPtr<T> LDSObjectPtr::EnumFlags(const char(& chars)[N]) const
    {
        return TLDSEnumFlagsPtr<T>(Path.Append(chars), Flags);
    }

    template <class T>
    TLDSObjectPtr<T>::TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags): LDSObjectPtr(path, flags)
    {
    }

    template <class T>
    TLDSObjectPtr<T>::TLDSObjectPtr(const LDSRecordPtr& other): LDSObjectPtr(other)
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
