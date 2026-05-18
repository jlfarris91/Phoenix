#include "MainFrameWidget.h"

#include <format>

#include "EditorWidgetContext.h"
#include "Docking/ImGuiDockRenderer.h"
#include "Menus/ImGuiMenuRenderer.h"
#include "Editor/Editor.h"
#include "UI/Docking/DockManager.h"
#include "UI/Menu/MenuManager.h"

using namespace Phoenix::UI;

MainFrameWidget::MainFrameWidget(const ImGuiWidgetContext& context)
{
    SetContext(context);

    std::shared_ptr<EditorWidgetContext> editorContext = this->GetContextObject<EditorWidgetContext>();

    Editor = editorContext->Editor;

    auto menuManager = Editor.lock()->ResolveService<IMenuManager>();
    MenuRenderer = std::make_unique<ImGuiMenuRenderer>(menuManager);

    auto dockManager = Editor.lock()->ResolveService<IDockManager>();
    DockRenderer = std::make_unique<ImGuiDockRenderer>(dockManager);
}

void MainFrameWidget::Render()
{
    // std::string mainFrameName = std::format("##MainFrame_{}", Editor.lock()->GetId());
    //
    // const ImGuiViewport* vp = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(vp->WorkPos);
    // ImGui::SetNextWindowSize(vp->WorkSize);
    // ImGui::SetNextWindowViewport(vp->ID);
    //
    // ImGuiWindowFlags hostFlags =
    //     ImGuiWindowFlags_NoTitleBar            |
    //     ImGuiWindowFlags_NoCollapse            |
    //     ImGuiWindowFlags_NoResize              |
    //     ImGuiWindowFlags_NoMove                |
    //     ImGuiWindowFlags_NoBringToFrontOnFocus |
    //     ImGuiWindowFlags_MenuBar               |
    //     ImGuiWindowFlags_NoNavFocus            ;
    //
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.0f, 0.0f));
    // ImGui::Begin(mainFrameName.c_str(), nullptr, hostFlags);
    // ImGui::PopStyleVar(3);

    RenderMainMenuBar();
    RenderDocking();

    // ImGui::End();
}

void MainFrameWidget::RenderMainMenuBar()
{
    auto editor = Editor.lock();
    if (!editor)
    {
        return;
    }

    auto menuManager = editor->ResolveService<IMenuManager>();
    
    if (ImGui::BeginMainMenuBar())
    {
        const MenuContext& context = editor->GetMainMenuContext();
        if (auto mainMenu = menuManager->GenerateMenu("MainMenu", context))
        {
            MenuRenderer->RenderMenu(mainMenu);
        }

        ImGui::EndMainMenuBar();
    }
}

void MainFrameWidget::RenderDocking()
{
    DockRenderer->Render();
}