#include "Phoenix.Sim/LDS/ObjectModel/LDSRecordPath.h"

using namespace Phoenix::LDS;

LDSRecordPath::LDSRecordPath(const FName& objectId, const FName& path)
    : ObjectId(objectId)
    , Path(path)
{
}

bool LDSRecordPath::IsValid() const
{
    return !FName::IsNoneOrEmpty(ObjectId);
}

LDSRecordPath LDSRecordPath::Append(const char* str, size_t len) const
{
    LDSRecordPath result = *this;
 
    if (str[0] != '/')
    {
        result.Path = result.Path.Append('/');
    }

    result.Path = result.Path.Append(str, len);
    return result;
}

LDSRecordPath LDSRecordPath::Append(uint32 index) const
{
    LDSRecordPath result = *this;
    char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "/%u", index);
    result.Path = result.Path.Append(buffer, len);
    return result;
}
