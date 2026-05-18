#include "EditorModuleManager.h"

#include "EditorModule.h"

bool Phoenix::EditorModuleManager::CanRegisterModule(const TypeDescriptor& moduleType) const
{
    return moduleType.IsA<IEditorModule>();
}

void Phoenix::EditorModuleManager::OnModuleInitialize(IModule* module)
{
    ModuleManager::OnModuleInitialize(module);
    Phoenix::Cast<IEditorModule>(module)->WeakEditor = WeakEditor;
}

void Phoenix::EditorModuleManager::OnModuleShutdown(IModule* module)
{
    ModuleManager::OnModuleShutdown(module);
    Phoenix::Cast<IEditorModule>(module)->WeakEditor.reset();
}
