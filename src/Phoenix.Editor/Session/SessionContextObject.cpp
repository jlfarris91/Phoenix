#include "SessionContextObject.h"

#include "SessionEditor.h"

std::shared_ptr<Phoenix::Session> Phoenix::SessionContextObject::GetSession() const
{
    std::shared_ptr<SessionEditor> sessionEditor = GetSessionEditor();
    return sessionEditor ? sessionEditor->GetSession() : nullptr;
}

std::shared_ptr<Phoenix::SessionEditor> Phoenix::SessionContextObject::GetSessionEditor() const
{
    return WeakSessionEditor.lock();
}
