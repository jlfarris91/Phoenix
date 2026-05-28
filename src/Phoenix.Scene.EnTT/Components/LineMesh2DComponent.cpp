#include "LineMesh2DComponent.h"

#include "Scene.h"
#include "SceneComponent.h"
#include "SceneComponentHandler.h"
#include "Proxies/LineMesh2DProxy.h"

void Phoenix::EnTT::LineMesh2DComponent::OnConstruct(Scene& scene, entt::entity)
{
    auto& proxyManager = scene.GetProxyManager();
    ProxyHandle = proxyManager.EmplaceProxy<Renderer::LineMesh2DProxy>();
}

void Phoenix::EnTT::LineMesh2DComponent::OnUpdate(Scene& scene, entt::entity entity)
{
    glm::mat3 transform;

    if (auto* sceneComp = scene.GetRegistry().try_get<SceneComponent>(entity))
    {
        transform = sceneComp->WorldTransform;
    }

    auto& proxyManager = scene.GetProxyManager();
    proxyManager.UpdateProxy<Renderer::LineMesh2DProxy>(ProxyHandle, [this, transform](Renderer::LineMesh2DProxy& proxy)
    {
        proxy.Layer = Layer;
        proxy.Tint = Tint;
        proxy.Mesh = Mesh;
        proxy.Texture = Texture;
        proxy.Transform = transform;
    });
}

void Phoenix::EnTT::LineMesh2DComponent::OnDestroy(Scene& scene, entt::entity)
{
    auto& proxyManager = scene.GetProxyManager();
    proxyManager.DestroyProxy(ProxyHandle);
}
