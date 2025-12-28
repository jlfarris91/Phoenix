#pragma once

#include <filesystem>
#include <fstream>

#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Containers/FixedArray.h"

class Logger : public Phoenix::ILogger
{
public:

    using LogCallback = std::function<void(Phoenix::ELogLevel level, const std::string& msg)>;

    Logger(const std::filesystem::path& logFilePath);
    ~Logger() override;

    void Log(Phoenix::ELogLevel level, const std::string& msg) override;

    void OnLog(const LogCallback& callback);

private:

    std::wofstream LogFileStream;
    LogCallback OnLogCallback;
};
