
#include "PhoenixSim/Worlds.h"

#include <algorithm>
#include <fstream>
#include <memory>

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"

using namespace Phoenix;

World::World(const WorldCtorArgs& args)
    : Session(args.Session)
    , Id(args.WorldId)
    , Type(args.WorldType)
    , Buffer(args.Blocks)
    , Config(args.Config)
{
}

World::World(const World& other)
    : Session(other.Session)
    , Id(other.Id)
    , Type(other.Type)
    , Buffer(other.Buffer)
    , Config(other.Config)
{
}

World::World(World&& other) noexcept
    : Session(other.Session)
    , Id(other.Id)
    , Type(other.Type)
    , Buffer(std::move(other.Buffer))
    , Config(std::move(other.Config))
{
}

TWeakPtr<Session> World::GetSession() const
{
    return Session;
}

FName World::GetId() const
{
    return Id;
}

FName World::GetType() const
{
    return Type;
}

const nlohmann::json& World::GetConfig() const
{
    return Config;
}

const nlohmann::json* World::GetFeatureConfig(const PHXString& featureId) const
{
    auto featuresIter = Config.find("features");
    if (featuresIter == Config.end())
    {
        return nullptr;
    }

    auto featureIter = featuresIter->find(featureId);
    if (featureIter == featuresIter->end())
    {
        return nullptr;
    }

    return &*featureIter;
}

bool World::IsInitialized() const
{
    return HasAnyFlags(Flags, EWorldFlags::Initialized);
}

bool World::IsShutDown() const
{
    return HasAnyFlags(Flags, EWorldFlags::ShutDown);
}

bool World::IsActive() const
{
    return HasNoneFlags(Flags, EWorldFlags::Initialized, EWorldFlags::ShutDown);
}

Time World::GetSimTime() const
{
    return GetBlockRef<WorldDynamicBlock>().SimTime;
}

World& World::operator=(const World& other)
{
    Id = other.Id;
    Type = other.Type;
    Buffer = other.Buffer;
    Config = other.Config;
    return *this;
}

World& World::operator=(World&& other) noexcept
{
    Id = other.Id;
    Type = other.Type;
    Buffer = std::move(other.Buffer);
    Config = std::move(other.Config);
    return *this;
}

BlockBuffer& World::GetBuffer()
{
    return Buffer;
}

const BlockBuffer& World::GetBuffer() const
{
    return Buffer;
}

WorldManager::WorldManager(const WorldManagerCtorArgs& args)
    : Session(args.Session)
    , FeatureSet(args.FeatureSet)
    , OnPostWorldUpdate(args.OnPostWorldUpdate)
{
    LoadConfig(args.Config);

    WorldBufferBlockArgs.RegisterBlock<WorldDynamicBlock>();

    for (const FeatureSharedPtr& feature : FeatureSet.lock()->GetFeatures())
    {
        FeatureDefinition worldFeatureDef = feature->GetFeatureDefinition();
        for (const BlockBuffer::BlockDefinition& blockArgs : worldFeatureDef.WorldBlocks.Definitions)
        {
            WorldBufferBlockArgs.Definitions.push_back(blockArgs);
        }
    }
}

WorldManager::~WorldManager()
{
}

WorldSharedPtr WorldManager::NewWorld(const NewWorldArgs& args)
{
    if (args.Id.IsSet())
    {
        // TODO (jfarris): report an error that a world with that id already exists
        if (WorldSharedPtr existingWorld = GetWorld(args.Id.Get()))
        {
            return nullptr;
        }
    }

    FName worldId = args.Id.GetValue(FName::None);
    if (FName::IsNoneOrEmpty(worldId))
    {
        worldId = GenerateNewWorldId(args.WorldType);
    }

    WorldCtorArgs worldCtorArgs;
    worldCtorArgs.Session = Session;
    worldCtorArgs.WorldId = worldId;
    worldCtorArgs.WorldType = args.WorldType;
    worldCtorArgs.Blocks = WorldBufferBlockArgs;

    auto worldConfigIter = WorldConfigs.find(args.WorldType);
    if (worldConfigIter != WorldConfigs.end())
    {
        worldCtorArgs.Config = worldConfigIter->second;
    }

    WorldSharedPtr world = std::make_shared<World>(worldCtorArgs);
    Worlds.push_back(world);

    return world;
}

WorldSharedPtr WorldManager::GetWorld(const FName& name) const
{
    for (const WorldSharedPtr& world : Worlds)
    {
        if (world->GetId() == name)
            return world;
    }
    return nullptr;
}

WorldSharedPtr WorldManager::GetPrimaryWorld() const
{
    return Worlds[0];
}

