#pragma once

#include "Phoenix.Sim/ECS/EntityId.h"
#include "Phoenix.Sim/Features.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataVital.h"
#include "Phoenix.Sim.RTS/Vitals/Damage.h"
#include "Phoenix.Sim.RTS/Vitals/Vitals.h"

namespace Phoenix::RTS
{
    struct Damage;

    class PHOENIX_RTS_API FeatureVitals : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureVitals) {}

    public:

        // Adds a new vital to the specified entity with the given initial value.
        // If the entity already has that vital, a pointer to the existing vital is returned.
        static Vital* AddVital(WorldRef world, ECS::EntityId entity, FName vitalId, Vital initialValue = {});

        // Removes the specified vital from the entity.
        // Returns true if the vital was successfully removed, false if the entity doesn't have that vital.
        static bool RemoveVital(WorldRef world, ECS::EntityId entity, FName vitalId);

        static Vital* GetVital(WorldRef world, ECS::EntityId entity, FName vitalId);
        static const Vital* GetVital(WorldConstRef world, ECS::EntityId entity, FName vitalId);

        static bool ApplyDamage(WorldRef world, ECS::EntityId target, const Damage& damage);
    };
}


PHX_DEFINE_TYPE(Phoenix::RTS::FeatureVitals)
{
    registration
        .Namespace("Phoenix.Vitals")
        .StaticMethod("ApplyDamage(world, target, damage)", &RTS::FeatureVitals::ApplyDamage);
}