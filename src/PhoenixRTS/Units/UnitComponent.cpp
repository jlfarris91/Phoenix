#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

PHX_DEFINE_TYPE(UnitComponent)
{
    registration
        .Field("OwningPlayer", &UnitComponent::OwningPlayer)
        .Field("UnitData",     &UnitComponent::UnitData);
}
