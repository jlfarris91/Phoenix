
#include "Phoenix.Sim.RTS/Data/DataVital.h"

using namespace Phoenix::RTS::Data;

bool Vital::Read(const LDS::LDSReadObjectArgs& args, Vital& outItem)
{
    bool success = true;
    return success;
}

VitalPtr::VitalPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

VitalComponentRefPtr VitalPtr::Component() const
{
    return ObjectRef<VitalComponentRefPtr>("component");
}
