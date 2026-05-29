#pragma once

#include <memory>

namespace Phoenix::App::Dev
{
    class SceneViewport;
    class SDL3ResourceManager;

    // Wraps the viewport texture in an ImGui window and resizes the SceneViewport
    // when the available content region changes.
    class ImGuiViewport
    {
    public:
        using Dependencies = std::tuple<SceneViewport, SDL3ResourceManager>;
        ImGuiViewport(std::shared_ptr<SceneViewport> viewport, std::shared_ptr<SDL3ResourceManager> resources);

        // Call inside an ImGui::Begin/End block. Displays the scene texture and
        // resizes the backing SceneViewport if the content region changed.
        void Draw();

    private:
        std::shared_ptr<SceneViewport> Viewport;
        std::shared_ptr<SDL3ResourceManager> Resources;
    };
}
