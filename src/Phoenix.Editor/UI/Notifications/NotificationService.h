#pragma once

#include "Editor/EditorService.h"

namespace Phoenix::UI
{
    class INotificationService : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(INotificationService, IEditorService)
    };
}
