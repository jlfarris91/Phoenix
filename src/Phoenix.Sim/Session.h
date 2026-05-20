#pragma once

#include <shared_mutex>
#include <nlohmann/json.hpp>

#include "Phoenix.Sim/BlockBuffer/BlockBufferOwner.h"
#include "Phoenix/Containers/Optional.h"
#include "Phoenix.Sim/Features.h"
#include "Phoenix/FPSCalc.h"
#include "Phoenix/Services/ServiceContainer.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"
#include "Phoenix.Sim/Worlds.h"

namespace Phoenix
{
    class WorldManager;
    class IFeature;
    class IConfigService;

    struct PHOENIX_SIM_API SessionCtorArgs
    {
        std::string DataDirectory;
        std::string ConfigName;

        // Directory where runtime binaries (e.g. lua.wasm) are located.
        // Typically the directory containing the application executable.
        // If empty, falls back to DataDirectory.
        std::string BinDirectory;
        TOptional<nlohmann::json> CustomConfig;

        std::shared_ptr<Phoenix::ServiceContainerBuilder> ServiceContainerBuilder;

        PostWorldUpdateDelegate OnPostWorldUpdate;
    };

    struct PHOENIX_SIM_API SessionLayoutContext
    {
        SessionJsonConfig Config;
    };

    struct PHOENIX_SIM_API SessionStepArgs
    {
        double SpeedMultiplier = 1.0;

        // Optionally only step this world.
        FName WorldName = FName::None;
    };

    class PHOENIX_SIM_API Session : public std::enable_shared_from_this<Session>
                                  , public BlockBufferOwner<Session>
                                  , public Phoenix::ServiceContainerOwner
    {
    public:

        Session() = default;
        ~Session() override;

        static std::shared_ptr<Session> Create(const SessionCtorArgs& args);

        void Initialize();
        void Shutdown();

        void EnqueueAction(const Action& action);

        void Tick(const SessionStepArgs& args);
        void Step(const SessionStepArgs& args);

        BlockBuffer& GetBuffer();
        const BlockBuffer& GetBuffer() const;

        std::shared_ptr<IFeature> GetFeature(const FName& featureId) const;

        template <class TFeature>
        std::shared_ptr<TFeature> GetFeature() const
        {
            return FeatureSet->GetFeature<TFeature>();
        }

        sys_clock_t GetCurrTime() const;
        sys_clock_t GetStartTime() const;
        sys_clock_t GetLastStepTime() const;
        simtime_t GetSimTime() const;
        const FPSCalc& GetFPSCalc() const;

        FeatureSet* GetFeatureSet() const;
        WorldManager* GetWorldManager() const;

        // Returns the absolute directory to the session data.
        std::string GetDataDirectory() const;

        // Returns the absolute directory to the Worlds directory.
        std::string GetWorldsDirectory() const;

        // Returns the absolute directory for a world.
        std::string GetWorldDirectory(const std::string& worldType) const;

    private:

        void LoadConfig() const;
        void ApplyConfig() const;

        void ProcessActions(simtime_t time);

        void UpdateSession(simtime_t time) const;

        std::filesystem::path DataDirectory;

        std::string ConfigName;
        std::shared_ptr<IConfigService> ConfigService;
        std::shared_ptr<FeatureSet> FeatureSet;
        std::shared_ptr<WorldManager> WorldManager;

        std::vector<std::tuple<simtime_t, Action>> ActionQueue;
        std::shared_mutex ActionQueueMutex;

        sys_clock_t StartTime;
        sys_clock_t CurrTickTime;
        sys_clock_t LastStepTime;
        simtime_t SimTime = 0;

        // Steps per second
        FPSCalc FPSCalc;

        BlockBuffer SessionBuffer;
    };

    template <class T>
    std::shared_ptr<T> GetFeature(WorldConstRef world)
    {
        std::shared_ptr<Session> session = world.GetSession().lock();
        return session ? session->GetFeature<T>() : std::shared_ptr<T>();
    }
}
