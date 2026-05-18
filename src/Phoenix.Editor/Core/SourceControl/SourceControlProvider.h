#pragma once
#include "Editor/EditorService.h"

namespace Phoenix
{
    class ISourceControlProvider : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(ISourceControlProvider, IEditorService)
    };
}
