#pragma once

#include "EditorContextObject.h"
#include "Modules/Module.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class Editor;

    class IEditorModule : public IModule, public EditorContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(IEditorModule, IModule)
        friend class EditorModuleManager;
    };
}
