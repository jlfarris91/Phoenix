#include "SessionDriver.h"

#include "SessionInstance.h"

using namespace Phoenix;

SessionHandle SessionDriver::CreateSession(const SessionCtorArgs& args)
{
    auto handle = ++SessionIdGen;
    auto sessionInstance = std::make_unique<SessionInstance>(handle, args);
    auto result = Sessions.emplace(handle, std::move(sessionInstance));
    if (result.second)
    {
        SessionInstance* instance = result.first->second.get();
        OnSessionCreated.Broadcast(instance);
    }
    return handle;
}

void SessionDriver::DestroySession(SessionHandle handle)
{
    auto iter = Sessions.find(handle);
    if (iter != Sessions.end())
    {
        SessionInstance* instance = iter->second.get();
        instance->Shutdown(true);
        OnSessionDestroyed.Broadcast(instance);
        Sessions.erase(iter);
    }
}

SessionInstance* SessionDriver::FindInstance(SessionHandle handle)
{
    auto iter = Sessions.find(handle);
    return iter != Sessions.end() ? iter->second.get() : nullptr;
}

std::vector<SessionInstance*> SessionDriver::GetSessions() const
{
    std::vector<SessionInstance*> result;
    for (const auto& session : Sessions | std::views::values)
    {
        if (SessionInstance* instance = session.get())
        {
            result.push_back(instance);
        }
    }
    return result;
}
