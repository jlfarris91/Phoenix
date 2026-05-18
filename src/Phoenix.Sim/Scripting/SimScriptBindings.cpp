#include "Phoenix.Sim/Scripting/SimScriptBindings.h"

#include "Phoenix.Sim/Blackboard/FeatureBlackboard.h"
#include "Phoenix.Sim/ECS/FeatureECS.h"
#include "Phoenix.Sim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

// ── Glue functions ────────────────────────────────────────────────────────────
//
// World is injected by the WASM trampoline — not passed from Lua.

namespace
{
    // ── World ─────────────────────────────────────────────────────────────────

    FName PhxSim_World_GetId   (WorldConstRef w) { return w.GetId();   }
    FName PhxSim_World_GetType (WorldConstRef w) { return w.GetType(); }

    // ── Random ────────────────────────────────────────────────────────────────

    void PhxSim_Random_Seed(WorldRef w, uint64 seed) { w.GetRandom().Seed(seed); }

    template<class T>
    T PhxSim_Random_Next(WorldRef w) { return w.GetRandom().Next<T>(); }

    template<class T>
    T PhxSim_Random_Range(WorldRef w, const T& min, const T& max) { return w.GetRandom().Range<T>(min, max); }

    Vec2 PhxSim_Random_PointInCircle(WorldRef w, Distance radius) { return w.GetRandom().PointInCircle(radius); }
    Vec2 PhxSim_Random_PointOnCircle(WorldRef w, Distance radius) { return w.GetRandom().PointOnCircle(radius); }

    // ── World blackboard ──────────────────────────────────────────────────────

    template<class T>
    ScriptOptional<T> PhxSim_TryGetBlackboardValue(WorldConstRef world, Blackboard::blackboard_key_t key)
    {
        const auto& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
        T value;
        return blackboard.GetValue<T>(key, value)
            ? ScriptOptional<T>::Some(value)
            : ScriptOptional<T>::None();
    }

    template<class T>
    T PhxSim_GetBlackboardValue(WorldConstRef world, Blackboard::blackboard_key_t key, const T& defaultValue)
    {
        const auto& blackboard = Blackboard::FeatureBlackboard::GetBlackboard(world);
        T value;
        return blackboard.GetValue<T>(key, value) ? value : defaultValue;
    }

    template<class T>
    bool PhxSim_SetBlackboardValue(WorldRef world, Blackboard::blackboard_key_t key, const T& value)
    {
        return Blackboard::FeatureBlackboard::GetBlackboard(world).SetValue<T>(key, value);
    }

    bool PhxSim_HasBlackboardValue(WorldConstRef world, Blackboard::blackboard_key_t key)
    {
        return Blackboard::FeatureBlackboard::GetBlackboard(world).HasValue(key);
    }

    // ── Entity blackboard (optional variant) ──────────────────────────────────

    template<class T>
    ScriptOptional<T> PhxECS_TryGetBlackboardValue(WorldConstRef world, const EntityId& id, const FName& key)
    {
        T value;
        return FeatureECS::TryGetBlackboardValue<T>(world, id, key, value)
            ? ScriptOptional<T>::Some(value)
            : ScriptOptional<T>::None();
    }
}

// ── Registration ──────────────────────────────────────────────────────────────

PHX_DEFINE_TYPE(Phoenix::SimScriptBindings)
{
    registration.Bases<IScriptBindings>();
}

void SimScriptBindings::Describe(ScriptModuleBuilder& builder) const
{
    constexpr TypePack<
        bool,
        int32, uint32,
        int64, uint64,
        Value, /* Distance, */ Angle, Time, Speed> RandomTypes;

    constexpr TypePack<
        bool,
        int32, uint32,
        int64, uint64,
        Value, /* Distance, */ Angle, Time, Speed, Vec2,
        FName, EntityId, Color> BlackboardTypes;

    builder
        .Namespace("Phoenix.World")
        .Function("GetId(world)",                                       &PhxSim_World_GetId)
        .Function("GetType(world)",                                     &PhxSim_World_GetType)
        .Function("HasBlackboardValue(world, key)",                     &PhxSim_HasBlackboardValue)
        .TEMPLATE_FUNCTION("TryGetBlackboardValue(world, key)",                   &PhxSim_TryGetBlackboardValue, BlackboardTypes)
        .TEMPLATE_FUNCTION("GetBlackboardValue(world, key, defaultValue)",      &PhxSim_GetBlackboardValue, BlackboardTypes)
        .TEMPLATE_FUNCTION("SetBlackboardValue(world, key, value)",             &PhxSim_SetBlackboardValue, BlackboardTypes);

    builder
        .Namespace("Phoenix.Random")
        .Function("Seed(world, seed)",              &PhxSim_Random_Seed)
        .TEMPLATE_FUNCTION("Next(world)",                   &PhxSim_Random_Next, RandomTypes)
        .TEMPLATE_FUNCTION("Range(world, min, max)",        &PhxSim_Random_Range, RandomTypes)
        .Function("PointInCircle(world, radius)",   &PhxSim_Random_PointInCircle)
        .Function("PointOnCircle(world, radius)",   &PhxSim_Random_PointOnCircle);

    builder
        .Class<EntityId>("Phoenix.Entity")
        .Method("IsValid(world, entity)",                                   &FeatureECS::IsEntityValid)
        .Method("HasTag(world, entity, tag)",                               &FeatureECS::HasTag)
        .Method("AddTag(world, entity, tag)",                               &FeatureECS::AddTag)
        .Method("RemoveTag(world, entity, tag)",                            &FeatureECS::RemoveTag)
        .Method("GetPosition(world, entity)",                               &FeatureECS::GetWorldPosition)
        .Method("GetFacing(world, entity)",                                 &FeatureECS::GetWorldFacing)
        .TEMPLATE_METHOD("GetBlackboardValue(world, entity, key, defaultValue)",    &FeatureECS::GetBlackboardValue, BlackboardTypes)
        .TEMPLATE_METHOD("SetBlackboardValue(world, entity, key, value)",           &FeatureECS::SetBlackboardValue, BlackboardTypes)
        .TEMPLATE_METHOD("TryGetBlackboardValue",                                   &PhxECS_TryGetBlackboardValue, BlackboardTypes);
}
