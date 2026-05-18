#pragma once

#include "Editor/EditorService.h"

namespace Phoenix::UI
{
    class IStatusBarManager : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(IStatusBarManager, IEditorService)
    };
}
