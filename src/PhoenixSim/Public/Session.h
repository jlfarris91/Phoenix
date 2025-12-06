#pragma once

#include <shared_mutex>
#include <nlohmann/json.hpp>

#include "Features.h"
#include "FPSCalc.h"

namespace Phoenix
{
    class WorldManager;
}

namespace Phoenix
{
    struct PHOENIXSIM_API SessionCtorArgs
    {
        FeatureSetCtorArgs FeatureSetArgs;
        PostWorldUpdateDelegate OnPostWorldUpdate;

        PHXString DataDirectory;
        PHXString ConfigName;
        TOptional<nlohmann::json> CustomConfig;
    };

    struct PHOENIXSIM_API SessionStepArgs
    {
        uint32 StepHz = 60;

        // Optionally only step this world.
        FName WorldName = FName::None;
    };

    class PHOENIXSIM_API Session : public TSharedAsThis<Session>, public BlockBufferOwner<Session>
    {
    public:

        Session() = default;
        ~Session();

        static TSharedPtr<Session> Create(const SessionCtorArgs& args);

        void Initialize();
        void Shutdown();

        void QueueAction(const Action& action);

        void Tick(const SessionStepArgs& args);
        void Step(const SessionStepArgs& args);

        BlockBuffer& GetBuffer();
        const BlockBuffer& GetBuffer() const;

        TSharedPtr<IFeature> GetFeature(const FName& featureId) const;

        template <class TFeature>
        TSharedPtr<TFeature> GetFeature() const
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
        PHXString GetDataDirectory() const;

        // Returns the absolute directory to the Worlds directory.
        PHXString GetWorldsDirectory() const;

        // Returns the absolute directory for a world.
        PHXString GetWorldDirectory(const PHXString& worldType) const;

    private:

        void LoadConfig();
        void ApplyConfig();

        void ProcessActions(simtime_t time);

        void UpdateSession(simtime_t time, uint32 stepHz) const;

        std::filesystem::path DataDirectory;

        PHXString ConfigName;
        nlohmann::json Config;
        TOptional<nlohmann::json> CustomConfig;
        TMap<FName, nlohmann::json> FeatureConfigs;

        TSharedPtr<FeatureSet> FeatureSet;
        TSharedPtr<WorldManager> WorldManager;

        TArray<TTuple<simtime_t, Action>> ActionQueue;
        std::shared_mutex ActionQueueMutex;

        sys_clock_t StartTime;
        sys_clock_t CurrTickTime;
        sys_clock_dur_t AccTickTime = sys_clock_dur_t(0);
        sys_clock_t LastStepTime;
        simtime_t SimTime = 0;

        // Steps per second
        FPSCalc FPSCalc;

        BlockBuffer SessionBuffer;
    };

    using SessionPtr = Session*;
    using SessionConstPtr = const Session*;
    using SessionRef = Session&;
    using SessionConstRef = const Session&;

    template <class T>
    TSharedPtr<T> GetFeature(WorldConstRef world)
    {
        TSharedPtr<Session> session = world.GetSession().lock();
        return session ? session->GetFeature<T>() : TSharedPtr<T>();
    }
}

