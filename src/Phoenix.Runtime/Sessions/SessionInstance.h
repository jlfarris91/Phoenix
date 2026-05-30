#pragma once

#include <memory>
#include <atomic>
#include <queue>
#include <thread>

#include <Phoenix/Name.h>
#include <Phoenix.Sim/Session.h>
#include <Phoenix/Delegates.h>

#include "Dispatch.h"

namespace Phoenix
{
    class WorldInstance;

    class SessionInstance : public std::enable_shared_from_this<SessionInstance>
                          , public Dispatcher
    {
    public:
        SessionInstance(uint32_t id, const SessionCtorArgs& args);
        ~SessionInstance() override;

        void Initialize();

        void Start();

        bool Shutdown(bool wait);

        bool IsShuttingDown() const;
        bool IsShutDown() const;

        uint32_t GetId() const;
        Session* GetSession() const;

        // Called on the sim thread to step the session forward.
        void TickSession();

        // Called on the game thread each frame to advance double buffers.
        void Tick();

        // Returns the WorldInstance for a given world ID, or null if not yet created.
        WorldInstance* GetWorldInstance(FName worldId) const;

        // Returns the stable world view for a given world ID, or null if not yet ready.
        const World* GetWorldView(FName worldId) const;

        PHX_DECLARE_MULTICAST_DELEGATE(FWorldInstanceCreated, WorldInstance*);
        FWorldInstanceCreated WorldInstanceCreated;

        PHX_DECLARE_MULTICAST_DELEGATE(FWorldInstanceDestroyed, WorldInstance*);
        FWorldInstanceDestroyed WorldInstanceDestroyed;

    private:

        static void SessionWorker(SessionInstance* instance);

        void OnPostWorldUpdateImpl_Sim(WorldConstRef world);

        void OnShutdown();

        uint32_t Id;

        SessionCtorArgs SessionArgs;
        std::shared_ptr<Session> Session;

        std::shared_mutex WorldsMutex;
        std::unordered_map<FName, std::unique_ptr<WorldInstance>> Worlds;

        std::unique_ptr<std::thread> SessionThread;
        std::atomic<bool> bSessionThreadWantsExit = false;
        std::atomic<bool> bSessionThreadExited = false;
        std::atomic<bool> bTickSession = true;
        std::atomic<uint32_t> WantsSessionStep = 0u;

        double SimSpeed = 1.0;
    };
}

