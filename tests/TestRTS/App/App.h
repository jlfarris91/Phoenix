#pragma once

#include <memory>
#include <unordered_map>

#include "SessionHandle.h"

class SessionEditor;
class SessionInstance;

namespace Phoenix
{
    class ILogger;
}

class SessionDriver;

class App
{
public:

    static App& Get();

    void Initialize();

    void Tick();

    void Shutdown();

    void OnAppEvent(const void* eventData);

    SessionDriver* GetSessionDriver() const;
    std::shared_ptr<Phoenix::ILogger> GetLogger() const;

    SessionEditor* GetSessionEditor(SessionHandle handle) const;

private:

    void TickSessions();

    void OnSessionCreated(SessionInstance* session);
    void OnSessionDestroyed(SessionInstance* session);

    void CreateSessionEditor(SessionInstance* session);

    std::unique_ptr<SessionDriver> Driver;
    std::shared_ptr<Phoenix::ILogger> Logger;
    std::unordered_map<SessionHandle, std::unique_ptr<SessionEditor>> Editors;
    SessionHandle FocusedEditor = 0;
};
