#pragma once

#include "SessionContextObject.h"
#include "Editor/EditorContextObject.h"
#include "Modules/Module.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class Editor;

    class ISessionModule : public IModule, public SessionContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(ISessionModule, IModule)
        friend class SessionModuleManager;
    };
}
