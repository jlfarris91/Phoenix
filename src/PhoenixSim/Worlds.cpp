#pragma once

#include "PhoenixSim/Worlds.h"

#include <algorithm>
#include <fstream>
#include <memory>

#include "Config.h"
#include "Logging.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Session.h"

using namespace Phoenix;

World::World(const WorldConfig& config)
    : Session(config.Session)
    , Id(config.WorldId)
    , Type(config.WorldType)
    , Buffer(config.BufferConfig)
    , Config(config.Config)
    , SimTime(0)
{
}

World::World(const World& other)
    : Session(other.Session)
    , Id(other.Id)
    , Type(other.Type)
    , Buffer(other.Buffer)
    , Config(other.Config)
    , SimTime(other.SimTime)
    , Random(other.Random)
{
}

World::World(World&& other) noexcept
    : Session(other.Session)
    , Id(other.Id)
    , Type(other.Type)
    , Buffer(std::move(other.Buffer))
    , Config(std::move(other.Config))
    , SimTime(other.SimTime)
    , Random(std::move(other.Random))
{
}

std::weak_ptr<Session> World::GetSession() const
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

const WorldJsonConfig& World::GetWorldConfig() const
{
    return Config;
}

const FeatureJsonConfig* World::GetFeatureConfig(const FName& featureId) const
{
    return Config.GetFeatureConfig(featureId);
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
    return SimTime;
}

Random& World::GetRandom()
{
    return Random;
}

World& World::operator=(const World& other)
{
    Id = other.Id;
    Type = other.Type;
    Buffer = other.Buffer;
    Config = other.Config;
    SimTime = other.SimTime;
    Random = other.Random;
    return *this;
}

World& World::operator=(World&& other) noexcept
{
    Id = other.Id;
    Type = other.Type;
    Buffer = std::move(other.Buffer);
    Config = std::move(other.Config);
    SimTime = other.SimTime;
    Random = std::move(other.Random);
    return *this;
}

void World::CopyTo(World& other) const
{
    other.Id = Id;
    other.Type = Type;
    other.Config = Config;
    other.SimTime = SimTime;
    other.Random = Random;
    Buffer.CopyTo(other.Buffer);
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
    ConfigService = Session->GetServiceAs<IConfigService>();
}

WorldManager::~WorldManager()
{
}

WorldSharedPtr WorldManager::NewWorld(const NewWorldArgs& args)
{
    if (args.Id.IsSet())
    {
        if (WorldSharedPtr existingWorld = GetWorld(args.Id.Get()))
        {
            LogError("A world with id '{}' already exists!", static_cast<hash32_t>(args.Id.Get()));
            return nullptr;
        }
    }

    FName worldId = args.Id.GetValue(FName::None);
    if (FName::IsNoneOrEmpty(worldId))
    {
        worldId = GenerateNewWorldId(args.WorldType);
    }

    WorldLayoutContext layoutContext;
    layoutContext.Session = Session;
    layoutContext.WorldId = worldId;
    layoutContext.WorldType = args.WorldType;

    if (const WorldJsonConfig* worldJsonConfig = ConfigService->GetWorldConfig(args.WorldType))
    {
        layoutContext.Config = *worldJsonConfig;
    }

    BlockBufferLayoutBuilder layoutBuilder;

    // TODO (jfarris): allow filter features for world
    // TODO (jfarris): sort features by dependencies so ie ECS is laid-out before steering
    for (const std::shared_ptr<IFeature>& feature : FeatureSet->GetFeatures())
    {
        const FeatureDefinition& featureDef = feature->GetFeatureDefinition();

        for (const BufferBlockDefinition& block : featureDef.WorldBlocks.Definitions)
        {
            layoutBuilder.RegisterBlock(block);
        }

        feature->OnWorldLayout(layoutContext, layoutBuilder);
    }

    WorldConfig worldConfig;
    worldConfig.Session = Session;
    worldConfig.WorldId = worldId;
    worldConfig.WorldType = args.WorldType;
    worldConfig.Config = layoutContext.Config;
    worldConfig.BufferConfig = layoutBuilder.GetLayout();

    WorldSharedPtr world = std::make_shared<World>(worldConfig);
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
    std::vector<WorldSharedPtr> worlds;

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
            InitializeWorld(*world, args.SimTime);
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
    std::vector<WorldSharedPtr> worlds;

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

void WorldManager::ApplyConfig() const
{
    for (WorldSharedPtr world : Worlds)
    {
        world->Config = {};
        if (const WorldJsonConfig* worldConfig = ConfigService->GetWorldConfig(world->GetType()))
        {
            world->Config = *worldConfig;
        }
    }
}

FName WorldManager::GenerateNewWorldId(const FName& worldType)
{
    ++WorldIdGen;
    char buffer[16];
    size_t len = snprintf(buffer, sizeof(buffer), "%u", WorldIdGen);
    return worldType.Append(buffer, len);
}

void WorldManager::InitializeWorld(WorldRef world, simtime_t time) const
{
    const nlohmann::json& worldConfigData = world.Config.GetData();

    world.SimTime = Time::QT(time);

    uint64 randomSeed = Hashing::FNV1A64Combine(time, world.Id);
    auto seedIter = worldConfigData.find("seed");
    if (seedIter != worldConfigData.end())
    {
        std::string seedStr = seedIter->get<std::string>();
        randomSeed = Hashing::FNV1A64(seedStr.c_str(), seedStr.length());
    }
    world.Random.Seed(randomSeed);

    std::vector<FeatureSharedPtr> channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldInitialize);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldInitialize(world);
    }

    for (const std::shared_ptr<IService>& service : Session->GetServices())
    {
        if (!IsA<IFeature>(service))
        {
            service->OnWorldInitialize(world);
        }
    }

    SetFlagRef(world.Flags, EWorldFlags::Initialized, true);
}

void WorldManager::ShutdownWorld(WorldRef world) const
{
    std::vector<FeatureSharedPtr> channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldShutdown);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldShutdown(world);
    }

    for (const std::shared_ptr<IService>& service : Session->GetServices())
    {
        if (!IsA<IFeature>(service))
        {
            service->OnWorldShutdown(world);
        }
    }

    SetFlagRef(world.Flags, EWorldFlags::ShutDown, true);
}

void WorldManager::UpdateWorld(WorldRef world, simtime_t time, clock_t stepHz) const
{
    PHX_PROFILE_ZONE_SCOPED;

    world.SimTime = Time::QT(time);

    FeatureUpdateArgs updateArgs;
    updateArgs.SimTime = time;
    updateArgs.StepHz = stepHz;

    // Pre-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreWorldUpdate");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreWorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreWorldUpdate(world, updateArgs);
        }
    }

    // Update
    {
        PHX_PROFILE_ZONE_SCOPED_N("WorldUpdate");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnWorldUpdate(world, updateArgs);
        }
    }

    // Post-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostWorldUpdate");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostWorldUpdate);
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

    // Pre handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreHandleWorldAction");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreHandleWorldAction(world, actionArgs);
        }
    }

    // Handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("HandleWorldAction");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::HandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnHandleWorldAction(world, actionArgs);
        }
    }

    // Post handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostHandleWorldAction");

        const std::vector<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostHandleWorldAction(world, actionArgs);
        }
    }
}
