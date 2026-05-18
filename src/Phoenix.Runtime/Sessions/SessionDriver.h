#pragma once

#include <unordered_map>

#include "SessionHandle.h"
#include <Phoenix.Sim/Session.h>

namespace Phoenix
{
    class SessionInstance;
    class WorldDoubleBuffer;

    class SessionDriver
    {
    public:

        SessionHandle CreateSession(const SessionCtorArgs& args);
        void DestroySession(SessionHandle handle);

        SessionInstance* FindInstance(SessionHandle handle);

        std::vector<SessionInstance*> GetSessions() const;

        PHX_DECLARE_MULTICAST_DELEGATE(FSessionCreated, SessionInstance*);
        FSessionCreated OnSessionCreated;

        PHX_DECLARE_MULTICAST_DELEGATE(FSessionDestroyed, SessionInstance*);
        FSessionDestroyed OnSessionDestroyed;

    private:

        std::unordered_map<uint32_t, std::unique_ptr<SessionInstance>> Sessions;
        SessionHandle SessionIdGen = 0;
    };
}
