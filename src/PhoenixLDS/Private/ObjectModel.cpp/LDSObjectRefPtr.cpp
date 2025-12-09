
#include "ObjectModel/LDSObjectRefPtr.h"

using namespace Phoenix::LDS;

LDSObjectRefPtrBase::LDSObjectRefPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSObjectRefPtrBase::LDSObjectRefPtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

LDSObjectRefPtr::LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSObjectRefPtrBase(path, flags)
{
}

LDSObjectRefPtr::LDSObjectRefPtr(const LDSObjectRefPtrBase& other)
    : LDSObjectRefPtrBase(other)
{
}
