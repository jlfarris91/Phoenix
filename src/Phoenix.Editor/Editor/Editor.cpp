#include "Editor.h"

#include <cassert>
#include <format>

#include "EditorService.h"
#include "Application/Application.h"
#include "Application/AppService.h"
#include "Phoenix/Services/ServiceContainer.h"
#include "UI/Docking/DockManager.h"
#include "UI/Docking/DockTab.h"
#include "UI/Menu/MenuManager.h"
#include "UI/Menu/MenuUtility.h"

using namespace Phoenix;

namespace
{
    std::string GMainMenuName = "MainMenu";
    std::string GFileMenuName = UI::JoinMenuPaths(GMainMenuName, "File");
    std::string GEditMenuName = UI::JoinMenuPaths(GMainMenuName, "Edit");
    std::string GWindowsMenuName = UI::JoinMenuPaths(GMainMenuName, "Windows");
    std::string GHelpMenuName = UI::JoinMenuPaths(GMainMenuName, "Help");

    std::shared_ptr<const UI::CommandInfo> GNewCommand;
    std::shared_ptr<const UI::CommandInfo> GOpenCommand;
    std::shared_ptr<const UI::CommandInfo> GSaveCommand;
    std::shared_ptr<const UI::CommandInfo> GSaveAsCommand;
    std::shared_ptr<const UI::CommandInfo> GSaveAllCommand;
    std::shared_ptr<const UI::CommandInfo> GCloseCommand;
    std::shared_ptr<const UI::CommandInfo> GExitCommand;
    std::shared_ptr<const UI::CommandInfo> GOpenTestWindowCommands[10];
    std::vector<std::shared_ptr<const UI::CommandInfo>> GCommands;
    std::shared_ptr<UI::CommandList> GCommandList;
}

Editor::Editor(std::shared_ptr<IServiceLocator> locator)
    : Application(std::move(locator))
{
}

const UI::MenuContext& Editor::GetMainMenuContext()
{
    return MainMenuContext;
}

void Editor::InitializeInternal()
{
    std::vector<std::shared_ptr<IService>> services;
    Container->ResolveServices<IService>(services);

    auto thisSP = std::static_pointer_cast<Editor>(shared_from_this());

    for (const auto& service : services)
    {
        if (auto editorService = Cast<IEditorService>(service))
        {
            editorService->Initialize(thisSP);
        }
        else if (auto appService = Cast<IAppService>(service))
        {
            appService->Initialize(thisSP);
        }
    }

    BuildDockingLayout();
    RegisterCommands();
    RegisterMainMenu();
}

void Editor::ShutdownInternal()
{
    std::vector<std::shared_ptr<IService>> services;
    Container->ResolveServices<IService>(services);

    auto thisSP = std::static_pointer_cast<Editor>(shared_from_this());

    for (const auto& service : services)
    {
        if (auto editorService = Cast<IEditorService>(service))
        {
            editorService->Shutdown();
        }
        else if (auto appService = Cast<IAppService>(service))
        {
            appService->Shutdown();
        }
    }
}

void Editor::BuildDockingLayout()
{
    // auto dockManager = ResolveService<UI::IDockManager>();
    //
    // const auto& rootDockAreas = dockManager->GetRootDockAreas();
    //
    // auto rootDockArea = rootDockAreas.front();
    // rootDockArea->SetOrientation(UI::EDockOrientation::Vertical);
    //
    // auto workArea = std::make_shared<UI::DockSplitter>();
    // workArea->SetOrientation(UI::EDockOrientation::Horizontal);
    // rootDockArea->Split(workArea);
    //
    // auto debugStack = std::make_shared<UI::DockStack>();
    // debugStack->AddTab({ .TabType = "TestWindow", .InstanceId = 1 });
    // workArea->Split(debugStack);
    //
    // auto viewportStack = std::make_shared<UI::DockStack>();
    // viewportStack->AddTab({ .TabType = "TestWindow", .InstanceId = 2 });
    // workArea->Split(viewportStack);
    //
    // auto inspectorStack = std::make_shared<UI::DockStack>();
    // inspectorStack->AddTab({ .TabType = "TestWindow", .InstanceId = 3 });
    // workArea->Split(inspectorStack);
    //
    // auto browserStack = std::make_shared<UI::DockStack>();
    // browserStack->AddTab({ .TabType = "TestWindow", .InstanceId = 3 });
    // rootDockArea->Split(browserStack);
}

