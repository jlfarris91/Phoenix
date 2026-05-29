#include "TransformComponentSyncHandler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "Components/SceneComponent.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.ECS/TransformComponent.h"

bool TransformComponentSyncHandler::CanSync(const Phoenix::App::Dev::SceneComponentSyncArgs& args)
{
    return args.SimComponentTypeId == Phoenix::StaticTypeName<Phoenix::ECS::TransformComponent>::TypeId;
}

void TransformComponentSyncHandler::OnSync(const Phoenix::App::Dev::SceneComponentSyncArgs& args)
{
    glm::mat4 worldTransform = glm::mat4(1.0f);

    if (auto transformPtr = Phoenix::ECS::FeatureECS::GetWorldTransformPtr(*args.World, args.SimEntity))
    {
        glm::vec3 position = { (float)transformPtr->Position.X, (float)transformPtr->Position.Y, 0 };
        auto rotation = glm::radians((float)transformPtr->Rotation);
        glm::vec3 scale = { (float)transformPtr->Scale, (float)transformPtr->Scale, 1 };

        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), rotation, { 0, 0, 1 });
        glm::mat4 scaleMtx = glm::scale(glm::mat4(1.0f), scale);

        worldTransform = transMtx * rotMtx * scaleMtx;
    }

    auto& registry = args.Scene->GetRegistry();
    auto& sceneComp = registry.get_or_emplace<Phoenix::App::Dev::SceneComponent>(args.SceneEntity);
    sceneComp.WorldTransform = worldTransform;
}
