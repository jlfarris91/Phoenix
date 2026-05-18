#pragma once

#include <memory>
#include <atomic>
#include <thread>

#include <PhoenixSim/Name.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/Delegates.h>

namespace Phoenix
{
    class WorldDoubleBuffer;

    class SessionInstance : public std::enable_shared_from_this<SessionInstance>
    {
    public:
        SessionInstance(uint32_t id, const Phoenix::SessionCtorArgs& args);
        ~SessionInstance();

        void Initialize();

        void Start();

        bool Shutdown(bool wait);

        bool IsShuttingDown() const;
        bool IsShutDown() const;

        uint32_t GetId() const;
        Phoenix::Session* GetSession() const;

        void TickSession();

        PHX_DECLARE_MULTICAST_DELEGATE(FPostWorldUpdate, SessionInstance*, Phoenix::WorldConstRef);
        FPostWorldUpdate OnPostWorldUpdate;

    private:

        static void SessionWorker(SessionInstance* instance);

        void OnPostWorldUpdateImpl(Phoenix::WorldConstRef world);

        void OnShutdown();

        uint32_t Id;

        Phoenix::SessionCtorArgs SessionArgs;
        std::shared_ptr<Phoenix::Session> Session;
        std::unordered_map<Phoenix::FName, std::unique_ptr<WorldDoubleBuffer>> WorldSinks;

        std::unique_ptr<std::thread> SessionThread;
        std::atomic<bool> bSessionThreadWantsExit = false;
        std::atomic<bool> bSessionThreadExited = false;
        std::atomic<bool> bTickSession = true;
        std::atomic<uint32_t> WantsSessionStep = 0u;

        double SimSpeed = 1.0;
    };
}

