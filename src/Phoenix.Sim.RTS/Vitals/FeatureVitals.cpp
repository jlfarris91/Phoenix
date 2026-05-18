#include "Phoenix.Sim.RTS/Vitals/FeatureVitals.h"

#include "VitalsTask.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix/Logging.h"
#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.RTS/Vitals/Damage.h"
#include "Phoenix.Sim.RTS/Vitals/VitalComponent.h"
#include "Phoenix.Sim/Tasks/FeatureTask.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

Vital* FeatureVitals::AddVital(WorldRef world, EntityId entity, FName vitalId, Vital initialValue)
{
    VitalsComponent* comp = FeatureECS::GetOrAddComponent<VitalsComponent>(world, entity);
    if (!comp)
    {
        return nullptr;
    }

    // If a vital already exists, return a pointer to it.
    for (auto& entry : comp->Vitals)
    {
        if (entry.Id == vitalId)
        {
            return &entry.Vital;
        }
    }

    // Is there room for any more vitals?
    if (comp->VitalCount == _countof(VitalsComponent::Vitals))
    {
        LogWarning("Trying to add new vital '{0}' to {1} but it already has the maximum number of vitals", FName::GetNameEntry(vitalId), (uint32)entity);
        return nullptr;
    }

    VitalsComponent::VitalEntry& entry = comp->Vitals[comp->VitalCount++];
    entry.Id = vitalId;
    entry.Vital = initialValue;

    // If this was the first vital added, then create a task to update it.
    if (comp->VitalCount == 1)
    {
        Tasks::FeatureTask::Allocate<VitalsTask>(world, entity);
    }

    return &entry.Vital;
}

bool FeatureVitals::RemoveVital(WorldRef world, EntityId entity, FName vitalId)
{
    VitalsComponent* comp = FeatureECS::GetComponent<VitalsComponent>(world, entity);
    if (!comp || comp->VitalCount == 0)
    {
        return false;
    }

    // Try to find the vital being removed.
    uint8 index;
    for (index = 0; index < comp->VitalCount; ++index)
    {
        if (comp->Vitals[index].Id == vitalId)
        {
            break;
        }
    }

    // Couldn't find the vital.
    if (index == comp->VitalCount)
    {
        return false;
    }

    // Remove the vital by shifting down all vitals to fill the space.
    for (uint8 i = index; i < comp->VitalCount - 1; ++i)
    {
        comp->Vitals[i] = comp->Vitals[i + 1];
    }

    --comp->VitalCount;

    // If this was the last vital to be removed, then remove the task that was updating vitals.
    if (comp->VitalCount == 0)
    {
        Tasks::FeatureTask::FinishTask<VitalsTask>(world, entity);
    }

    return true;
}

Vital* FeatureVitals::GetVital(WorldRef world, EntityId entity, FName vitalId)
{
    if (VitalsComponent* comp = FeatureECS::GetComponent<VitalsComponent>(world, entity))
    {
        for (VitalsComponent::VitalEntry& vitalEntry : comp->Vitals)
        {
            if (vitalEntry.Id == vitalId)
            {
                return &vitalEntry.Vital;
            }
        }
    }
    return nullptr;
}

const Vital* FeatureVitals::GetVital(WorldConstRef world, EntityId entity, FName vitalId)
{
    if (const VitalsComponent* comp = FeatureECS::GetComponent<VitalsComponent>(world, entity))
    {
        for (const VitalsComponent::VitalEntry& vitalEntry : comp->Vitals)
        {
            if (vitalEntry.Id == vitalId)
            {
                return &vitalEntry.Vital;
            }
        }
    }
    return nullptr;
}

bool FeatureVitals::ApplyDamage(WorldRef world, EntityId target, const Damage& damage)
{
    if (FeatureUnit::UnitIsDead(world, UnitId(target)))
    {
        return false;
    }

    Vital* vital = GetVital(world, target, damage.VitalId);
    if (!vital)
    {
        return false;
    }

    Value amount = Max(damage.Amount, 0);
    Value current = vital->Current;

    // TODO (jfarris): this should move to some sort of Vitals handler
    if (amount >= current)
    {
        FeatureUnit::OnUnitKilled(world, UnitId(target), damage.SourceId);
    }

    vital->Current -= amount;
    vital->Current = Max(vital->Current, 0);

    LogVerbose("Unit {0} dealt {1} damage to unit {2}. New health is {3}", (uint32)damage.SourceId, (uint32)amount, (uint32)target, (double)vital->Current);

    return true;
}