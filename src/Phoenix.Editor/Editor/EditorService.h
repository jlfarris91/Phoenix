#pragma once

#include <memory>

#include "EditorContextObject.h"
#include "Services/Service.h"

namespace Phoenix
{
    class Editor;

    class IEditorService : public IService, public EditorContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(IEditorService, IService)
    public:

        virtual void Initialize(const std::shared_ptr<Editor>& editor);
        virtual void Shutdown();
    };
}