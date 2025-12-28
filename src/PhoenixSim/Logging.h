
#pragma once

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API ELogLevel : uint8
    {
        Verbose,
        Info,
        Warning,
        Error,
        COUNT
    };

    PHX_FORCE_INLINE const char* ToString(ELogLevel level)
    {
        switch (level)
        {
            case ELogLevel::Verbose:    return "Verbose";
            case ELogLevel::Info:       return "Info";
            case ELogLevel::Warning:    return "Warn";
            case ELogLevel::Error:      return "Error";
            default:                    return "";
        }
    }

    void PHOENIX_SIM_API OSLog(ELogLevel level, const std::string& msg);
    void PHOENIX_SIM_API OSLog(const wchar_t* msg);

    std::string PHOENIX_SIM_API GetLogStringWithUnixTime(ELogLevel level, const std::string& msg);
    std::wstring PHOENIX_SIM_API GetLogWStringWithUnixTime(ELogLevel level, const std::string& msg);

    class PHOENIX_SIM_API ILogger
    {
    public:

        virtual ~ILogger() = default;

        virtual void Log(ELogLevel level, const std::string& msg) = 0;

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE void LogVerbose(const std::format_string<TFmtArgs...>& fmt, TFmtArgs&&... fmtArgs)
        {
            Log(ELogLevel::Verbose, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE void LogInfo(const std::format_string<TFmtArgs...>& fmt, TFmtArgs&&... fmtArgs)
        {
            Log(ELogLevel::Info, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE void LogWarning(const std::format_string<TFmtArgs...>& fmt, TFmtArgs&&... fmtArgs)
        {
            Log(ELogLevel::Warning, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE void LogError(const std::format_string<TFmtArgs...>& fmt, TFmtArgs&&... fmtArgs)
        {
            Log(ELogLevel::Error, fmt, std::forward<TFmtArgs>(fmtArgs)...);
        }

        template <class ...TFmtArgs>
        PHX_FORCE_INLINE void Log(ELogLevel level, const std::format_string<TFmtArgs...>& fmt, TFmtArgs&&... fmtArgs)
        {
            PHXString message = std::format(fmt, std::forward<TFmtArgs>(fmtArgs)...);
            Log(level, message);
        }
    };

    PHOENIX_SIM_API bool HasLogger(const std::string& id);
    PHOENIX_SIM_API ILogger& GetLogger();
    PHOENIX_SIM_API ILogger& GetLogger(const std::string& id);
    PHOENIX_SIM_API void SetLogger(const TSharedPtr<ILogger>& logger);
    PHOENIX_SIM_API void SetLogger(const TSharedPtr<ILogger>& logger, const std::string& id);

    template <class ...TFmtArgs>
    PHX_FORCE_INLINE void LogVerbose(
        const std::format_string<TFmtArgs...>& fmt,
        TFmtArgs&&... fmtArgs)
    {
        Log(ELogLevel::Verbose, fmt, std::forward<TFmtArgs>(fmtArgs)...);
    }

    template <class ...TFmtArgs>
    PHX_FORCE_INLINE void LogInfo(
        const std::format_string<TFmtArgs...>& fmt,
        TFmtArgs&&... fmtArgs)
    {
        Log(ELogLevel::Info, fmt, std::forward<TFmtArgs>(fmtArgs)...);
    }

    template <class ...TFmtArgs>
    PHX_FORCE_INLINE void LogWarning(
        const std::format_string<TFmtArgs...>& fmt,
        TFmtArgs&&... fmtArgs)
    {
        Log(ELogLevel::Warning, fmt, std::forward<TFmtArgs>(fmtArgs)...);
    }

    template <class ...TFmtArgs>
    PHX_FORCE_INLINE void LogError(
        const std::format_string<TFmtArgs...>& fmt,
        TFmtArgs&&... fmtArgs)
    {
        Log(ELogLevel::Error, fmt, std::forward<TFmtArgs>(fmtArgs)...);
    }

    template <class ...TFmtArgs>
    PHX_FORCE_INLINE void Log(
        ELogLevel level,
        const std::format_string<TFmtArgs...>& fmt,
        TFmtArgs&&... fmtArgs)
    {
        PHX_ASSERT(static_cast<uint8>(level) < static_cast<uint8>(ELogLevel::COUNT));
        PHXString message = std::format(fmt, std::forward<TFmtArgs>(fmtArgs)...);
        GetLogger().Log(level, message);
    }

    struct PHOENIX_SIM_API LogMessage
    {
        PHXString Message;
    };

    template <class TLog = LogMessage>
    struct LogCollection
    {
        const std::string& GetLoggerId() const
        {
            return LoggerId;
        }

        void SetLoggerId(const std::string& id)
        {
            LoggerId = id;
        }

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
            GetLogger(LoggerId).Log(level, message);
            return Logs[static_cast<uint8>(level)].back();
        }

    private:

        std::string LoggerId = {};
        TArray<TLog> Logs[static_cast<uint8>(ELogLevel::COUNT)];
    };

    template <class TLog = LogMessage>
    class ILogCollectionOwner
    {
    public:

        const std::string& GetLoggerId() const
        {
            return Logs.GetLoggerId();
        }

        void SetLoggerId(const std::string& id)
        {
            Logs.SetLoggerId(id);
        }

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