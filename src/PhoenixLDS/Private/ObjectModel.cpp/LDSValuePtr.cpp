
#include "ObjectModel/LDSValuePtr.h"

using namespace Phoenix::LDS;

LDSValuePtr::LDSValuePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSValuePtr::LDSValuePtr(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}
