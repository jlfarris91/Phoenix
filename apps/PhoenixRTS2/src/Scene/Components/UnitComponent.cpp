#include "UnitComponent.h"

#include "Phoenix.Sim.RTS/Units/UnitComponent.h"

#include "Scene.h"
#include "SceneComponentHandler.h"
#include "Components/LineMesh2DComponent.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"

using namespace Phoenix;
using namespace Phoenix::App::Dev;

glm::vec4 gPlayerColors[] =
{
    { 1.000, 0.008, 0.008, 1.0f },
    { 0.000, 0.255, 1.000, 1.0f },
    { 0.000, 0.765, 1.000, 1.0f },
    { 0.569, 0.000, 1.000, 1.0f },
    { 1.000, 1.000, 0.000, 1.0f },
    { 0.996, 0.537, 0.051, 1.0f },
    { 0.122, 0.749, 0.000, 1.0f },
    { 0.894, 0.353, 0.667, 1.0f },
    { 0.580, 0.584, 0.588, 1.0f },
    { 0.490, 0.745, 0.945, 1.0f },
    { 0.059, 0.380, 0.271, 1.0f },
    { 0.302, 0.161, 0.012, 1.0f }
};

void UnitComponent::OnConstruct(App::Dev::Scene& scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    (void)registry.get_or_emplace<LineMesh2DComponent>(entity);
}

void UnitComponent::OnDestroy(App::Dev::Scene& scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    if (registry.any_of<LineMesh2DComponent>(entity))
    {
        registry.erase<LineMesh2DComponent>(entity);
    }
}

void UnitComponent::OnSync(const SceneComponentSyncArgs& args)
{
    auto simComp = static_cast<const RTS::UnitComponent*>(args.SimComponentData);

    OwningPlayer = simComp->OwningPlayer;

    Renderer::TResourcePtr<LineMesh2D> mesh = Mesh;
    glm::vec4 tint = Tint;
    float scale = Scale;

    if (simComp->UnitData != UnitData)
    {
        UnitData = simComp->UnitData;

        const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(*args.World);

        RTS::Data::UnitPtr unitData(simComp->UnitData);
        RTS::Data::UnitActorPtr unitActorData = unitData.Actor().ResolveObject(lds);

        Mesh = mesh = unitActorData.Asset().GetValue(lds);
        Scale = scale = (float)unitActorData.Scale().GetValue(lds);

        auto tintSim = unitActorData.Tint().GetValue(lds);
        Tint = tint = glm::vec4 {
            tintSim.R / 255.0f,
            tintSim.G / 255.0f,
            tintSim.B / 255.0f,
            tintSim.A / 255.0f
        };
    }

    tint *= gPlayerColors[OwningPlayer];

    bIsAlive = RTS::FeatureUnit::UnitIsAlive(*args.World, RTS::UnitId(args.SimEntity));
    if (!bIsAlive)
    {
        mesh = Renderer::TResourcePtr<LineMesh2D>(FName("Units/Corpse.json"));
        tint *= 0.5f;
    }

    auto& meshComp = args.Scene->GetRegistry().get_or_emplace<LineMesh2DComponent>(args.SceneEntity);
    meshComp.Mesh = mesh;
    meshComp.Tint = tint;
    meshComp.Scale = scale;
}