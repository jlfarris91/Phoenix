#include "SceneManager.h"

#include <ranges>

#include "Scene.h"
#include "Phoenix/Logging.h"

std::shared_ptr<Phoenix::Scene::IScene> Phoenix::App::Dev::SceneManager::CreateScene(FName id)
{
    if (Scenes.contains(id))
    {
        LogError("Scene already exists with id {}", id.ToString());
        return nullptr;
    }

    auto scene = std::make_shared<Scene>(id);
    Scenes.emplace(id, scene);

    return scene;
}

std::shared_ptr<Phoenix::Scene::IScene> Phoenix::App::Dev::SceneManager::FindScene(FName id)
{
    auto iter = Scenes.find(id);
    return iter != Scenes.end() ? iter->second : nullptr;
}

bool Phoenix::App::Dev::SceneManager::DestroyScene(FName id)
{
    auto iter = Scenes.find(id);
    if (iter == Scenes.end())
    {
        LogError("Scene not found with id {}", id.ToString());
        return false;
    }
    Scenes.erase(iter);
    return true;
}

std::vector<std::shared_ptr<Phoenix::Scene::IScene>> Phoenix::App::Dev::SceneManager::GetAllScenes()
{
    std::vector<std::shared_ptr<Scene::IScene>> scenes;
    scenes.reserve(Scenes.size());
    for (const auto& scene: Scenes | std::views::values)
    {
        scenes.push_back(scene);
    }
    return scenes;
}
