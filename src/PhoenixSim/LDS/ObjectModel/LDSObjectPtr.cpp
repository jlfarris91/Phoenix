
#include "PhoenixSim/LDS/ObjectModel/LDSObjectPtr.h"

using namespace Phoenix::LDS;

LDSObjectPtrBase::LDSObjectPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSObjectPtrBase::LDSObjectPtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

LDSObjectPtr::LDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectPtrBase(path, flags)
{
}

LDSObjectPtr::LDSObjectPtr(const LDSObjectPtrBase& other)
    : LDSObjectPtrBase(other)
{
}