#include "UnitComponent.h"

#include "Phoenix.Sim.RTS/Units/UnitComponent.h"

#include "Scene.h"
#include "SceneComponentHandler.h"
#include "Components/Circle2DComponent.h"
#include "Components/LineMesh2DComponent.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::App::Dev;

void UnitComponent::OnConstruct(App::Dev::Scene &scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    auto& circleComp = registry.get_or_emplace<Circle2DComponent>(entity);
    circleComp.Color = Color4b::White();
    circleComp.Radius = 0.5f;
}

void UnitComponent::OnDestroy(App::Dev::Scene &scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    if (registry.any_of<Circle2DComponent>(entity))
    {
        registry.erase<Circle2DComponent>(entity);
    }
}

void UnitComponent::OnSync(const SceneComponentSyncArgs& args)
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