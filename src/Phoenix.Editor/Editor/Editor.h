#pragma once

#include "Application/Application.h"
#include "Services/ServiceContainer.h"
#include "UI/Menu/MenuContext.h"

namespace Phoenix
{
    class Application;
    class ServiceContainerBuilder;

    class Editor : public Application
    {
    public:

        struct CtorArgs
        {
            ServiceContainerBuilder* Builder;
        };

        Editor(const CtorArgs& args);

        const UI::MenuContext& GetMainMenuContext();

    protected:

        void InitializeInternal() override;
        void ShutdownInternal() override;
        
    private:

        void BuildDockingLayout();
        void RegisterCommands();
        void RegisterMainMenu();
        void RegisterFileMenu();
        void RegisterWindowsMenu();

        UI::MenuContext MainMenuContext;
    };
}
