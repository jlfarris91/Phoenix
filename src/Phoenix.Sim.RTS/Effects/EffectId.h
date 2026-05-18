#pragma once

#include "Phoenix.Sim.ECS/EntityId.h"

namespace Phoenix::RTS
{
    PHX_ECS_DECLARE_ENTITY_ID_SPEC(EffectNode);
    PHX_ECS_DECLARE_ENTITY_ID_SPEC_WITH_BASE(EffectScope, EffectNode);
}
