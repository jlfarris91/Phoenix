#include "Scene.h"

namespace Phoenix
{
    Renderer::RenderScene Scene::Gather(WorldConstRef world) const
    {
        Renderer::RenderScene scene;
        scene.View   = Camera.GetView();
        scene.Target = Target;
        for (const auto& layer : Layers)
            layer->Gather(world, scene);
        return scene;
    }
}
