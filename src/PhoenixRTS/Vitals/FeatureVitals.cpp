#include "PhoenixRTS/Vitals/FeatureVitals.h"

#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/LDS/LDSQueryContext.h"

#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Data/DataVital.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixRTS/Vitals/Damage.h"
#include "PhoenixRTS/Vitals/VitalComponents.h"
#include "PhoenixRTS/Vitals/VitalsSystem.h"
#include "PhoenixSim/Logging.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;

bool FeatureVitals::ApplyDamage(WorldRef world, EntityId target, const Damage& damage)
{
    const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);

    if (FeatureUnit::UnitIsDead(world, UnitId(target)))
    {
        return false;
    }

    Data::VitalPtr vitalPtr(damage.VitalId);
    Data::VitalComponentPtr vitalComponentPtr = vitalPtr.Component().ResolveObject(lds);
    if (!vitalComponentPtr.Exists(lds))
    {
        return false;
    }

    FName vitalComponentTypeId = vitalComponentPtr.TypeId().GetValue(lds);
    IComponent* vitalComponent = FeatureECS::GetComponent(world, target, vitalComponentTypeId);
    if (!vitalComponent)
    {
        return false;
    }

    // TODO (jfarris): remove hard-coded cast to health component
    HealthComponent* healthComponent = static_cast<HealthComponent*>(vitalComponent);

    Value amount = Max(damage.Amount, 0);
    Value health = healthComponent->Health.Current;

    // TODO (jfarris): this should move to some sort of Vitals handler
    if (amount >= health)
    {
        FeatureUnit::OnUnitKilled(world, UnitId(target), damage.SourceId);
    }

    healthComponent->Health.Current -= amount;
    healthComponent->Health.Current = Max(healthComponent->Health.Current, 0);

    LogVerbose("Unit {0} dealt {1} damage to unit {2}. New health is {3}", (uint32)damage.SourceId, (uint32)amount, (uint32)target, (double)healthComponent->Health.Current);

    return true;
}

void FeatureVitals::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    VitalsSystem = MakeShared<RTS::VitalsSystem>();

    TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>();
    featureECS->RegisterSystem(VitalsSystem);
}

void FeatureVitals::Shutdown()
{
    IFeature::Shutdown();

    if (TSharedPtr<FeatureECS> featureECS = Session->GetFeatureSet()->GetFeature<FeatureECS>())
    {
        featureECS->UnregisterSystem(VitalsSystem);
    }

    VitalsSystem.reset();
}
