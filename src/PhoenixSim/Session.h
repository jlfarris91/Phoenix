#pragma once

#include <shared_mutex>
#include <nlohmann/json.hpp>

#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/FPSCalc.h"
#include "PhoenixSim/Services/ServiceLocator.h"
#include "PhoenixSim/Worlds.h"

namespace Phoenix
{
    class ServiceContainer;
    class WorldManager;
    class IFeature;
    class ServiceContainerBuilder;
    class IConfigService;

    struct PHOENIX_SIM_API SessionCtorArgs
    {
        PHXString DataDirectory;
        PHXString ConfigName;
        TOptional<nlohmann::json> CustomConfig;

        TSharedPtr<ServiceContainerBuilder> ServiceContainerBuilder;

        PostWorldUpdateDelegate OnPostWorldUpdate;
    };

    struct PHOENIX_SIM_API SessionStepArgs
    {
        uint32 StepHz = 60;

        // Optionally only step this world.
        FName WorldName = FName::None;
    };

    class PHOENIX_SIM_API Session : public TSharedAsThis<Session>
                                  , public BlockBufferOwner<Session>
                                  , public ServiceLocator<Session>
    {
    public:

        Session() = default;
        ~Session() override;

        static TSharedPtr<Session> Create(const SessionCtorArgs& args);

        void Initialize();
        void Shutdown();

        void EnqueueAction(const Action& action);

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
        ServiceContainer* GetServiceContainer() const;

        // Returns the absolute directory to the session data.
        PHXString GetDataDirectory() const;

        // Returns the absolute directory to the Worlds directory.
        PHXString GetWorldsDirectory() const;

        // Returns the absolute directory for a world.
        PHXString GetWorldDirectory(const PHXString& worldType) const;

        // Begin ServiceLocator implementation
        TSharedPtr<IService> GetService(const FName& typeId) const override;
        uint32 GetServices(const FName& typeId, TVector<TSharedPtr<IService>>& outServices) const override;
        const TVector<TSharedPtr<IService>>& GetServices() const override;
        // End ServiceLocator implementation

    private:

        void LoadConfig() const;
        void ApplyConfig() const;

        void ProcessActions(simtime_t time);

        void UpdateSession(simtime_t time, uint32 stepHz) const;

        std::filesystem::path DataDirectory;

        PHXString ConfigName;
        TSharedPtr<IConfigService> ConfigService;

        TSharedPtr<FeatureSet> FeatureSet;
        TSharedPtr<WorldManager> WorldManager;
        TSharedPtr<ServiceContainer> ServiceContainer;

        TVector<TTuple<simtime_t, Action>> ActionQueue;
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

    template <class T>
    TSharedPtr<T> GetFeature(WorldConstRef world)
    {
        TSharedPtr<Session> session = world.GetSession().lock();
        return session ? session->GetFeature<T>() : TSharedPtr<T>();
    }
}

