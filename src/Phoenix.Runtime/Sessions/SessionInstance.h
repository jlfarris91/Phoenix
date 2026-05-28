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
        SessionInstance(uint32_t id, const Phoenix::SessionCtorArgs& args);
        ~SessionInstance() override;

        void Initialize();

        void Start();

        bool Shutdown(bool wait);

        bool IsShuttingDown() const;
        bool IsShutDown() const;

        uint32_t GetId() const;
        Phoenix::Session* GetSession() const;

        // Called on the sim thread to step the session forward.
        void TickSession();

        // Called on the game thread each frame to advance double buffers.
        void Tick();

        // Returns the WorldInstance for a given world ID, or null if not yet created.
        WorldInstance* GetWorldInstance(Phoenix::FName worldId) const;

        // Returns the stable world view for a given world ID, or null if not yet ready.
        const Phoenix::World* GetWorldView(Phoenix::FName worldId) const;

        PHX_DECLARE_MULTICAST_DELEGATE(FWorldInstanceCreated, WorldInstance*, Phoenix::WorldConstRef);
        FWorldInstanceCreated WorldInstanceCreated;

        PHX_DECLARE_MULTICAST_DELEGATE(FWorldInstanceUpdated, WorldInstance*, Phoenix::WorldConstRef);
        FWorldInstanceUpdated WorldInstanceUpdated;

        PHX_DECLARE_MULTICAST_DELEGATE(FWorldInstanceDestroyed, WorldInstance*);
        FWorldInstanceDestroyed WorldInstanceDestroyed;

    private:

        static void SessionWorker(SessionInstance* instance);

        void OnPostWorldUpdateImpl(Phoenix::WorldConstRef world);

        void OnShutdown();

        uint32_t Id;

        Phoenix::SessionCtorArgs SessionArgs;
        std::shared_ptr<Phoenix::Session> Session;
        std::unordered_map<Phoenix::FName, std::unique_ptr<WorldInstance>> Worlds;

        std::unique_ptr<std::thread> SessionThread;
        std::atomic<bool> bSessionThreadWantsExit = false;
        std::atomic<bool> bSessionThreadExited = false;
        std::atomic<bool> bTickSession = true;
        std::atomic<uint32_t> WantsSessionStep = 0u;

        double SimSpeed = 1.0;
    };
}

