
#include "ObjectModel/LDSValueArrayPtr.h"

using namespace Phoenix::LDS; 

LDSValueArrayPtr::LDSValueArrayPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSArrayPtrBase(path, flags)
{
}

LDSValueArrayPtr::LDSValueArrayPtr(const LDSRecordPtr& other)
    : LDSArrayPtrBase(other)
{
}