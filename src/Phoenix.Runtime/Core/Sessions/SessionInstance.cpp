#include "SessionInstance.h"

#include "WorldDoubleBuffer.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"

SessionInstance::SessionInstance(uint32_t id, const Phoenix::SessionCtorArgs& args)
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

    Phoenix::SessionCtorArgs args2 = SessionArgs;
    args2.OnPostWorldUpdate = [weakThis = shared_from_this()](Phoenix::WorldConstRef world)
    {
        if (weakThis)
        {
            weakThis->OnPostWorldUpdateImpl(world);
        }
    };

    Session = Phoenix::Session::Create(args2);

    Session->Initialize();

    Phoenix::WorldManager* worldManager = Session->GetWorldManager();

    worldManager->NewWorld({
        .WorldType = "DefaultWorld"_n,
        .Id = "TestWorld"_n
    });
}

void SessionInstance::Start()
{
    if (Session)
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

Phoenix::Session* SessionInstance::GetSession() const
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

    Phoenix::SessionStepArgs stepArgs;
    stepArgs.SpeedMultiplier = SimSpeed;

    Session->Tick(stepArgs);
}

void SessionInstance::SessionWorker(SessionInstance* instance)
{
    PHX_PROFILE_SET_THREAD_NAME("Sim", instance->Id);

    instance->bSessionThreadWantsExit = false;
    instance->bSessionThreadExited = false;

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

void SessionInstance::OnPostWorldUpdateImpl(Phoenix::WorldConstRef world)
{
    WorldDoubleBuffer* worldSink = nullptr;

    auto iter = WorldSinks.find(world.GetId());
    if (iter != WorldSinks.end())
    {
        worldSink = iter->second.get();
    }
    else
    {
        auto result = WorldSinks.emplace(world.GetId(), std::make_unique<WorldDoubleBuffer>());
        if (result.second)
        {
            worldSink = result.first->second.get();
        }
    }

    if (worldSink)
    {
        worldSink->OnSimUpdate(world);
    }

    OnPostWorldUpdate.Broadcast(this, world);
}

void SessionInstance::OnShutdown()
{
    Session->Shutdown();
}
