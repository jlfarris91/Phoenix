
#include "PhoenixSim/Session.h"

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Services/ServiceContainerBuilder.h"
#include "PhoenixSim/Worlds.h"

#include <algorithm>
#ifdef _WIN32
#include <windows.h>  // For Sleep
#else
#include <unistd.h>   // For usleep
#endif

#include <fstream>

#include "Logging.h"

using namespace Phoenix;

Session::~Session()
{
    FeatureSet.reset();
    WorldManager.reset();
    ServiceContainer.reset();
}

TSharedPtr<Session> Session::Create(const SessionCtorArgs& args)
{
    TSharedPtr<Session> session = MakeShared<Session>();

    session->DataDirectory = std::filesystem::absolute(args.DataDirectory);
    session->ConfigName = args.ConfigName;

    if (args.CustomConfig.IsSet())
    {
        session->CustomConfig = args.CustomConfig.Get();
    }

    if (args.ServiceContainerBuilder)
    {
        session->ServiceContainer = args.ServiceContainerBuilder->Build();
    }
    else
    {
        session->ServiceContainer = MakeShared<Phoenix::ServiceContainer>();
    }

    auto featuresIter = session->Config.find("features");
    if (featuresIter != session->Config.end())
    {
        for (auto && [featureId, featureConfig] : featuresIter->items())
        {
            session->FeatureConfigs.emplace(FName(featureId), featureConfig);
        }
    }

    FeatureSetCtorArgs featureSetCtorArgs;
    session->ServiceContainer->GetServices2<IFeature>(featureSetCtorArgs.Features);
    session->FeatureSet = MakeShared<Phoenix::FeatureSet>(featureSetCtorArgs);

    WorldManagerCtorArgs worldManagerArgs;
    worldManagerArgs.Session = session;
    worldManagerArgs.FeatureSet = session->FeatureSet;
    worldManagerArgs.OnPostWorldUpdate = args.OnPostWorldUpdate;
    worldManagerArgs.Config = session->Config["worlds"];
    session->WorldManager = MakeShared<Phoenix::WorldManager>(worldManagerArgs);

    BlockBuffer::CtorArgs sessionBlockArgs;
    for (const FeatureSharedPtr& feature : session->FeatureSet->GetFeatures())
    {
        const FeatureDefinition& featureDefinition = feature->GetFeatureDefinition();
        for (const BlockBuffer::BlockDefinition& sessionBlock : featureDefinition.SessionBlocks.Definitions)
        {
            sessionBlockArgs.Definitions.push_back(sessionBlock);
        }
    }

    session->SessionBuffer = BlockBuffer(sessionBlockArgs);

    return session;
}

void Session::Initialize()
{
    LoadConfig();

    for (const TSharedPtr<IService>& service : ServiceContainer->GetServices())
    {
        service->Initialize(shared_from_this());
    }

    StartTime = PHX_SYS_CLOCK_NOW();
    CurrTickTime = StartTime;
    LastStepTime = StartTime;
    AccTickTime = sys_clock_dur_t(0);
    FPSCalc.Reset();
}

