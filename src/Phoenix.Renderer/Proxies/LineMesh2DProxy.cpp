#include "LineMesh2DProxy.h"

#include "RenderScene.h"

void Phoenix::Renderer::LineMesh2DProxy::Gather(RenderScene& scene)
{
    ISceneProxy::Gather(scene);

    LineMesh2DCall call;
    call.Mesh = Mesh;
    call.Texture = Texture;
    call.Transform = Transform;
    call.Tint = Tint;

    scene.Commands.emplace_back(Layer, call);
}
