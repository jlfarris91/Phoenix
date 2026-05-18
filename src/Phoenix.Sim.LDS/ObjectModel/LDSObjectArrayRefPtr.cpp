
#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectRefArrayPtr.h"

using namespace Phoenix::LDS;

LDSObjectRefArrayPtrBase::LDSObjectRefArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtrBase(path, flags)
{
}

LDSObjectRefArrayPtrBase::LDSObjectRefArrayPtrBase(const LDSRecordPtr& other)
    : LDSArrayPtrBase(other)
{
}

LDSObjectRefArrayPtr::LDSObjectRefArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectRefArrayPtrBase(path, flags)
{
}

LDSObjectRefArrayPtr::LDSObjectRefArrayPtr(const LDSObjectRefArrayPtrBase& other)
    : LDSObjectRefArrayPtrBase(other)
{
}
