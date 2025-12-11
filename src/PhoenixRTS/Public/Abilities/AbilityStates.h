#pragma once

#include "DLLExport.h"
#include "Name.h"
#include "EntityId.h"
#include "FixedPoint/FixedTypes.h"
#include "FeatureECS.h"
#include "Units/UnitId.h"

namespace Phoenix::RTS
{
    class FeatureUnit;
}

namespace Phoenix::RTS
{
    enum class EAbilityState : uint32
    {
        Approach = (uint32)"Approach"_n,
        Face = (uint32)"Face"_n
    };

    enum class EAbilityStateResult : uint8
    {
        Continue,
        Fail,
        Complete
    };

    struct MoveToPositionState
    {
        Vec2 Target;
        Distance Range;

        EAbilityStateResult OnEnter(WorldRef world, const UnitId& unit, const Vec2& target, Distance range);
        EAbilityStateResult OnUpdate(WorldRef world, const UnitId& unit);
        void OnInterrupt(WorldRef world, const UnitId& unit);
        void OnExit(WorldRef world, const UnitId& unit);
    };
}
