#include "PhoenixRTS/Units/UnitComponent.h"
#include "PhoenixSim/Reflection/TypeRegistrationBuilder.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

PHX_TYPE_REGISTRATION(UnitComponent)
{
    registration
        .Field("OwningPlayer", &UnitComponent::OwningPlayer)
        .Field("UnitData",     &UnitComponent::UnitData);
}
