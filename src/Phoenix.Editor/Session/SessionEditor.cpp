#include "SessionEditor.h"

void Phoenix::SessionEditor::Initialize(
    const std::shared_ptr<Phoenix::Session>& session,
    const std::shared_ptr<SessionModuleFactory>& sessionModuleFactory,
    const std::shared_ptr<IFileManager>& fileManager)
{
}

void Phoenix::SessionEditor::Shutdown()
{
}

void Phoenix::SessionEditor::Activate()
{
}

void Phoenix::SessionEditor::Deactivate()
{
}

std::shared_ptr<Phoenix::Session> Phoenix::SessionEditor::GetSession() const
{
    return ActiveSession;
}

std::shared_ptr<Phoenix::SessionEditor> Phoenix::SessionEditor::GetSessionEditor() const
{
    // Yikes... but oh well, this feels like a special case... right?
    return std::const_pointer_cast<SessionEditor>(shared_from_this());
}
