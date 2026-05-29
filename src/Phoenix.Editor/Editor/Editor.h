#pragma once

#include "Application/Application.h"
#include "Phoenix/Services/ServiceContainer.h"
#include "UI/Menu/MenuContext.h"

namespace Phoenix
{
    class Application;
    class ServiceContainerBuilder;

    class Editor : public Application
    {
    public:

        explicit Editor(std::shared_ptr<IServiceLocator> locator);

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
