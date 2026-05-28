#include "SceneProxyManager.h"

#include <ranges>

using namespace Phoenix::Renderer;

SceneProxyHandle SceneProxyManager::CreateProxy(std::unique_ptr<ISceneProxy>&& proxy)
{
    auto handle = SceneProxyHandle{ ++ProxyIdGen };
    Proxies.emplace(handle.Id, std::move(proxy));
    return handle;
}

bool SceneProxyManager::UpdateProxy(SceneProxyHandle proxy, std::function<void(ISceneProxy&)>&& func)
{
    auto iter = Proxies.find(proxy.Id);
    if (iter == Proxies.end())
    {
        return false;
    }
    func(*iter->second);
    return true;
}

bool SceneProxyManager::DestroyProxy(SceneProxyHandle proxy)
{
    auto iter = Proxies.find(proxy.Id);
    if (iter == Proxies.end())
    {
        return false;
    }
    Proxies.erase(iter);
    return true;
}

void SceneProxyManager::DestroyAllProxies()
{
    Proxies.clear();
    ProxyIdGen = 0;
}

size_t SceneProxyManager::GetNumProxies() const
{
    return Proxies.size();
}

size_t SceneProxyManager::GetAllProxies(std::vector<const ISceneProxy*>& outProxies) const
{
    outProxies.clear();
    outProxies.reserve(Proxies.size());
    size_t count = 0;
    for (auto&& proxy : Proxies | std::views::values)
    {
        outProxies.push_back(proxy.get());
        ++count;
    }
    return count;
}

void SceneProxyManager::Gather(RenderScene& scene) const
{
    for (auto&& proxy : Proxies | std::views::values)
    {
        proxy->Gather(scene);
    }
}
