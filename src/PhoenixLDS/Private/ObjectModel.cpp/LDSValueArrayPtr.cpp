
#include "ObjectModel/LDSValueArrayPtr.h"

using namespace Phoenix::LDS;

LDSValueArrayPtrBase::LDSValueArrayPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtrBase(path, flags)
{
}

LDSValueArrayPtrBase::LDSValueArrayPtrBase(const LDSRecordPtr& other)
    : LDSArrayPtrBase(other)
{
}

LDSValueArrayPtr::LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSValueArrayPtrBase(path, flags)
{
}

LDSValueArrayPtr::LDSValueArrayPtr(const LDSValueArrayPtrBase& other)
    : LDSValueArrayPtrBase(other)
{
}
