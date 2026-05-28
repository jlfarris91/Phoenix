#pragma once

#include "SceneProxy.h"
#include "Application/AppService.h"

namespace Phoenix::Renderer
{
    struct SceneProxyHandle
    {
        uint64_t Id = 0;
    };

    class ISceneProxyManager
    {
        PHX_DECLARE_TYPE_INTERFACE(ISceneProxyManager)
    public:
        virtual ~ISceneProxyManager() {}
        virtual SceneProxyHandle CreateProxy(std::unique_ptr<ISceneProxy>&& proxy) = 0;
        virtual bool UpdateProxy(SceneProxyHandle proxy, std::function<void(ISceneProxy&)>&& func) = 0;
        virtual bool DestroyProxy(SceneProxyHandle proxy) = 0;
        virtual void DestroyAllProxies() = 0;
        virtual size_t GetNumProxies() const = 0;
        virtual size_t GetAllProxies(std::vector<const ISceneProxy*>& outProxies) const = 0;
        virtual void Gather(RenderScene& scene) const = 0;

        template <class T, class... TArgs>
        SceneProxyHandle EmplaceProxy(TArgs&&... args)
        {
            return CreateProxy(std::make_unique<T>(std::forward<TArgs>(args)...));
        }

        template <class T>
        bool UpdateProxy(SceneProxyHandle proxy, std::function<void(T&)>&& func)
        {
            return UpdateProxy(proxy, [&](ISceneProxy& untypedProxy)
            {
                PHX_ASSERT(untypedProxy.GetTypeDescriptor().GetTypeId() == StaticTypeName<T>::TypeId);
                func(static_cast<T&>(untypedProxy));
            });
        }
    };

    class SceneProxyManager : public ISceneProxyManager
    {
        PHX_DECLARE_TYPE_DERIVED(SceneProxyManager, ISceneProxyManager)
    public:

        SceneProxyHandle CreateProxy(std::unique_ptr<ISceneProxy>&& proxy) override;

        bool UpdateProxy(SceneProxyHandle proxy, std::function<void(ISceneProxy&)>&& func) override;

        bool DestroyProxy(SceneProxyHandle proxy) override;

        void DestroyAllProxies() override;

        size_t GetNumProxies() const override;

        size_t GetAllProxies(std::vector<const ISceneProxy*>& outProxies) const override;

        void Gather(RenderScene& scene) const override;

    private:
        std::unordered_map<uint64_t, std::unique_ptr<ISceneProxy>> Proxies;
        uint64_t ProxyIdGen = 0;
    };
}
