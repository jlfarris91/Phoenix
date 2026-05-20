#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim/Config.h"
#include "Phoenix.Sim/Features.h"
#include "Phoenix/Profiling.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"
#include "Phoenix.Sim/Services/ISessionService.h"
#include "Phoenix.Sim/Worlds.h"

#include <algorithm>
#include <thread>

using namespace Phoenix;

Session::~Session()
{
    FeatureSet.reset();
    WorldManager.reset();
    Container.reset();
}

std::shared_ptr<Session> Session::Create(const SessionCtorArgs& args)
{
    std::shared_ptr<Session> session = std::make_shared<Session>();

    session->DataDirectory = std::filesystem::absolute(args.DataDirectory);
    session->ConfigName = args.ConfigName;

    if (args.ServiceContainerBuilder)
    {
        (void)args.ServiceContainerBuilder->Register<DefaultConfigService>().AsInterfaces();
        session->Container = args.ServiceContainerBuilder->Build();
    }
    else
    {
        ServiceContainerBuilder builder;
        (void)builder.Register<DefaultConfigService>().AsInterfaces();
        session->Container = builder.Build();
    }

    session->ConfigService = session->Container->ResolveService<IConfigService>();
    session->ConfigService->LoadConfig(args.DataDirectory, args.ConfigName);

    FeatureSetCtorArgs featureSetCtorArgs;
    session->Container->ResolveServices<IFeature>(featureSetCtorArgs.Features);
    session->FeatureSet = std::make_shared<Phoenix::FeatureSet>(featureSetCtorArgs);

    WorldManagerCtorArgs worldManagerArgs;
    worldManagerArgs.Session = session;
    worldManagerArgs.FeatureSet = session->FeatureSet;
    worldManagerArgs.ConfigService = session->ConfigService;
    worldManagerArgs.OnPostWorldUpdate = args.OnPostWorldUpdate;
    session->WorldManager = std::make_shared<Phoenix::WorldManager>(worldManagerArgs);

    // Construct block buffer
    {
        SessionLayoutContext layoutContext;
        layoutContext.Config = session->ConfigService->GetSessionConfig();
        
        BlockBufferConfigBuilder layoutBuilder;

        for (const std::shared_ptr<IFeature>& feature : session->FeatureSet->GetFeatures())
        {
            const FeatureDefinition& featureDef = feature->GetFeatureDefinition();

            for (const BufferBlockDefinition& block : featureDef.SessionBlocks.Definitions)
            {
                layoutBuilder.RegisterBlock(block);
            }

            feature->OnSessionLayout(layoutContext, layoutBuilder);
        }

        session->SessionBuffer = BlockBuffer(layoutBuilder.GetLayout());
    }

    return session;
}

void Session::Initialize()
{
    LoadConfig();

    for (const auto& service : Container->GetInstances())
    {
        if (auto sessionService = Cast<ISessionService>(service))
            sessionService->Initialize(shared_from_this());
    }

    StartTime = PHX_SYS_CLOCK_NOW();
    CurrTickTime = StartTime;
    LastStepTime = StartTime;
    FPSCalc.Reset();
}

void Session::Shutdown()
{
    WorldManager->Shutdown();

    auto instances = Container->GetInstances();
    for (const auto& service : instances)
    {
        if (auto sessionService = Cast<ISessionService>(service))
            sessionService->Shutdown();
    }
}

void Session::EnqueueAction(const Action& action)
{
    std::lock_guard lock(ActionQueueMutex);
    ActionQueue.emplace_back(SimTime + 1, action);
}

void Session::Tick(const SessionStepArgs& args)
{
    using namespace std::chrono;

    auto stepStart = PHX_SYS_CLOCK_NOW();

    Step(args);

    auto stepElapsed = PHX_SYS_CLOCK_NOW() - stepStart;

    sys_clock_dur_t targetInterval = duration_cast<sys_clock_dur_t>(
        duration<double>(sys_clock_dur_t(1s) / Time::D) / args.SpeedMultiplier);

    auto remaining = targetInterval - stepElapsed;
    if (remaining > sys_clock_dur_t(0))
    {
        std::this_thread::sleep_for(remaining);
    }
}

