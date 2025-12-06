#include "ObjectModel/LDSQueryContext.h"

using namespace Phoenix::LDS;

bool ILDSQueryContext::RecordExists(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
{
    return QueryRecord(path, flags) != nullptr;
}