void Session::Shutdown()
{
    for (const TSharedPtr<IService>& service : ServiceContainer->GetServices())
    {
        service->Shutdown();
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

    auto currTime = PHX_SYS_CLOCK_NOW();
    auto dt = currTime - CurrTickTime;
    CurrTickTime = currTime;

    // Skip frames during debug break
    if (dt > 3s)
    {
        return;
    }

    sys_clock_dur_t hz = sys_clock_dur_t(1s) / args.StepHz;

    AccTickTime += dt;
    while (AccTickTime >= hz)
    {
        auto startStepTime = PHX_SYS_CLOCK_NOW();

        Step(args);

        sys_clock_t endStepTime = PHX_SYS_CLOCK_NOW();
        auto stepElapsed = endStepTime - startStepTime;

        if (stepElapsed > 3s)
        {
            break;
        }

        AccTickTime -= hz;
        CurrTickTime = PHX_SYS_CLOCK_NOW();
    }
}

void Session::Step(const SessionStepArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    LastStepTime = PHX_SYS_CLOCK_NOW();
    SimTime += 1;

    FPSCalc.Tick();

    // Process actions
    ProcessActions(SimTime);

    // Step features at the session level
    UpdateSession(SimTime, args.StepHz);

    // Step active worlds
    WorldStepArgs worldStepArgs;
    worldStepArgs.SimTime = SimTime;
    worldStepArgs.StepHz = args.StepHz;
    WorldManager->Step(worldStepArgs);
}

BlockBuffer& Session::GetBuffer()
{
    return SessionBuffer;
}

const BlockBuffer& Session::GetBuffer() const
{
    return SessionBuffer;
}

TSharedPtr<IFeature> Session::GetFeature(const FName& featureId) const
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

ServiceContainer* Session::GetServiceContainer() const
{
    return ServiceContainer.get();
}

PHXString Session::GetDataDirectory() const
{
    return DataDirectory.generic_string();
}

PHXString Session::GetWorldsDirectory() const
{
    return (DataDirectory / "Worlds").generic_string();
}

PHXString Session::GetWorldDirectory(const PHXString& worldType) const
{
    return (DataDirectory / "Worlds" / worldType).generic_string();
}

TSharedPtr<IService> Session::GetService(const FName& typeId) const
{
    return ServiceContainer->GetService(typeId);
}

uint32 Session::GetServices(const FName& typeId, TArray2<TSharedPtr<IService>>& outServices) const
{
    return ServiceContainer->GetServices(typeId, outServices);
}

const TArray2<TSharedPtr<IService>>& Session::GetServices() const
{
    return ServiceContainer->GetServices();
}

void Session::LoadConfig()
{
    Config.clear();

    std::filesystem::path sessionConfigPath = DataDirectory / (ConfigName + ".json");

    LogVerbose("Loading config at path {0}", sessionConfigPath.string());

    std::ifstream configStream(sessionConfigPath);
    if (!configStream.is_open())
    {
        LogWarning("Failed to load config at path {0}", sessionConfigPath.string());
        return;
    }

    nlohmann::json configJson = nlohmann::json::parse(configStream);
    if (!configJson.is_discarded())
    {
        Config = configJson;
    }

    if (CustomConfig.IsSet())
    {
        Config.merge_patch(CustomConfig.Get());
    }

    FeatureConfigs.clear();

    auto featuresIter = Config.find("features");
    if (featuresIter != Config.end())
    {
        for (auto && [featureId, featureConfig] : featuresIter->items())
        {
            FeatureConfigs.emplace(FName(featureId), featureConfig);
        }
    }

    ApplyConfig();
}

void Session::ApplyConfig()
{
    for (const FeatureSharedPtr& feature : FeatureSet->GetFeatures())
    {
        feature->Config.clear();

        const TypeDescriptor& typeDescriptor = feature->GetTypeDescriptor();
        
        auto featureConfigIter = FeatureConfigs.find(typeDescriptor.GetFName());
        if (featureConfigIter != FeatureConfigs.end())
        {
            feature->Config = featureConfigIter->second;
        }
    }

    auto worldsIter = Config.find("worlds");
    if (worldsIter != Config.end())
    {
        WorldManager->LoadConfig(*worldsIter);
    }
}

void Session::ProcessActions(simtime_t time)
{
    PHX_PROFILE_ZONE_SCOPED;

    // Process incoming actions
    {
        std::lock_guard lock(ActionQueueMutex);

        // Sort action queue by sim time
        std::ranges::sort(ActionQueue,
              [](const TTuple<simtime_t, Action>& a, const TTuple<simtime_t, Action>& b)
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

void Session::UpdateSession(simtime_t time, uint32 stepHz) const
{
    FeatureUpdateArgs updateArgs;
    updateArgs.SimTime = time;
    updateArgs.StepHz = stepHz;

    // Pre-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreUpdate");
        const TArray2<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreUpdate(updateArgs);
        }
    }

    // Update
    {
        PHX_PROFILE_ZONE_SCOPED_N("Update");
        const TArray2<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::Update);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnUpdate(updateArgs);
        }
    }

    // Post-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostUpdate");
        const TArray2<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostUpdate(updateArgs);
        }
    }
}
