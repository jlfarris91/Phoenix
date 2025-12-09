#pragma once

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

    template <IsRecordPtr TRecordPtr, size_t N>
    TRecordPtr LDSObjectPtr::Property(const char(& chars)[N]) const
    {
        return TRecordPtr(Path.Append(chars), Flags);
    }

    template <IsValuePtr TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const
    {
        return Property<TValuePtr>(chars);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, size_t N>
    TValuePtr LDSObjectPtr::Value(const char(& chars)[N]) const
    {
        return Property<TValuePtr>(chars);
    }

    template <IsObjectPtr TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const
    {
        return Property<TObjectPtr>(chars);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, size_t N>
    TObjectPtr LDSObjectPtr::Object(const char(& chars)[N]) const
    {
        return Property<TObjectPtr>(chars);
    }

    template <IsObjectRefPtr TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefPtr TObjectRefPtr, size_t N>
    TObjectRefPtr LDSObjectPtr::ObjectRef(const char(& chars)[N]) const
    {
        return Property<TObjectRefPtr>(chars);
    }

    template <IsEnumFlagsPtr TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <IsNotEnumFlagsPtr TUnderlyingValue, IsEnumFlagsPtr TEnumFlagsPtr, size_t N>
    TEnumFlagsPtr LDSObjectPtr::EnumFlags(const char(& chars)[N]) const
    {
        return Property<TEnumFlagsPtr>(chars);
    }

    template <IsArrayPtr TArrayPtr, size_t N>
    TArrayPtr LDSObjectPtr::Array(const char(& chars)[N]) const
    {
        return Property<TArrayPtr>(chars);
    }

    template <IsValueArrayPtr TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <IsValuePtr TValuePtr, IsValueArrayPtr TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <IsNotRecordPtr TValue, IsValuePtr TValuePtr, IsValueArrayPtr TValueArrayPtr, size_t N>
    TValueArrayPtr LDSObjectPtr::ValueArray(const char(& chars)[N]) const
    {
        return Property<TValueArrayPtr>(chars);
    }

    template <IsObjectArrayPtr TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <IsObjectPtr TObjectPtr, IsObjectArrayPtr TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectArrayPtr TObjectArrayPtr, size_t N>
    TObjectArrayPtr LDSObjectPtr::ObjectArray(const char(& chars)[N]) const
    {
        return Property<TObjectArrayPtr>(chars);
    }

    template <IsObjectRefArrayPtr TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <IsObjectPtr TObjectPtr, IsObjectRefArrayPtr TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <IsNotRecordPtr TObject, IsObjectPtr TObjectPtr, IsObjectRefArrayPtr TObjectRefArrayPtr, size_t N>
    TObjectRefArrayPtr LDSObjectPtr::ObjectRefArray(const char(& chars)[N]) const
    {
        return Property<TObjectRefArrayPtr>(chars);
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectPtr<TObject>::TLDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
    }

    template <IsNotRecordPtr TObject>
    TLDSObjectPtr<TObject>::TLDSObjectPtr(const LDSObjectPtrBase& other)
        : LDSObjectPtr(other)
    {
    }

    template <IsNotRecordPtr TObject>
    TObject TLDSObjectPtr<TObject>::ReadObject(const ILDSQueryContext& context) const
    {
        return LDSObjectPtr::ReadObject<TObject>(context);
    }

    template <IsNotRecordPtr TObject>
    bool TLDSObjectPtr<TObject>::TryReadObject(const ILDSQueryContext& context, TObject& outObject) const
    {
        return LDSObjectPtr::TryReadObject<TObject>(context, outObject);
    }
}
