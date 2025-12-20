
#include "PhoenixSim/LDS/ObjectModel/LDSRecordPtr.h"

#include "PhoenixSim/LDS/LDSRecordStore.h"
#include "PhoenixSim/LDS/LDSQueryContext.h"

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

const Phoenix::FName& LDSRecordPtr::GetObjectId() const
{
    return Path.ObjectId;
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

const LDSRecord* LDSRecordPtr::GetRecord(const ILDSQueryContext& context) const
{
    return context.QueryRecord(Path, Flags);
}

ELDSValueType LDSRecordPtr::GetRecordType(const ILDSQueryContext& context) const
{
    return GetRecord(context)->GetValueType();
}

LDSTypedValue LDSRecordPtr::GetRecordValue(const ILDSQueryContext& context) const
{
    return GetRecord(context)->GetValue();
}
