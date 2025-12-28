#include "Logger.h"

using namespace Phoenix;

Logger::Logger(const std::filesystem::path& logFilePath)
{
    LogFileStream.open(logFilePath);
}

Logger::~Logger()
{
    LogFileStream.close();
}

void Logger::Log(ELogLevel level, const std::string& msg)
{
    std::wstring wmsg = GetLogWStringWithUnixTime(level, msg);
    OSLog(wmsg.c_str());

    if (OnLogCallback)
    {
        OnLogCallback(level, msg);
    }

    if (LogFileStream.is_open())
    {
        LogFileStream << wmsg;
        LogFileStream << L'\n';
        LogFileStream.flush();
    }
}

void Logger::OnLog(const LogCallback& callback)
{
    OnLogCallback = callback;
}
