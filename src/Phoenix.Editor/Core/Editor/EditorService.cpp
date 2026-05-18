#include "EditorService.h"

#include "Editor.h"

void Phoenix::IEditorService::Initialize(const std::shared_ptr<Phoenix::Editor>& editor)
{
    WeakEditor = editor;
}

void Phoenix::IEditorService::Shutdown()
{
    WeakEditor.reset();
}
