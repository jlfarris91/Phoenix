#if _WIN32

#include <string>
#include <locale>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Logging.h"

std::wstring Phoenix::ToWideString(const std::string& str)
{
    if (str.empty())
    {
        return {};
    }

    // Calculate the needed buffer size (in wchar_t units)
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (size_needed == 0)
    {
        return {}; 
    }

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);

    // Remove the null terminator added by MultiByteToWideChar if necessary
    if (!wstr.empty() && wstr.back() == L'\0')
    {
        wstr.pop_back();
    }

    return wstr;
}

void Phoenix::OSLog(ELogLevel level, const std::string& msg)
{
    OSLog(GetLogWStringWithUnixTime(level, msg).c_str());
}

void Phoenix::OSLog(const wchar_t* wstr)
{
    OutputDebugStringW(wstr);
    OutputDebugStringW(L"\n");
}

std::string Phoenix::GetLogStringWithUnixTime(ELogLevel level, const std::string& msg)
{
    char timeStr[64];
    GetNowUnixTimeString(timeStr, _countof(timeStr));
    return std::format("[{0}] [{1}]: {2}", timeStr, ToString(level), msg);
}

std::wstring Phoenix::GetLogWStringWithUnixTime(ELogLevel level, const std::string& msg)
{
    return ToWideString(GetLogStringWithUnixTime(level, msg));
}

size_t Phoenix::GetNowLocalTimeString(char* buffer, size_t sizeInBytes)
{
    std::time_t currTime = std::time(nullptr);
    std::tm localTime;
    (void)localtime_s(&localTime, &currTime);
    return strftime(buffer, sizeInBytes, "%Y-%m-%d %H:%M:%S", &localTime);
}

size_t Phoenix::GetNowUnixTimeString(char* buffer, size_t sizeInBytes)
{
    std::time_t currTime = std::time(nullptr);
    std::tm localTime;
    (void)gmtime_s(&localTime, &currTime);
    return strftime(buffer, sizeInBytes, "%Y-%m-%d %H:%M:%S", &localTime);
}

#endif