void Editor::RegisterCommands()
{
    GNewCommand = UI::CommandInfo::CreateCommand("File.New", "New", "Save the current file", {});
    GCommands.push_back(GNewCommand);

    GOpenCommand = UI::CommandInfo::CreateCommand("File.Open", "Open", "Save the current file", {});
    GCommands.push_back(GOpenCommand);

    GSaveCommand = UI::CommandInfo::CreateCommand("File.Save", "Save", "Save the current file", {});
    GCommands.push_back(GSaveCommand);

    GSaveAsCommand = UI::CommandInfo::CreateCommand("File.SaveAs", "Save As...", "Save the current file", {});
    GCommands.push_back(GSaveAsCommand);

    GSaveAllCommand = UI::CommandInfo::CreateCommand("File.SaveAll", "Save All", "Save the current file", {});
    GCommands.push_back(GSaveAllCommand);

    GCloseCommand = UI::CommandInfo::CreateCommand("File.Close", "Close", "Save the current file", {});
    GCommands.push_back(GCloseCommand);

    GExitCommand = UI::CommandInfo::CreateCommand("File.Exit", "Exit", "Save the current file", {});
    GCommands.push_back(GExitCommand);

    for (size_t i = 0; i < _countof(GOpenTestWindowCommands); ++i)
    {
        std::string name = std::format("Windows.OpenTestWindow{}", i + 1);
        std::string label = std::format("Open Test Window {}", i + 1);
        GOpenTestWindowCommands[i] = UI::CommandInfo::CreateCommand(name, label, "Open a test window", {});
        GCommands.push_back(GOpenTestWindowCommands[i]);
    }

    GCommandList = std::make_shared<UI::CommandList>();
    
    for (size_t i = 0; i < _countof(GOpenTestWindowCommands); ++i)
    {
        GCommandList->BindAction(GOpenTestWindowCommands[i], [this, i]
        {
            if (auto dockManager = ResolveService<UI::IDockManager>())
            {
                auto instanceId = static_cast<uint32_t>(i + 1);
                dockManager->RestoreTab(UI::DockTabId{.TabType="TestWindow", .InstanceId=instanceId}, true);
            }
        });
    }
    
    MainMenuContext.AppendCommandList(GCommandList);
}

void Editor::RegisterMainMenu()
{
    auto menuManager = ResolveService<UI::IMenuManager>();

    auto menuBar = menuManager->RegisterMenu(GMainMenuName);

    UI::MenuContext menuContext;
    menuContext.AppendCommandList(GCommandList);
    menuBar->SetContext(menuContext);

    menuManager->RegisterMenu(GFileMenuName);
    menuBar->AddSubMenu({}, "File", "File", "Open the file menu");

    menuManager->RegisterMenu(GEditMenuName);
    menuBar->AddSubMenu({}, "Edit", "Edit", "Open the edit menu");

    menuManager->RegisterMenu(GWindowsMenuName);
    menuBar->AddSubMenu({}, "Windows", "Windows", "Open the windows menu");

    menuManager->RegisterMenu(GHelpMenuName);
    menuBar->AddSubMenu({}, "Help", "Help", "Open the help menu");

    RegisterFileMenu();
    RegisterWindowsMenu();
}

void Editor::RegisterFileMenu()
{
    auto menuManager = ResolveService<UI::IMenuManager>();
 
    auto fileMenu = menuManager->FindMenu(GFileMenuName);

    // New Section
    {
        UI::MenuSection& newSection = fileMenu->AddSection(
            "FileNew",
            "New",
            UI::MenuInsertPosition({}, UI::EMenuInsertPosition::First));
        newSection.AddEntry(GNewCommand);
    }

    // Open Section
    {
        UI::MenuSection& openSection = fileMenu->AddSection(
            "FileOpen",
            "Open",
            UI::MenuInsertPosition("FileNew", UI::EMenuInsertPosition::After));

        openSection.AddEntry(GOpenCommand);
    }

    // Save Section
    {
        UI::MenuSection& saveSection = fileMenu->AddSection(
            "FileSession",
            "Session",
            UI::MenuInsertPosition("FileOpen", UI::EMenuInsertPosition::After));
        saveSection.AddEntry(GSaveCommand);
        saveSection.AddEntry(GSaveAsCommand);
        saveSection.AddEntry(GSaveAllCommand);
        saveSection.AddEntry(GCloseCommand);
    }

    // Exit Section
    {
        UI::MenuSection& exitSection = fileMenu->AddSection("FileExit", "Exit");
        exitSection.AddEntry(GExitCommand);
    }
}

void Editor::RegisterWindowsMenu()
{
    auto menuManager = ResolveService<UI::IMenuManager>();
 
    auto windowsMenu = menuManager->FindMenu(GWindowsMenuName);

    // Test Window Section
    {
        UI::MenuSection& testWindowSection = windowsMenu->AddSection("Test", "Test");
        for (size_t i = 0; i < _countof(GOpenTestWindowCommands); ++i)
        {
            testWindowSection.AddEntry(GOpenTestWindowCommands[i]);
        }
    }
}