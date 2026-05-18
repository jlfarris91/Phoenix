
#include "Phoenix.Sim.RTS/Data/DataCommandButton.h"

using namespace Phoenix::RTS::Data;

bool CommandButton::Read(const LDS::LDSReadObjectArgs& args, CommandButton& outItem)
{
    bool success = true;
    return success;
}

CommandButtonPtr::CommandButtonPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

IconPtr CommandButtonPtr::Icon() const
{
    return Object<IconPtr>("icon");
}

Phoenix::LDS::TLDSValuePtr<bool> CommandButtonPtr::IsRepeatable() const
{
    return Value<bool>("repeatable");
}
