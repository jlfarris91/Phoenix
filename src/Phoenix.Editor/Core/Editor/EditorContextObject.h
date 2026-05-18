#pragma once

#include <memory>

#include "Application/AppContextObject.h"

namespace Phoenix
{
    class Editor;

    class IEditorContextObject : public IAppContextObject
    {
    public:
        virtual std::shared_ptr<Editor> GetEditor() const = 0;
    };

    class EditorContextObject : public IEditorContextObject
    {
    public:
        virtual std::shared_ptr<Editor> GetEditor() const override;
        virtual std::shared_ptr<Application> GetApplication() const override;
    protected:
        std::weak_ptr<Editor> WeakEditor;
    };
}
