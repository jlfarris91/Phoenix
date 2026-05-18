
#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectRefPtr.h"

using namespace Phoenix::LDS;

LDSObjectRefPtrBase::LDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSObjectRefPtrBase::LDSObjectRefPtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

Phoenix::FName LDSObjectRefPtrBase::GetReferenceId(const ILDSQueryContext& context) const
{
    return context.QueryRecordValueAs<FName>(Path, FName::None, Flags);
}

LDSObjectRefPtr::LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectRefPtrBase(path, flags)
{
}

LDSObjectRefPtr::LDSObjectRefPtr(const LDSObjectRefPtrBase& other)
    : LDSObjectRefPtrBase(other)
{
}
