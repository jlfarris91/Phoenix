#include "SessionService.h"

void Phoenix::ISessionService::Initialize(const std::shared_ptr<SessionEditor>& sessionEditor)
{
    WeakSessionEditor = sessionEditor;
}

void Phoenix::ISessionService::Shutdown()
{
    WeakSessionEditor.reset();
}
