
#include "ObjectModel/LDSRecordPtr.h"

#include "LDSRecordStore.h"
#include "ObjectModel/LDSQueryContext.h"

using namespace Phoenix::LDS;

LDSRecordPtr::LDSRecordPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : Path(path)
    , Flags(flags)
{
}

LDSRecordPtr::LDSRecordPtr(const LDSRecordPtr& other)
    : Path(other.Path)
    , Flags(other.Flags)
{
}

const LDSRecordPath& LDSRecordPtr::GetPath() const
{
    return Path;
}

ELDSRecordQueryFlags LDSRecordPtr::GetFlags() const
{
    return Flags;
}

void LDSRecordPtr::SetFlags(ELDSRecordQueryFlags flags)
{
    Flags = flags;
}

bool LDSRecordPtr::IsValid() const
{
    return Path.IsValid();
}

bool LDSRecordPtr::RecordExists(const ILDSQueryContext& context) const
{
    return context.RecordExists(Path);
}

ELDSValueType LDSRecordPtr::GetRecordType(const ILDSQueryContext& context) const
{
    return context.QueryRecord(Path, Flags)->GetValueType();
}