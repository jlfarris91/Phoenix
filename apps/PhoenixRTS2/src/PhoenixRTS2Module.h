#pragma once

#include "Application/AppModule.h"

class PhoenixRTS2Module : public Phoenix::IAppModule
{
    PHX_DECLARE_TYPE_DERIVED(PhoenixRTS2Module, Phoenix::IAppModule)
public:
    void Register(Phoenix::ServiceContainerBuilder& builder) override;
    void Initialize(Phoenix::ModuleInitContext &context) override;
    void Load(Phoenix::ModuleLoadContext &context) override;
};