void WorldManager::Step(const WorldStepArgs& args)
{
    TArray<WorldSharedPtr> worlds;

    if (args.WorldName != FName::None)
    {
        if (WorldSharedPtr world = GetWorld(args.WorldName))
        {
            worlds.push_back(world);
        }
    }
    else
    {
        worlds = Worlds;
    }

    for (const WorldSharedPtr& world : worlds)
    {
        if (!world->IsInitialized())
        {
            InitializeWorld(*world);
        }
    }

    // TODO (jfarris): parallelize
    for (const WorldSharedPtr& world : worlds)
    {
        UpdateWorld(*world, args.SimTime, args.StepHz);
    }
}

void WorldManager::SendAction(const WorldSendActionArgs& args)
{
    TArray<WorldSharedPtr> worlds;

    if (args.WorldName != FName::None)
    {
        if (WorldSharedPtr world = GetWorld(args.WorldName))
        {
            worlds.push_back(world);
        }
    }
    else
    {
        worlds = Worlds;
    }

    // TODO (jfarris): parallelize
    for (const WorldSharedPtr& world : worlds)
    {
        SendActionToWorld(*world, args.Action);
    }
}

bool WorldManager::LoadConfig(const nlohmann::json& config)
{
    Config = config;
    WorldConfigs.clear();

    bool anyFailed = false;
    for (auto && [worldType, worldConfig] : Config.items())
    {
        auto configIter = worldConfig.find("config");
        if (configIter == worldConfig.end())
        {
            // TODO (jfarris): report warning
            anyFailed = true;
            continue;
        }

        PHXString configPath = configIter->get<PHXString>();

        if (!LoadWorldConfig(worldType, configPath))
        {
            anyFailed = true;
        }
    }

    return anyFailed == false;
}

bool WorldManager::LoadWorldConfig(const PHXString& worldType, const PHXString& configPath)
{
    std::filesystem::path dataDir = Session.lock()->GetDataDirectory();
    std::filesystem::path worldConfigPath = absolute(dataDir / configPath);
    if (!exists(worldConfigPath))
    {
        // TODO (jfarris): report warning
        return false;
    }

    std::ifstream worldConfigStream(worldConfigPath);
    if (!worldConfigStream.is_open())
    {
        // TODO (jfarris): report warning
        return false;
    }

    nlohmann::json worldConfigJson = nlohmann::json::parse(worldConfigStream);
    if (worldConfigJson.is_discarded())
    {
        // TODO (jfarris): report error
        return false;
    }

    WorldConfigs.emplace(FName(worldType), worldConfigJson);
    return true;
}

FName WorldManager::GenerateNewWorldId(const FName& worldType)
{
    ++WorldIdGen;
    char buffer[16];
    size_t len = snprintf(buffer, sizeof(buffer), "%u", WorldIdGen);
    return worldType.Append(buffer, len);
}

void WorldManager::InitializeWorld(WorldRef world) const
{
    TArray2<FeatureSharedPtr> channelFeatures = FeatureSet.lock()->GetChannelRef(FeatureChannels::WorldInitialize);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldInitialize(world);
    }

    SetFlagRef(world.Flags, EWorldFlags::Initialized, true);
}

void WorldManager::ShutdownWorld(WorldRef world) const
{
    TArray2<FeatureSharedPtr> channelFeatures = FeatureSet.lock()->GetChannelRef(FeatureChannels::WorldShutdown);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldShutdown(world);
    }

    SetFlagRef(world.Flags, EWorldFlags::ShutDown, true);
}

void WorldManager::UpdateWorld(WorldRef world, simtime_t time, clock_t stepHz) const
{
    PHX_PROFILE_ZONE_SCOPED;

    world.GetBlockRef<WorldDynamicBlock>().SimTime = Time::QT(time);

    FeatureUpdateArgs updateArgs;
    updateArgs.SimTime = time;
    updateArgs.StepHz = stepHz;

    TSharedPtr<Phoenix::FeatureSet> featureSet = FeatureSet.lock();
    if (!featureSet)
    {
        return;
    }

    // Pre-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreWorldUpdate");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::PreWorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreWorldUpdate(world, updateArgs);
        }
    }

    // Update
    {
        PHX_PROFILE_ZONE_SCOPED_N("WorldUpdate");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::WorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnWorldUpdate(world, updateArgs);
        }
    }

    // Post-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostWorldUpdate");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::PostWorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostWorldUpdate(world, updateArgs);
        }
    }

    OnPostWorldUpdate(world);
}

void WorldManager::SendActionToWorld(WorldRef world, const Action& action) const
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureActionArgs actionArgs;
    actionArgs.Action = action;

    TSharedPtr<Phoenix::FeatureSet> featureSet = FeatureSet.lock();
    if (!featureSet)
    {
        return;
    }

    // Pre handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreHandleWorldAction");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::PreHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreHandleWorldAction(world, actionArgs);
        }
    }

    // Handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("HandleWorldAction");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::HandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnHandleWorldAction(world, actionArgs);
        }
    }

    // Post handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostHandleWorldAction");

        const TArray2<FeatureSharedPtr>& channelFeatures = featureSet->GetChannelRef(FeatureChannels::PostHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostHandleWorldAction(world, actionArgs);
        }
    }
}
