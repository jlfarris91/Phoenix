
#include "Phoenix.Sim.LDS/ObjectModel/LDSObjectArrayPtr.h"

using namespace Phoenix::LDS;

LDSObjectArrayPtrBase::LDSObjectArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtrBase(path, flags)
{
}

LDSObjectArrayPtrBase::LDSObjectArrayPtrBase(const LDSRecordPtr& other)
    : LDSArrayPtrBase(other)
{
}

LDSObjectArrayPtr::LDSObjectArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectArrayPtrBase(path, flags)
{
}

LDSObjectArrayPtr::LDSObjectArrayPtr(const LDSObjectArrayPtrBase& other)
    : LDSObjectArrayPtrBase(other)
{
}
