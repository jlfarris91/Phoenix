#include "VitalsTask.h"

#include "Damage.h"
#include "FeatureVitals.h"
#include "VitalComponent.h"
#include "PhoenixSim/ECS/FeatureECS.h"

void Phoenix::RTS::VitalsTask::OnCreate(WorldRef world, uint32)
{
    SetInterval(world, 1.0);
}

void Phoenix::RTS::VitalsTask::OnUpdate(WorldRef world, uint32 context)
{
    ECS::EntityId entity = ECS::EntityId(context);

    VitalsComponent* comp = ECS::FeatureECS::GetComponent<VitalsComponent>(world, context);
    if (!comp || comp->VitalCount == 0)
    {
        return;
    }

    for (uint8 i = 0; i < comp->VitalCount; ++i)
    {
        VitalsComponent::VitalEntry& entry = comp->Vitals[i];
        Vital& vital = entry.Vital;

        if (entry.Id == FName::None)
        {
            continue;
        }

        if (vital.Regen < 0.0)
        {
            Damage damage;
            damage.VitalId = entry.Id;
            damage.SourceId = entity;
            damage.Amount = -vital.Regen;
            damage.BaseAmount = damage.Amount;
            damage.ArmorMultiplier = 0;
            FeatureVitals::ApplyDamage(world, entity, damage);
        }
        else if (vital.Regen > 0.0)
        {
            vital.Current += vital.Regen;
            if (vital.Current >= vital.Max)
            {
                vital.Current = vital.Max;
            }
        }
    }
}
