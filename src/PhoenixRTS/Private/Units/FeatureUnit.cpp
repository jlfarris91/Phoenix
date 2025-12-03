
#include "Units/FeatureUnit.h"

#include "FeatureECS.h"

using namespace Phoenix;

RTS::Unit RTS::FeatureUnit::SpawnUnit(
    WorldRef world,
    const FName& unitData,
    uint32 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    return {};
}

uint32 RTS::FeatureUnit::SpawnUnits(
    WorldRef world,
    uint32 num,
    const FName& unitData,
    uint32 owner,
    const Vec2& pos,
    Angle facing,
    const SpawnUnitArgs& args)
{
    return 0;
}

Value RTS::FeatureUnit::GetHealth(WorldConstRef world, Unit unit)
{
    return 0.0;
}
