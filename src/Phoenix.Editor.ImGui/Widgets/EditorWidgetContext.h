#pragma once

#include "Object.h"

namespace Phoenix
{
    class Editor;
}

namespace Phoenix::UI
{
    class EditorWidgetContext : public IObject
    {
        PHX_DECLARE_TYPE_DERIVED(EditorWidgetContext, IObject)
    public:
        std::weak_ptr<Editor> Editor;
    };
}
