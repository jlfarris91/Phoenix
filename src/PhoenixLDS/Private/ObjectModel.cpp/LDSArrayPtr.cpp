
#include "ObjectModel/LDSArrayPtr.h"

#include "ObjectModel/LDSQueryContext.inl"

using namespace Phoenix::LDS;

LDSArrayPtr::LDSArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSArrayPtr::LDSArrayPtr(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

Phoenix::uint32 LDSArrayPtr::GetSize(const ILDSQueryContext& context) const
{
    return context.QueryRecordValueAs<uint32>(Path.Append("size"), 0, Flags);
}

const LDSRecord* LDSArrayPtr::GetItemRecord(
    const ILDSQueryContext& context,
    uint32 index) const
{
    return context.QueryRecord(Path.Append(index), Flags);
}

LDSRecordPtr LDSArrayPtr::Item(uint32 index) const
{
    return LDSRecordPtr(Path.Append(index), Flags);
}

LDSValueArrayPtr::LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtr(path, flags)
{
}

LDSValueArrayPtr::LDSValueArrayPtr(const LDSRecordPtr& other)
    : LDSArrayPtr(other)
{
}

LDSValuePtr LDSValueArrayPtr::Item(uint32 index) const
{
    return LDSArrayPtr::Item(index);
}
