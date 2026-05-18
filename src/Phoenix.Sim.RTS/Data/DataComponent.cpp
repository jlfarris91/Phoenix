
#include "Phoenix.Sim.RTS/Data/DataComponent.h"

using namespace Phoenix::RTS::Data;

bool Component::Read(const LDS::LDSReadObjectArgs& args, Component& outItem)
{
    bool success = true;
    return success;
}

ComponentPtr::ComponentPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::NamePtr ComponentPtr::TypeId() const
{
    return Value<LDS::NamePtr>("type_id");
}
