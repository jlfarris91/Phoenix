#pragma once

#include "Phoenix.Scene/SceneManager.h"

namespace Phoenix::EnTT
{
    class Scene;

    class SceneManager : public Phoenix::Scene::ISceneManager
    {
        PHX_DECLARE_TYPE_DERIVED(SceneManager, ISceneManager)
    public:
        std::shared_ptr<Phoenix::Scene::IScene> CreateScene(FName id) override;
        std::shared_ptr<Phoenix::Scene::IScene> FindScene(FName id) override;
        bool DestroyScene(FName id) override;
        std::vector<std::shared_ptr<Phoenix::Scene::IScene>> GetAllScenes() override;
    private:
        std::unordered_map<FName, std::shared_ptr<Scene>> Scenes;
    };
}
