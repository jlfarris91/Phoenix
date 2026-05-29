#pragma once

#include "SceneComponentHandler.h"

class TransformComponentSyncHandler : public Phoenix::App::Dev::ISceneComponentHandler
{
    PHX_DECLARE_TYPE_DERIVED(TransformComponentSyncHandler, ISceneComponentHandler)
public:
    bool CanSync(const Phoenix::App::Dev::SceneComponentSyncArgs &args) override;
    void OnSync(const Phoenix::App::Dev::SceneComponentSyncArgs &args) override;
};