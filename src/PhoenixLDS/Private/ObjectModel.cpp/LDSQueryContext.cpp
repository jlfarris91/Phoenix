#include "ObjectModel/LDSQueryContext.h"

using namespace Phoenix::LDS;

bool ILDSQueryContext::ObjectRecordExists(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
{
    return QueryObjectRecord(path, flags) != nullptr;
}

bool ILDSQueryContext::TypeRecordExists(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
{
    return QueryTypeRecord(path, flags) != nullptr;
}