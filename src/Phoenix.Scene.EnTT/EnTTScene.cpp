#include "EnTTScene.h"

namespace Phoenix
{
    Renderer::RenderScene EnTTScene::Gather(WorldConstRef world)
    {
        EntitySync.Sync(world);

        Renderer::RenderScene scene;
        scene.View   = Camera.GetView();
        scene.Target = Target;

        for (const auto& layer : Layers)
            layer->Gather(Registry, world, scene);

        return scene;
    }
}
