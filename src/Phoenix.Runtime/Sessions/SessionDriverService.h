#pragma once

#include "Application/AppService.h"
#include "SessionDriver.h"

namespace Phoenix
{
    class SessionDriverService : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(SessionDriverService, IAppService)
    public:

        void Initialize(const std::shared_ptr<Application> &application) override;
        void Shutdown() override;

        SessionDriver& GetSessionDriver() const;

        Phoenix::SessionDriver::FSessionCreated SessionCreated;
        Phoenix::SessionDriver::FSessionDestroyed SessionDestroyed;

        void Tick() override;

    private:

        void OnSessionCreated(SessionInstance* instance);
        void OnSessionDestroyed(SessionInstance* instance);

        std::unique_ptr<Phoenix::SessionDriver> SessionDriver;
    };
}
