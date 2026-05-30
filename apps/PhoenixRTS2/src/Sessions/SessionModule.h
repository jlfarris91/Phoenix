#pragma once

#include "Application/AppModule.h"
#include "Sessions/SessionHandle.h"
#include "Worlds/WorldInstance.h"

class SessionModule : public Phoenix::IAppModule
{
    PHX_DECLARE_TYPE_DERIVED(SessionModule, Phoenix::IAppModule)
public:

    void Register(Phoenix::ServiceContainerBuilder& builder) override;
    void Initialize(Phoenix::ModuleInitContext& context) override;
    void Load(Phoenix::ModuleLoadContext& context) override;
    void Shutdown() override;

private:

    SessionHandle CreateInitialSession();

    void OnSessionInstanceCreated(Phoenix::SessionInstance* instance);
    void OnSessionInstanceDestroyed(Phoenix::SessionInstance* instance);

    void OnWorldInstanceCreated(Phoenix::WorldInstance* instance);
    void OnWorldInstanceDestroyed(Phoenix::WorldInstance* instance);
};
