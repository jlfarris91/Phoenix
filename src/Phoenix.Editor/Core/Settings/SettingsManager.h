#pragma once
#include "Editor/EditorService.h"

namespace Phoenix
{
    class ISettingsManager : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(ISettingsManager, IEditorService)
    };
}
