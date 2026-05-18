#include "Logging.h"

using namespace Phoenix;

class DefaultLogger : public ILogger
{
public:
    void Log(ELogLevel level, const std::string& msg) override
    {
        OSLog(level, msg);
    }
} gDefaultLogger;

std::shared_ptr<ILogger> gPrimaryLogger;
std::unordered_map<std::string, std::shared_ptr<ILogger>> gSecondaryLoggers;

bool Phoenix::HasLogger(const std::string& id)
{
    return id.empty() || gSecondaryLoggers.contains(id); 
}

ILogger& Phoenix::GetLogger()
{
    return gPrimaryLogger ? *gPrimaryLogger : gDefaultLogger;
}

ILogger& Phoenix::GetLogger(const std::string& id)
{
    if (id.empty())
    {
        return GetLogger();
    }
    return *gSecondaryLoggers[id];
}

void Phoenix::SetLogger(const std::shared_ptr<ILogger>& logger)
{
    gPrimaryLogger = logger;
}

void Phoenix::SetLogger(const std::shared_ptr<ILogger>& logger, const std::string& id)
{
    if (id.empty())
    {
        SetLogger(logger);
    }
    else
    {
        gSecondaryLoggers.emplace(id, logger);
    }
}
