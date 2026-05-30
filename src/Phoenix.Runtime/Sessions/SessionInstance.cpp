#include "SessionInstance.h"

#include "../Worlds/WorldInstance.h"
#include "Phoenix/Profiling.h"
#include "Phoenix.Sim/Session.h"

using namespace Phoenix;

SessionInstance::SessionInstance(uint32_t id, const SessionCtorArgs& args)
    : Id(id)
    , SessionArgs(args)
{
}

SessionInstance::~SessionInstance()
{
    Shutdown(true);
}

void SessionInstance::Initialize()
{
    if (Session)
    {
        return;
    }

    SessionCtorArgs args2 = SessionArgs;
    args2.OnPostWorldUpdate = [weakThis = weak_from_this()](WorldConstRef world)
    {
        if (auto self = weakThis.lock())
        {
            self->OnPostWorldUpdateImpl_Sim(world);
        }
    };

    Session = Session::Create(args2);

    Session->Initialize();

    WorldManager* worldManager = Session->GetWorldManager();

    worldManager->NewWorld({
        .WorldType = "DefaultWorld"_n,
        .Id = "TestWorld"_n
    });
}

void SessionInstance::Start()
{
    if (SessionThread)
    {
        return;
    }

#ifndef __EMSCRIPTEN__
    SessionThread = std::make_unique<std::thread>(SessionWorker, this);
#endif
}

bool SessionInstance::Shutdown(bool wait)
{
    if (bSessionThreadExited)
    {
        return true;
    }

    if (bSessionThreadWantsExit)
    {
        return bSessionThreadExited;
    }

    bSessionThreadWantsExit = true;
    if (SessionThread)
    {
        if (wait)
        {
            SessionThread->join();
        }
    }
    else
    {
        // There may not be a thread if the user is manually ticking the session. In that case just shutdown.
        OnShutdown();
    }

    return bSessionThreadExited;
}

bool SessionInstance::IsShuttingDown() const
{
    return bSessionThreadWantsExit && !bSessionThreadExited;
}

bool SessionInstance::IsShutDown() const
{
    return bSessionThreadExited;
}

uint32_t SessionInstance::GetId() const
{
    return Id;
}

Session* SessionInstance::GetSession() const
{
    return Session.get();
}

void SessionInstance::TickSession()
{
    if (!Session)
    {
        return;
    }

#ifndef __EMSCRIPTEN__
    // TODO (jfarris): this can cause a crash for some reason
    // FrameMarkNamed("Sim");
#endif

    FlushDispatchQueue();

    SessionStepArgs stepArgs;
    stepArgs.SpeedMultiplier = SimSpeed;

    Session->Tick(stepArgs);
}

void SessionInstance::SessionWorker(SessionInstance* instance)
{
    PHX_PROFILE_SET_THREAD_NAME("Sim", instance->Id);

    instance->bSessionThreadWantsExit = false;
    instance->bSessionThreadExited = false;
    instance->SetOwningThread();

    while (!instance->bSessionThreadWantsExit)
    {
        if (instance->bTickSession || instance->WantsSessionStep)
        {
            instance->TickSession();

            if (instance->WantsSessionStep)
            {
                --instance->WantsSessionStep;
            }
        }
    }

    instance->OnShutdown();

    instance->bSessionThreadExited = true;
}

void SessionInstance::Tick()
{
    for (auto &world: Worlds | std::views::values)
    {
        world->Sink();
    }
}

WorldInstance* SessionInstance::GetWorldInstance(FName worldId) const
{
    auto it = Worlds.find(worldId);
    return it != Worlds.end() ? it->second.get() : nullptr;
}

const World* SessionInstance::GetWorldView(FName worldId) const
{
    const WorldInstance* world = GetWorldInstance(worldId);
    return world ? world->GetWorldView() : nullptr;
}

void SessionInstance::OnPostWorldUpdateImpl_Sim(WorldConstRef world)
{
    WorldInstance* worldInstance = nullptr;

    // Get or create a new world instance
    {
        std::scoped_lock lock(WorldsMutex);

        auto it = Worlds.find(world.GetId());
        if (it == Worlds.end())
        {
            auto newWorldInstance = std::make_unique<WorldInstance>(world.GetId());
            auto result = Worlds.emplace(world.GetId(), std::move(newWorldInstance));
            it = result.first;
            worldInstance = it->second.get();

            Dispatch([this, worldInstance]
            {
                WorldInstanceCreated.Broadcast(worldInstance);
            });
        }

        worldInstance = it->second.get();
    }

    worldInstance->OnUpdate_Sim(world);
}

void SessionInstance::OnShutdown()
{
    Session->Shutdown();
}
