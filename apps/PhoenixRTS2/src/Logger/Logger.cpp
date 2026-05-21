#include "Logger.h"

using namespace Phoenix;

Logger::Logger(const std::filesystem::path& logFilePath)
{
    LogFileStream.open(logFilePath);
}

void Logger::Log(ELogLevel level, const std::string& msg)
{
    std::scoped_lock lock(LogMutex);

    std::wstring wmsg = GetLogWStringWithUnixTime(level, msg);
    OSLog(wmsg.c_str());

    if (OnLogCallback)
    {
        OnLogCallback(level, msg);
    }

    if (LogFileStream.is_open())
    {
        std::string msgWithTime = GetLogStringWithUnixTime(level, msg);
        LogFileStream << msgWithTime << '\n';
        if (LogFileStream.fail())
        {
            __debugbreak();
        }
        LogFileStream.flush();
    }
}

void Logger::OnLog(const LogCallback& callback)
{
    OnLogCallback = callback;
}
