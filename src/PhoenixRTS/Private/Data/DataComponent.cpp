
#include "Data/DataComponent.h"

using namespace Phoenix::RTS::Data;

bool Component::Read(const LDS::LDSReadObjectContext& context, Component& outItem)
{
    bool success = true;
    return success;
}

ComponentPtr::ComponentPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
