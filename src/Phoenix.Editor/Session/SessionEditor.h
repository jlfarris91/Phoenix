#pragma once

#include <memory>

#include "SessionContextObject.h"

namespace Phoenix
{
    class Session;
    class SessionModuleFactory;
    class IFileManager;

    class SessionEditor : public std::enable_shared_from_this<SessionEditor>, public ISessionContextObject
    {
    public:

        void Initialize(
            const std::shared_ptr<Session>& session,
            const std::shared_ptr<SessionModuleFactory>& sessionModuleFactory,
            const std::shared_ptr<IFileManager>& fileManager);

        void Shutdown();

        void Activate();

        void Deactivate();

        // Begin ISessionContextObject implementation
        virtual std::shared_ptr<Session> GetSession() const override;
        virtual std::shared_ptr<SessionEditor> GetSessionEditor() const override;
        // End ISessionContextObject implementation

    private:

        std::shared_ptr<Session> ActiveSession;
    };
}
