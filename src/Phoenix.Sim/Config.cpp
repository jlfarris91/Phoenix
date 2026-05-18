#include "Phoenix.Sim/Config.h"

#include <fstream>

#include "Phoenix/Logging.h"

using namespace Phoenix;

JsonConfig::JsonConfig(
    const nlohmann::json& data,
    const nlohmann::json::json_pointer& pointerToData,
    const std::filesystem::path& filePath)
    : Data(data)
    , PointerToData(pointerToData)
    , FilePath(filePath)
{
    
}

const nlohmann::json& JsonConfig::GetData() const
{
    return Data;
}

const std::filesystem::path& JsonConfig::GetFilePath()
{
    return FilePath;
}

const nlohmann::json::json_pointer& JsonConfig::GetPointerToData() const
{
    return PointerToData;
}

bool WorldJsonConfig::LoadConfig(
    const FName& worldType,
    const std::filesystem::path& filePath,
    WorldJsonConfig& outConfig)
{
    outConfig.WorldType = worldType;
    outConfig.FilePath = filePath;
    return outConfig.LoadConfig();
}

bool WorldJsonConfig::LoadConfig()
{
    Data.clear();
    FeatureConfigs.clear();

    LogVerbose("Loading world config at path {0}", FilePath.string());

    std::ifstream configStream(FilePath);
    if (!configStream.is_open())
    {
        LogWarning("Failed to load world config at path {0}", FilePath.string());
        return false;
    }

    try
    {
        Data = nlohmann::json::parse(configStream);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        LogError("Failed to parse world config file: {}", FilePath.string(), ex.what());
        return false;
    }

    nlohmann::json::json_pointer featuresPtr = "/features"_json_pointer;
    if (Data.contains(featuresPtr))
    {
        const nlohmann::json& featuresJson = Data[featuresPtr];
        for (auto && [featureTypeStr, featureConfig] : featuresJson.items())
        {
            nlohmann::json::json_pointer pointer = featuresPtr / featureTypeStr;
            FeatureConfigs.emplace(FName(featureTypeStr), FeatureJsonConfig(featureConfig, pointer, FilePath));
        }
    }

    return true;
}

const FeatureJsonConfig* WorldJsonConfig::GetFeatureConfig(const FName& featureId) const
{
    auto configIter = FeatureConfigs.find(featureId);
    return configIter != FeatureConfigs.end() ? &configIter->second : nullptr;
}

bool SessionJsonConfig::LoadConfig(
    const FName& configName,
    const std::filesystem::path& filePath,
    SessionJsonConfig& outConfig)
{
    outConfig.ConfigName = configName;
    outConfig.FilePath = filePath;
    return outConfig.LoadConfig();
}

bool SessionJsonConfig::LoadConfig()
{
    Data.clear();
    WorldConfigs.clear();
    FeatureConfigs.clear();

    LogVerbose("Loading session config at path {0}", FilePath.string());

    std::ifstream configStream(FilePath);
    if (!configStream.is_open())
    {
        LogWarning("Failed to load session config at path {0}", FilePath.string());
        return false;
    }

    try
    {
        Data = nlohmann::json::parse(configStream);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        LogError("Failed to parse session config file: {}", FilePath.string(), ex.what());
        return false;
    }

    nlohmann::json::json_pointer featuresPtr = "/features"_json_pointer;
    if (Data.contains(featuresPtr))
    {
        const nlohmann::json& featuresJson = Data[featuresPtr];
        for (auto && [featureTypeStr, featureConfig] : featuresJson.items())
        {
            nlohmann::json::json_pointer pointer = featuresPtr / featureTypeStr;
            FeatureConfigs.emplace(FName(featureTypeStr), FeatureJsonConfig(featureConfig, pointer, FilePath));
        }
    }

    auto worldsIter = Data.find("worlds");
    if (worldsIter != Data.end())
    {
        for (auto && [worldTypeStr, worldConfig] : worldsIter->items())
        {
            auto configIter = worldConfig.find("config");
            if (configIter == worldConfig.end())
            {
                LogWarning("Expected property 'config' with a relative path to a world config json file.");
                continue;
            }

            std::filesystem::path parentPath = FilePath.parent_path();
            std::filesystem::path configFilePath = absolute(parentPath / configIter->get<std::string>());

            WorldJsonConfig worldJsonConfig;
            if (WorldJsonConfig::LoadConfig(worldTypeStr, configFilePath, worldJsonConfig))
            {
                WorldConfigs[FName(worldTypeStr)] = worldJsonConfig;
            }
        }
    }

    return true;
}

const WorldJsonConfig* SessionJsonConfig::GetWorldConfig(const FName& worldType) const
{
    auto configIter = WorldConfigs.find(worldType);
    return configIter != WorldConfigs.end() ? &configIter->second : nullptr;
}

const FeatureJsonConfig* SessionJsonConfig::GetFeatureConfig(const FName& featureId) const
{
    auto configIter = FeatureConfigs.find(featureId);
    return configIter != FeatureConfigs.end() ? &configIter->second : nullptr;
}

bool DefaultConfigService::LoadConfig(const std::filesystem::path& dataDir, const std::string& configName)
{
    std::filesystem::path sessionConfigPath = absolute(dataDir / (configName + ".json"));
    return SessionJsonConfig::LoadConfig(configName, sessionConfigPath, SessionConfig);
}

const SessionJsonConfig& DefaultConfigService::GetSessionConfig() const
{
    return SessionConfig;
}

const WorldJsonConfig* DefaultConfigService::GetWorldConfig(const FName& worldType) const
{
    return SessionConfig.GetWorldConfig(worldType);
}

const FeatureJsonConfig* DefaultConfigService::GetSessionFeatureConfig(const FName& featureId) const
{
    return SessionConfig.GetFeatureConfig(featureId);
}

const FeatureJsonConfig* DefaultConfigService::GetWorldFeatureConfig(const FName& worldType, const FName& featureId) const
{
    const WorldJsonConfig* worldConfig = GetWorldConfig(worldType);
    return worldConfig ? worldConfig->GetFeatureConfig(featureId) : nullptr;
}
