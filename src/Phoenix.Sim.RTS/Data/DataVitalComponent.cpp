
#include "Phoenix.Sim.RTS/Data/DataVitalComponent.h"

using namespace Phoenix::RTS::Data;

bool VitalComponent::Read(const LDS::LDSReadObjectArgs& args, VitalComponent& outItem)
{
    bool success = true;
    return success;
}

VitalComponentPtr::VitalComponentPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : ComponentPtr(path, flags)
{
}
