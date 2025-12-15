#pragma once

#include <nlohmann/json.hpp>

#include "Actions.h"
#include "Optional.h"
#include "Containers/BlockBuffer.h"

namespace Phoenix
{
    class Session;
    struct IDebugRenderer;
}

namespace Phoenix
{
    struct IDebugState;
}

namespace Phoenix
{
    class FeatureSet;
    class FeatureSet;
}

namespace Phoenix
{
    enum class EWorldFlags : uint8
    {
        None = 0,
        Initialized = 1,
        ShutDown = 2
    };
    
    struct PHOENIXSIM_API WorldCtorArgs
    {
        TWeakPtr<Session> Session;
        FName WorldId;
        FName WorldType;
        BlockBuffer::CtorArgs Blocks;
        nlohmann::json Config;
    };

    class PHOENIXSIM_API World : public BlockBufferOwner<World>
    {
    public:

        World(const WorldCtorArgs& args);
        World(const World& other);
        World(World&& other) noexcept;
        ~World() = default;

        TWeakPtr<Session> GetSession() const;

        FName GetId() const;
        FName GetType() const;

        const nlohmann::json& GetConfig() const;
        const nlohmann::json* GetFeatureConfig(const PHXString& featureId) const;

        bool IsInitialized() const;
        bool IsShutDown() const;
        bool IsActive() const;

        Time GetSimTime() const;

        World& operator=(const World& other);
        World& operator=(World&& other) noexcept;

        BlockBuffer& GetBuffer();
        const BlockBuffer& GetBuffer() const;

    private:

        friend class WorldManager;

        TWeakPtr<Session> Session;
        FName Id;
        FName Type;
        BlockBuffer Buffer;
        EWorldFlags Flags = EWorldFlags::None;
        nlohmann::json Config;
    };

    typedef World* WorldPtr;
    typedef const World* WorldConstPtr;

    typedef World& WorldRef;
    typedef const World& WorldConstRef;

    typedef TSharedPtr<World> WorldSharedPtr;
    
    using PostWorldUpdateDelegate = TDelegate<void(WorldConstRef world)>;

    struct PHOENIXSIM_API WorldManagerCtorArgs
    {
        TWeakPtr<Session> Session;
        TWeakPtr<FeatureSet> FeatureSet;
        PostWorldUpdateDelegate OnPostWorldUpdate;
        nlohmann::json Config;
    };

    struct PHOENIXSIM_API NewWorldArgs
    {
        FName WorldType;
        TOptional<FName> Id;
        TOptional<nlohmann::json> Config;
    };

    struct PHOENIXSIM_API WorldStepArgs
    {
        simtime_t SimTime = 0;
        uint32 StepHz = 0;
        FName WorldName = FName::None;
    };

    struct PHOENIXSIM_API WorldSendActionArgs
    {
        Action Action;
        FName WorldName = FName::None;
    };

    struct PHOENIXSIM_API WorldDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(WorldDynamicBlock)
        Time SimTime = 0;
    };

    class PHOENIXSIM_API WorldManager
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

        bool LoadConfig(const nlohmann::json& config);
        bool LoadWorldConfig(const PHXString& worldType, const PHXString& configPath);

        FName GenerateNewWorldId(const FName& worldType);

        void InitializeWorld(WorldRef world) const;
        void ShutdownWorld(WorldRef world) const;
        void UpdateWorld(WorldRef world, simtime_t time, clock_t stepHz) const;
        void SendActionToWorld(WorldRef world, const Action& action) const;

        TWeakPtr<Session> Session;
        TWeakPtr<FeatureSet> FeatureSet;
        TArray<WorldSharedPtr> Worlds;
        BlockBuffer::CtorArgs WorldBufferBlockArgs;

        uint32 WorldIdGen = 0;

        nlohmann::json Config;
        TMap<FName, nlohmann::json> WorldConfigs;

        PostWorldUpdateDelegate OnPostWorldUpdate;
    };
}

