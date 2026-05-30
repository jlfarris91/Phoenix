#pragma once

#include "Application/AppModule.h"

class RendererModule : public Phoenix::IAppModule
{
    PHX_DECLARE_TYPE_DERIVED(RendererModule, Phoenix::IAppModule)
public:
    virtual void Initialize(Phoenix::ModuleInitContext& context) override;
    virtual void Load(Phoenix::ModuleLoadContext& context) override;
};