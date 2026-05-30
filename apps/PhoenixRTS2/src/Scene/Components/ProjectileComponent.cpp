#include "ProjectileComponent.h"

#include "Phoenix.Sim.RTS/Projectiles/ProjectileComponent.h"

#include "Scene.h"
#include "SceneComponentHandler.h"
#include "Components/Circle2DComponent.h"
#include "Components/LineMesh2DComponent.h"

#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.RTS/Data/DataUnit.h"

using namespace Phoenix;
using namespace Phoenix::App::Dev;

void ProjectileComponent::OnConstruct(App::Dev::Scene &scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    (void)registry.get_or_emplace<LineMesh2DComponent>(entity);
}

void ProjectileComponent::OnDestroy(App::Dev::Scene &scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    if (registry.any_of<LineMesh2DComponent>(entity))
    {
        registry.erase<LineMesh2DComponent>(entity);
    }
}

void ProjectileComponent::OnSync(const SceneComponentSyncArgs& args)
{
    auto simComp = static_cast<const RTS::ProjectileComponent*>(args.SimComponentData);

    if (ProjectileData != simComp->ProjectileDataId)
    {
        ProjectileData = simComp->ProjectileDataId;

        const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(*args.World);

        RTS::Data::ProjectilePtr projectileData(ProjectileData);
        RTS::Data::ProjectileActorPtr projectileActorData = projectileData.Actor().ResolveObject(lds);

        Mesh = projectileActorData.Asset().GetValue(lds);
        Scale = (float)projectileActorData.Scale().GetValue(lds);

        auto tintSim = projectileActorData.Tint().GetValue(lds);
        Tint = glm::vec4 {
            tintSim.R / 255.0f,
            tintSim.G / 255.0f,
            tintSim.B / 255.0f,
            tintSim.A / 255.0f
        };
    }

    auto& meshComp = args.Scene->GetRegistry().get_or_emplace<LineMesh2DComponent>(args.SceneEntity);
    meshComp.Mesh = Mesh;
    meshComp.Tint = Tint;
    meshComp.Scale = Scale;
}