#include "Phoenix.Sim.LDS/ObjectModel/LDSEnumFlagsPtr.h"

using namespace Phoenix::LDS;

LDSEnumFlagsPtrBase::LDSEnumFlagsPtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSEnumFlagsPtrBase::LDSEnumFlagsPtrBase(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

LDSEnumFlagsPtr::LDSEnumFlagsPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSEnumFlagsPtrBase(path, flags)
{
}

LDSEnumFlagsPtr::LDSEnumFlagsPtr(const LDSEnumFlagsPtrBase& other)
    : LDSEnumFlagsPtrBase(other)
{
}