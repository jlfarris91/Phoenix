#include "UnitComponent.h"

#include "Phoenix.Sim.RTS/Units/UnitComponent.h"

#include "Scene.h"
#include "SceneComponentHandler.h"
#include "Components/LineMesh2DComponent.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::App::Dev;

void UnitComponent::OnSpawn(const SceneComponentSyncArgs &args)
{
}

void UnitComponent::OnUpdate(const SceneComponentSyncArgs& args)
{
    auto simComp = static_cast<const RTS::UnitComponent*>(args.SimComponentData);

    OwningPlayer = simComp->OwningPlayer;

    if (simComp->UnitData != UnitData)
    {
        UnitData = simComp->UnitData;

        // const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(*args.World);
        //
        // RTS::Data::UnitPtr unitData(simComp->UnitData);
        // RTS::Data::UnitActorPtr unitActorData = unitData.Actor().ResolveObject(lds);
        //
        // auto& meshComp = args.Scene->GetRegistry().get_or_emplace<EnTT::LineMesh2DComponent>(args.SceneEntity);
        // meshComp.Asset = unitActorData.Asset().GetValue(lds);
        // meshComp.Scale = (float)unitActorData.Scale().GetValue(lds);
    }
}

void UnitComponent::OnDestroy(const SceneComponentSyncArgs &args)
{
    auto& registry = args.Scene->GetRegistry();
    if (registry.any_of<LineMesh2DComponent>(args.SceneEntity))
    {
        registry.erase<LineMesh2DComponent>(args.SceneEntity);
    }
}
