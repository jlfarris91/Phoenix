#include "EditorContextObject.h"

#include "Editor.h"

std::shared_ptr<Phoenix::Editor> Phoenix::EditorContextObject::GetEditor() const
{
    return WeakEditor.lock();
}

std::shared_ptr<Phoenix::Application> Phoenix::EditorContextObject::GetApplication() const
{
    return WeakEditor.lock();
}