void Session::Step(const SessionStepArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    LastStepTime = PHX_SYS_CLOCK_NOW();
    SimTime += 1;

    FPSCalc.Time = LastStepTime;

    WorldStepArgs worldStepArgs;
    worldStepArgs.SimTime = SimTime;

    // Start tracking memory changes prior to processing actions that may mutate the world state.
    WorldManager->PreStep(worldStepArgs);

    // Process actions
    ProcessActions(SimTime);

    // Step features at the session level
    UpdateSession(SimTime);

    // Step active worlds
    WorldManager->Step(worldStepArgs);

    // After mutating the world state, end tracking and calculate dirty pages.
    WorldManager->PostStep(worldStepArgs);

    FPSCalc.Tick();
}

BlockBuffer& Session::GetBuffer()
{
    return SessionBuffer;
}

const BlockBuffer& Session::GetBuffer() const
{
    return SessionBuffer;
}

std::shared_ptr<IFeature> Session::GetFeature(const FName& featureId) const
{
    return FeatureSet->GetFeature(featureId);
}

sys_clock_t Session::GetCurrTime() const
{
    return CurrTickTime;
}

sys_clock_t Session::GetStartTime() const
{
    return StartTime;
}

sys_clock_t Session::GetLastStepTime() const
{
    return LastStepTime;
}

simtime_t Session::GetSimTime() const
{
    return SimTime;
}

const FPSCalc& Session::GetFPSCalc() const
{
    return FPSCalc;
}

FeatureSet* Session::GetFeatureSet() const
{
    return FeatureSet.get();
}

WorldManager* Session::GetWorldManager() const
{
    return WorldManager.get();
}


std::string Session::GetDataDirectory() const
{
    return DataDirectory.generic_string();
}

std::string Session::GetWorldsDirectory() const
{
    return (DataDirectory / "Worlds").generic_string();
}

std::string Session::GetWorldDirectory(const std::string& worldType) const
{
    return (DataDirectory / "Worlds" / worldType).generic_string();
}

void Session::LoadConfig() const
{
    ConfigService->LoadConfig(DataDirectory, ConfigName);
    ApplyConfig();
}

void Session::ApplyConfig() const
{
    for (const FeatureSharedPtr& feature : FeatureSet->GetFeatures())
    {
        feature->Config.clear();
        if (const FeatureJsonConfig* featureConfig = ConfigService->GetSessionFeatureConfig(feature->GetFeatureId()))
        {
            feature->Config = featureConfig->GetData();
        }
    }

    WorldManager->ApplyConfig();
}

void Session::ProcessActions(simtime_t time)
{
    PHX_PROFILE_ZONE_SCOPED;

    // Process incoming actions
    {
        std::lock_guard lock(ActionQueueMutex);

        // Sort action queue by sim time
        std::ranges::sort(ActionQueue,
              [](const std::tuple<simtime_t, Action>& a, const std::tuple<simtime_t, Action>& b)
              {
                  return std::get<0>(a) < std::get<0>(b);
              });
        
        auto iter = ActionQueue.begin();
        for (; iter != ActionQueue.end(); ++iter)
        {
            const simtime_t& timestamp = std::get<0>(*iter);
            const Action& action = std::get<1>(*iter);

            // 
            if (timestamp > time)
            {
                break;
            }

            WorldSendActionArgs args;
            args.Action = action;
            WorldManager->SendAction(args);
        }

        if (iter != ActionQueue.begin())
        {
            ActionQueue.erase(ActionQueue.begin(), iter);
        }
    }
}

void Session::UpdateSession(simtime_t time) const
{
    FeatureUpdateArgs updateArgs;
    updateArgs.SimTime = time;

    // Pre-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreUpdate");
        const std::vector<Phoenix::FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreUpdate);
        for (const Phoenix::FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreUpdate(updateArgs);
        }
    }

    // Update
    {
        PHX_PROFILE_ZONE_SCOPED_N("Update");
        const std::vector<Phoenix::FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::Update);
        for (const Phoenix::FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnUpdate(updateArgs);
        }
    }

    // Post-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostUpdate");
        const std::vector<Phoenix::FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostUpdate);
        for (const Phoenix::FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostUpdate(updateArgs);
        }
    }
}
