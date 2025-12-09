
#include "ObjectModel/LDSValuePtr.h"

using namespace Phoenix::LDS;

LDSValuePtrBase::LDSValuePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSValuePtrBase::LDSValuePtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

LDSValuePtr::LDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSValuePtrBase(path, flags)
{
}

LDSValuePtr::LDSValuePtr(const LDSValuePtrBase& other)
    : LDSValuePtrBase(other)
{
}
