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
    auto& circleComp = registry.get_or_emplace<Circle2DComponent>(entity);
    circleComp.Color = Color4b::White();
    circleComp.Radius = 0.5f;
}

void ProjectileComponent::OnDestroy(App::Dev::Scene &scene, entt::entity entity)
{
    auto& registry = scene.GetRegistry();
    if (registry.any_of<Circle2DComponent>(entity))
    {
        registry.erase<Circle2DComponent>(entity);
    }
}

void ProjectileComponent::OnSync(const SceneComponentSyncArgs& args)
{
}