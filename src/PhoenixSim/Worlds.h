#pragma once

#include <nlohmann/json.hpp>

#include "Config.h"
#include "SessionFwd.h"
#include "PhoenixSim/Actions.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/Containers/BlockBuffer.h"
#include "PhoenixSim/Random.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class IConfigService;
    struct WorldLayoutContext;
}

namespace Phoenix
{
    class Session;
    class FeatureSet;
    class IDebugRenderer;
    class IDebugState;
}

namespace Phoenix
{
    enum class PHOENIX_SIM_API EWorldFlags : uint8
    {
        None = 0,
        Initialized = 1,
        ShutDown = 2
    };

    struct PHOENIX_SIM_API WorldConfig
    {
        std::shared_ptr<Session> Session;
        FName WorldType;
        FName WorldId;
        WorldJsonConfig Config;
        BlockBufferConfig BufferConfig;
    };

    class PHOENIX_SIM_API World : public BlockBufferOwner<World>
    {
    public:

        World(const WorldConfig& config);
        World(const World& other);
        World(World&& other) noexcept;
        ~World() = default;

        std::weak_ptr<Session> GetSession() const;

        FName GetId() const;
        FName GetType() const;

        const WorldJsonConfig& GetWorldConfig() const;
        const FeatureJsonConfig* GetFeatureConfig(const FName& featureId) const;

        bool IsInitialized() const;
        bool IsShutDown() const;
        bool IsActive() const;

        Time GetSimTime() const;

        Random& GetRandom();

        World& operator=(const World& other);
        World& operator=(World&& other) noexcept;

        void CopyTo(World& other) const;
        void SyncTo(World& view) const;

        BlockBuffer& GetBuffer();
        const BlockBuffer& GetBuffer() const;

    private:

        friend class WorldManager;

        std::weak_ptr<Session> Session;
        FName Id;
        FName Type;
        BlockBuffer Buffer;
        EWorldFlags Flags = EWorldFlags::None;
        WorldJsonConfig Config;

        Time SimTime;
        Random Random;
    };

    struct PHOENIX_SIM_API WorldManagerCtorArgs
    {
        std::weak_ptr<Session> Session;
        std::weak_ptr<FeatureSet> FeatureSet;
        std::weak_ptr<IConfigService> ConfigService;
        PostWorldUpdateDelegate OnPostWorldUpdate;
    };

    struct PHOENIX_SIM_API NewWorldArgs
    {
        FName WorldType;
        TOptional<FName> Id;
        TOptional<nlohmann::json> Config;
    };

    struct PHOENIX_SIM_API WorldStepArgs
    {
        simtime_t SimTime = 0;
        uint32 StepHz = 0;
        FName WorldName = FName::None;
    };

    struct PHOENIX_SIM_API WorldSendActionArgs
    {
        Action Action;
        FName WorldName = FName::None;
    };

    struct PHOENIX_SIM_API ViewContext
    {
        EBufferBlockTypeFlags BlockTypeFlags = EBufferBlockTypeFlags::None;
        std::vector<BufferBlockDefinition> BlockDefinitions;
    };

    struct PHOENIX_SIM_API WorldLayoutContext
    {
        std::shared_ptr<Session> Session;
        FName WorldType;
        FName WorldId;
        WorldJsonConfig Config;
    };

    class PHOENIX_SIM_API WorldManager
    {
    public:
        WorldManager(const WorldManagerCtorArgs& args);
        ~WorldManager();

        // Create a new world.
        WorldSharedPtr NewWorld(const NewWorldArgs& args);

        // Gets a world by the given name. Returns nullptr if not found.
        WorldSharedPtr GetWorld(const FName& name) const;

        // Gets the first world created by the session.
        WorldSharedPtr GetPrimaryWorld() const;

        void Step(const WorldStepArgs& args);

        void SendAction(const WorldSendActionArgs& args);

    private:

        friend class Session;

        void ApplyConfig() const;

        FName GenerateNewWorldId(const FName& worldType);

        void InitializeWorld(WorldRef world, simtime_t time) const;
        void ShutdownWorld(WorldRef world) const;
        void UpdateWorld(WorldRef world, simtime_t time, clock_t stepHz) const;
        void SendActionToWorld(WorldRef world, const Action& action) const;

        std::shared_ptr<Session> Session;
        std::shared_ptr<FeatureSet> FeatureSet;
        std::shared_ptr<IConfigService> ConfigService;
        std::vector<WorldSharedPtr> Worlds;

        uint32 WorldIdGen = 0;

        PostWorldUpdateDelegate OnPostWorldUpdate;
    };
}
