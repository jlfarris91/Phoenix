#include "App.h"

#include "Logger.h"
#include "SessionDriver.h"
#include "SessionEditor.h"
#include "SessionInstance.h"

#include "PhoenixSim/Parallel.h"

App& App::Get()
{
    static App app;
    return app;
}

void App::Initialize()
{
#ifndef __EMSCRIPTEN__
    //Profiling::SetProfiler(&GTracyProfiler);

    unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    if (numThreads > 1)
    {
        Phoenix::SetThreadPool("SimThreadPool", numThreads - 1, 1024);
    }
#endif

    Driver = std::make_unique<SessionDriver>();
    Driver->OnSessionCreated.AddStatic(&App::OnSessionCreated);
    Driver->OnSessionDestroyed.AddStatic(&App::OnSessionDestroyed);

    Logger = std::make_shared<::Logger>("./Phoenix.log");
    Phoenix::SetLogger(Logger);
}

void App::Tick()
{
#ifdef __EMSCRIPTEN__
    TickSessions();
#endif
}

void App::Shutdown()
{
    for (auto&& session : Driver->GetSessions())
    {
        session->Shutdown(true);
    }
}

void App::OnAppEvent(const void* eventData)
{
    if (SessionEditor* editor = GetSessionEditor(FocusedEditor))
    {
        editor->OnAppEvent(eventData);
    }
}

SessionDriver* App::GetSessionDriver() const
{
    return Driver.get();
}

std::shared_ptr<Phoenix::ILogger> App::GetLogger() const
{
    return Logger;
}

SessionEditor* App::GetSessionEditor(SessionHandle handle) const
{
    auto iter = Editors.find(handle);
    return iter == Editors.end() ? nullptr : iter->second.get();
}

void App::TickSessions()
{
    for (auto&& session : Driver->GetSessions())
    {
        session->TickSession();
    }
}

void App::OnSessionCreated(SessionInstance* session)
{
    auto editor = GetSessionEditor(session->GetId());
    assert(editor == nullptr);
    if (!editor)
    {
        CreateSessionEditor(session);
    }
}

void App::OnSessionDestroyed(SessionInstance* session)
{
    Editors.erase(session->GetId());
}

void App::CreateSessionEditor(SessionInstance* session)
{
    auto editor = std::make_unique<SessionEditor>(session);
    Editors.emplace(session->GetId(), std::move(editor));
}
