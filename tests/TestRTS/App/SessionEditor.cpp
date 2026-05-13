#include "SessionEditor.h"

#include "SessionInstance.h"
#include "Tool.h"
#include "ToolManager.h"
#include "../sdl/SDLDebugState.h"

#include "../Tools/CameraTool.h"
#include "../Tools/EntityTool.h"
#include "../Tools/NavMeshTool.h"
#include "../Tools/PlayerController.h"

SessionEditor::SessionEditor(::SessionInstance* sessionInstance)
    : SessionInstance(sessionInstance)
{
    ToolManager = std::make_unique<::ToolManager>();
    DebugState = std::make_unique<SDLDebugState>();

    RegisterTools();
}

SessionInstance* SessionEditor::GetSession() const
{
    return SessionInstance;
}

ToolManager* SessionEditor::GetToolManager() const
{
    return ToolManager.get();
}

Phoenix::FName SessionEditor::GetFocusedWorldId() const
{
    return FocusedWorldId;
}

Phoenix::WorldPtr SessionEditor::GetFocusedWorld() const
{
    Phoenix::Session* session = SessionInstance->GetSession();
    if (!session)
    {
        return nullptr;
    }
    return session->GetWorldManager()->GetWorld(FocusedWorldId).get();
}

void SessionEditor::OnAppEvent(const void* eventData)
{
    const SDL_Event* event = static_cast<const SDL_Event*>(eventData);
    static_cast<SDLDebugState*>(DebugState.get())->OnAppEvent(event);

    if (Phoenix::WorldPtr focusedWorld = GetFocusedWorld())
    {
        for (auto&& tool : ToolManager->GetActiveTools())
        {
            tool->OnAppEvent(*focusedWorld, *DebugState, eventData);
        }
    }
}

void SessionEditor::RegisterTools()
{
    // auto cameraTool         = std::make_unique<CameraTool>(shared_from_this());
    // auto entityTool         = std::make_unique<EntityTool>(shared_from_this());
    // auto navMeshTool        = std::make_unique<NavMeshTool>(shared_from_this());
    // auto playerController   = std::make_unique<PlayerController>(shared_from_this());
    //
    // ToolManager->RegisterTool(std::move(cameraTool));
    // ToolManager->RegisterTool(std::move(entityTool));
    // ToolManager->RegisterTool(std::move(navMeshTool));
    //
    // ITool* pc = ToolManager->RegisterTool(std::move(playerController));
    // ToolManager->ActivateTool(pc);
}
