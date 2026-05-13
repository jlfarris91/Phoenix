#pragma once

#include <unordered_map>

#include "SessionView.h"
#include "Widget.h"
#include "../App/SessionHandle.h"

class SessionInstance;

class AppView : public Widget
{
public:

    void Initialize() override;

    void Render() override;

    void OnAppEvent(const void* eventData);

private:
    void RenderMenuBar();

    void RenderDockSpace();

    void RenderSessionWindows();

    void RenderSessionWindow(SessionInstance* value);

    void HandleNewSession();

    void HandleExit();

    void OnSessionCreated(SessionInstance* value);
    void OnSessionDestroyed(SessionInstance* value);

    std::unordered_map<SessionHandle, std::unique_ptr<SessionView>> SessionViews;
};
