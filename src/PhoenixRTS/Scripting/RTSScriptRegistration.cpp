// RTSScriptRegistration.cpp
//
// Registers PhoenixRTS APIs (Unit, Orders, Vitals) with the ScriptTypeRegistry
// so that any IScriptRuntime (Lua, etc.) can expose them without needing to
// include RTS headers in the scripting bridge layer.
//
// All registered functions use plain-C++ types covered by GenericConverter<T>:
//   ECS::EntityId, FName, uint8/32, float, bool
// RTS-specific types (UnitId, Command, Damage) are adapted in local wrappers.

#include "PhoenixSim/Scripting/ScriptRegistrationBuilder.h"

#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixRTS/Vitals/Damage.h"
#include "PhoenixRTS/Vitals/FeatureVitals.h"
#include "PhoenixRTS/Vitals/VitalComponents.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

// ── Local wrappers ─────────────────────────────────────────────────────────────
//
// These adapt RTS-specific types to GenericConverter-compatible types.
// SpawnUnitArgs default args are fine — scripts get the default behaviour.

namespace
{
    // Unit

    ECS::EntityId Script_Unit_Spawn(WorldRef world, FName unitData,
                                    uint32 owner, float x, float y, float facing)
    {
        return static_cast<ECS::EntityId>(
            FeatureUnit::SpawnUnit(world, unitData, static_cast<uint8>(owner),
                                   Vec2(Distance(x), Distance(y)), Angle(facing)));
    }

    bool Script_Unit_IsAlive(WorldConstRef world, ECS::EntityId entityId)
    {
        return FeatureUnit::UnitIsAlive(world, UnitId(entityId));
    }

    uint32 Script_Unit_GetOwner(WorldConstRef world, ECS::EntityId entityId)
    {
        return FeatureUnit::GetOwningPlayer(world, UnitId(entityId));
    }

    FName Script_Unit_GetUnitData(WorldConstRef world, ECS::EntityId entityId)
    {
        return FeatureUnit::GetUnitDataId(world, UnitId(entityId));
    }

    // Orders

    bool Script_Orders_IssueCommand(WorldRef world, ECS::EntityId entityId,
                                    FName commandId, float targetX, float targetY,
                                    uint32 sender)
    {
        Command cmd;
        cmd.CommandId      = commandId;
        cmd.TargetLocation = Vec2(Distance(targetX), Distance(targetY));
        cmd.Sender         = sender;
        return FeatureOrders::StaticIssueCommand(world, UnitId(entityId), cmd);
    }

    bool Script_Orders_HasOrders(WorldConstRef world, ECS::EntityId entityId)
    {
        return FeatureOrders::HasOrders(world, UnitId(entityId));
    }

    // Vitals

    bool Script_Vitals_ApplyDamage(WorldRef world, ECS::EntityId entityId, float amount)
    {
        Damage damage;
        damage.Amount     = Value(amount);
        damage.BaseAmount = damage.Amount;
        return FeatureVitals::ApplyDamage(world, entityId, damage);
    }

    float Script_Vitals_GetHealth(WorldConstRef world, ECS::EntityId entityId)
    {
        const auto* hc = ECS::FeatureECS::GetComponent<HealthComponent>(world, entityId);
        return hc ? static_cast<float>(hc->Health.Current) : 0.f;
    }

    float Script_Vitals_GetMaxHealth(WorldConstRef world, ECS::EntityId entityId)
    {
        const auto* hc = ECS::FeatureECS::GetComponent<HealthComponent>(world, entityId);
        return hc ? static_cast<float>(hc->Health.Max) : 0.f;
    }
} // anonymous namespace

// ── Force-link symbol ─────────────────────────────────────────────────────────
//
// Referenced from app startup to prevent MSVC from dead-stripping this TU.

namespace Phoenix::RTS
{
    void EnsureScriptRegistrations() {}
}

// ── PHX_SCRIPT_REGISTRATION ────────────────────────────────────────────────────

PHX_SCRIPT_REGISTRATION(FeatureUnit)
{
    registration
        .namespace_("Phoenix.Unit")
        .world_function("SpawnUnit",   Script_Unit_Spawn)
        .world_function("IsAlive",     Script_Unit_IsAlive)
        .world_function("GetOwner",    Script_Unit_GetOwner)
        .world_function("GetUnitData", Script_Unit_GetUnitData);
}

PHX_SCRIPT_REGISTRATION(FeatureOrders)
{
    registration
        .namespace_("Phoenix.Orders")
        .world_function("IssueCommand", Script_Orders_IssueCommand)
        .world_function("HasOrders",    Script_Orders_HasOrders);
}

PHX_SCRIPT_REGISTRATION(FeatureVitals)
{
    registration
        .namespace_("Phoenix.Vitals")
        .world_function("ApplyDamage",  Script_Vitals_ApplyDamage)
        .world_function("GetHealth",    Script_Vitals_GetHealth)
        .world_function("GetMaxHealth", Script_Vitals_GetMaxHealth);
}
