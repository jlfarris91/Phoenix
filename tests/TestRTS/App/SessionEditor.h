#pragma once

#include <memory>

#include "ToolManager.h"
#include "PhoenixSim/Debug/Debug.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/WorldsFwd.h"

class SessionInstance;

class SessionEditor : public std::enable_shared_from_this<SessionEditor>
{
public:
    SessionEditor(SessionInstance* sessionInstance);

    SessionInstance* GetSession() const;
    ToolManager* GetToolManager() const;

    Phoenix::FName GetFocusedWorldId() const;
    Phoenix::WorldPtr GetFocusedWorld() const;

    void OnAppEvent(const void* eventData);

private:

    void RegisterTools();

    SessionInstance* SessionInstance;
    std::unique_ptr<ToolManager> ToolManager;
    Phoenix::FName FocusedWorldId;

    std::unique_ptr<Phoenix::IDebugState> DebugState;
};
