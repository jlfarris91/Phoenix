
#include "ObjectModel/LDSArrayPtr.h"

#include "ObjectModel/LDSQueryContext.inl"

using namespace Phoenix::LDS;

LDSArrayPtrBase::LDSArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSArrayPtrBase::LDSArrayPtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

Phoenix::uint32 LDSArrayPtrBase::GetSize(const ILDSQueryContext& context) const
{
    return context.QueryRecordValueAs<uint32>(Path.Append("size"), 0, Flags);
}

const LDSRecord* LDSArrayPtrBase::GetItemRecord(
    const ILDSQueryContext& context,
    uint32 index) const
{
    return context.QueryRecord(Path.Append(index), Flags);
}

LDSArrayPtr::LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtrBase(path, flags)
{
}

LDSArrayPtr::LDSArrayPtr(const LDSRecordPtr& other)
    : LDSArrayPtrBase(other)
{
}