
#include "ObjectModel/LDSObjectPtr.h"

using namespace Phoenix::LDS;

LDSObjectPtr::LDSObjectPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSObjectPtr::LDSObjectPtr(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}