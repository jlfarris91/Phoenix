#include "RenderScene.h"

namespace Phoenix::Renderer
{
    void RenderScene::Clear()
    {
        Sprites.clear();
        Lines.clear();
        Circles.clear();
        Rects.clear();
        Texts.clear();
    }
}
