#pragma once

#include "Editor/EditorService.h"

namespace Phoenix::UI
{
    class IShellManager : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(IShellManager, IEditorService)
    };
}
