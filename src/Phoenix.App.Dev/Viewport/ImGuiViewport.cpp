#include "ImGuiViewport.h"

#include <imgui.h>

#include "SceneViewport.h"
#include "SDL3ResourceManager.h"
#include "Resources/Texture2D.h"

namespace Phoenix::App::Dev
{
    using namespace App;

    ImGuiViewport::ImGuiViewport(std::shared_ptr<SceneViewport>       viewport,
                                  std::shared_ptr<SDL3ResourceManager>  resources)
        : Viewport(std::move(viewport))
        , Resources(std::move(resources))
    {
    }

    void ImGuiViewport::Draw()
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        int w = static_cast<int>(size.x);
        int h = static_cast<int>(size.y);

        if (w > 0 && h > 0)
            Viewport->Resize(w, h);

        auto rt = Viewport->GetRenderTarget();
        if (!rt.IsValid())
            return;

        auto resource = Resources->GetResource<Texture2D>(rt);
        if (!resource)
            return;

        SDL_Texture* sdlTex = resource->GetSDLTexture();
        if (!sdlTex)
            return;

        ImGui::Image(reinterpret_cast<ImTextureID>(sdlTex), size);
    }
}
