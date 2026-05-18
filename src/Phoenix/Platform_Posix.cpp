#if !defined(_WIN32)

#include <cstdio>
#include <ctime>

#include "Phoenix/Platform.h"
#include "Phoenix/Logging.h"

std::string Phoenix::GetLogStringWithUnixTime(ELogLevel level, const std::string& msg)
{
    char timeStr[64];
    GetNowUnixTimeString(timeStr, sizeof(timeStr));
    return std::format("[{0}] [{1}]: {2}", timeStr, ToString(level), msg);
}

void Phoenix::OSLog(ELogLevel level, const std::string& msg)
{
    fprintf(stderr, "%s\n", GetLogStringWithUnixTime(level, msg).c_str());
}

size_t Phoenix::GetNowLocalTimeString(char* buffer, size_t sizeInBytes)
{
    std::time_t currTime = std::time(nullptr);
    std::tm localTime = {};
    localtime_r(&currTime, &localTime);
    return strftime(buffer, sizeInBytes, "%Y-%m-%d %H:%M:%S", &localTime);
}

size_t Phoenix::GetNowUnixTimeString(char* buffer, size_t sizeInBytes)
{
    std::time_t currTime = std::time(nullptr);
    std::tm localTime = {};
    gmtime_r(&currTime, &localTime);
    return strftime(buffer, sizeInBytes, "%Y-%m-%d %H:%M:%S", &localTime);
}

#endif
