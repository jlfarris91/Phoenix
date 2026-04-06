#pragma once

#include <nlohmann/json.hpp>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "Services/Service.h"

namespace Phoenix
{
    class PHOENIX_SIM_API JsonConfig
    {
    public:

        JsonConfig() = default;

        JsonConfig(
            const nlohmann::json& data,
            const nlohmann::json::json_pointer& pointerToData,
            const std::filesystem::path& filePath);

        // Gets the json object representing the configuration.
        const nlohmann::json& GetData() const;

        // Gets the pointer to the data in the file that this config came from.
        const nlohmann::json::json_pointer& GetPointerToData() const;

        // Gets the path to the file that this config came from.
        const std::filesystem::path& GetFilePath();

    protected:

        nlohmann::json Data;
        nlohmann::json::json_pointer PointerToData;
        std::filesystem::path FilePath;
    };

    class PHOENIX_SIM_API FeatureJsonConfig : public JsonConfig
    {
    public:
        using JsonConfig::JsonConfig;
    };

    class PHOENIX_SIM_API WorldJsonConfig : public JsonConfig
    {
    public:

        static bool LoadConfig(const FName& worldType, const std::filesystem::path& filePath, WorldJsonConfig& outConfig);

        bool LoadConfig();

        const FeatureJsonConfig* GetFeatureConfig(const FName& featureId) const;

    private:
        FName WorldType;
        std::unordered_map<FName, FeatureJsonConfig> FeatureConfigs;
    };

    class PHOENIX_SIM_API SessionJsonConfig : public JsonConfig
    {
    public:

        static bool LoadConfig(
            const FName& configName,
            const std::filesystem::path& filePath,
            SessionJsonConfig& outConfig);

        bool LoadConfig();

        const WorldJsonConfig* GetWorldConfig(const FName& worldType) const;
        const FeatureJsonConfig* GetFeatureConfig(const FName& featureId) const;

    private:
        FName ConfigName;
        std::unordered_map<FName, FeatureJsonConfig> FeatureConfigs;
        std::unordered_map<FName, WorldJsonConfig> WorldConfigs;
    };

    class PHOENIX_SIM_API IConfigService : public IService
    {
        PHX_DECLARE_TYPE_DERIVED(IConfigService, IService)

        virtual bool LoadConfig(const std::filesystem::path& dataDir, const std::string& configName) = 0;
        virtual const SessionJsonConfig& GetSessionConfig() const = 0;
        virtual const WorldJsonConfig* GetWorldConfig(const FName& worldType) const = 0;
        virtual const FeatureJsonConfig* GetSessionFeatureConfig(const FName& featureId) const = 0;
        virtual const FeatureJsonConfig* GetWorldFeatureConfig(const FName& worldType, const FName& featureId) const = 0;
    };

    class PHOENIX_SIM_API DefaultConfigService : public IConfigService
    {
        PHX_DECLARE_TYPE_DERIVED(DefaultConfigService, IConfigService)

    public:

        bool LoadConfig(const std::filesystem::path& dataDir, const std::string& configName) override;

        const SessionJsonConfig& GetSessionConfig() const override;

        // Gets the configuration for a world.
        const WorldJsonConfig* GetWorldConfig(const FName& worldType) const override;

        // Gets the config for a feature at the session level.
        const FeatureJsonConfig* GetSessionFeatureConfig(const FName& featureId) const override;

        // Gets the config for a feature for a given world.
        const FeatureJsonConfig* GetWorldFeatureConfig(const FName& worldType, const FName& featureId) const override;

    protected:

        SessionJsonConfig SessionConfig;
    };
}
