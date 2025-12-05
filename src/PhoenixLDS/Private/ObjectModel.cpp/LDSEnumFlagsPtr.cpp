#include "ObjectModel/LDSEnumFlagsPtr.h"

using namespace Phoenix::LDS;

LDSEnumFlagsPtr::LDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSEnumFlagsPtr::LDSEnumFlagsPtr(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}