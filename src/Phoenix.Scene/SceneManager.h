#pragma once

#include "Application/AppService.h"

namespace Phoenix::Scene
{
    class IScene;

    class ISceneManager : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(ISceneManager, IAppService);
    public:
        virtual std::shared_ptr<IScene> CreateScene(FName id) = 0;
        virtual std::shared_ptr<IScene> FindScene(FName id) = 0;
        virtual bool DestroyScene(FName id) = 0;
        virtual std::vector<std::shared_ptr<IScene>> GetAllScenes() = 0;
    };
}
