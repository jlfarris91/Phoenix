#pragma once

#include "Modules/ModuleManager.h"
#include "Editor/EditorService.h"

namespace Phoenix
{
    class EditorModuleManager : public ModuleManager, public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(EditorModuleManager, IEditorService)

    protected:

        virtual bool CanRegisterModule(const TypeDescriptor& moduleType) const override;
        virtual void OnModuleInitialize(IModule* module) override;
        virtual void OnModuleShutdown(IModule* module) override;
    };
}
