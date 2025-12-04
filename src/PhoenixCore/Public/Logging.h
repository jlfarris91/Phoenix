
#pragma once

#include "Platform.h"

namespace Phoenix
{
    enum class ELogLevel : uint8
    {
        Verbose,
        Info,
        Warning,
        Error,
        COUNT
    };

    struct LogMessage
    {
        PHXString Message;
    };

    template <class TLog = LogMessage>
    struct LogCollection
    {
        const auto& GetLogs() const
        {
            return Logs;
        }

        const TArray<TLog>& GetLogs(ELogLevel level) const
        {
            return Logs[static_cast<uint8>(level)];
        }
        
        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogVerbose(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Log(ELogLevel::Verbose, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogInfo(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Log(ELogLevel::Info, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogWarning(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Log(ELogLevel::Warning, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogError(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Log(ELogLevel::Error, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& Log(
            ELogLevel level,
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            PHX_ASSERT(static_cast<uint8>(level) < static_cast<uint8>(ELogLevel::COUNT));
            PHXString message = std::format(fmt, std::forward<TFmtArgs>(fmtArgs)...);
            Logs[static_cast<uint8>(level)].push_back({{message}});
            return Logs[static_cast<uint8>(level)].back();
        }

    private:

        TArray<TLog> Logs[static_cast<uint8>(ELogLevel::COUNT)];
    };

    template <class TLog = LogMessage>
    class ILogger
    {
    public:

        const LogCollection<TLog>& GetLogCollection() const
        {
            return Logs;
        }

        const auto& GetLogs() const
        {
            return Logs.GetLogs();
        }

        const TArray<TLog>& GetLogs(ELogLevel level) const
        {
            return Logs.GetLogs(level);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogVerbose(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Logs.LogVerbose(fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogInfo(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Logs.LogInfo(fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogWarning(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Logs.LogWarning(fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& LogError(
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Logs.LogError(fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE TLog& Log(
            ELogLevel level,
            const std::format_string<TFmtArgs...>& fmt,
            TFmtArgs&&... fmtArgs)
        {
            return Logs.Log(level, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

    private:

        LogCollection<TLog> Logs;
    };
}
