#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Session.h"
#include "Editor/EditorService.h"

namespace Phoenix
{
    struct NewSessionArgs;
    class SessionModuleFactory;

    struct SaveSessionResult
    {
        std::shared_ptr<Session> Session;
        bool Succeeded = false;
    };

    using SessionSavedCallback = std::function<void(const SaveSessionResult& result)>;

    class SessionManager : public std::enable_shared_from_this<SessionManager>
                         , public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(SessionManager, IEditorService)

    public:

        // Begin IApplicationService implementation
        virtual void Initialize(const std::shared_ptr<Phoenix::Editor>& editor) override;
        virtual void Shutdown() override;
        // End IApplicationService implementation

        void RegisterSessionSpawner(const std::string& sessionType, const SessionModuleFactory& factory);
        bool UnregisterSessionSpawner(const std::string& sessionType);
        uint32_t UnregisterAllSessionSpawners();

        std::shared_ptr<Session> OpenSession(const std::string& sessionType, const std::string& rootDirectory);

        std::shared_ptr<Session> NewSession(const NewSessionArgs& args);

        bool RequestCloseSession(const std::string& sessionId);

        bool RequestCloseSession(const std::shared_ptr<Session>& session);

        bool RequestCloseAllSessions();
        
        std::shared_ptr<Session> RequestReloadSession(const std::shared_ptr<Session>& session);

        uint32_t RequestReloadSessions(
            const std::vector<std::shared_ptr<Session>>& sessions,
            std::vector<std::shared_ptr<Session>>& outReloadedSessions);

        uint32_t RequestReloadAllSessions(std::vector<std::shared_ptr<Session>>& outReloadedSessions);

        std::shared_ptr<Session> GetActiveSession() const;

        std::shared_ptr<SessionEditor> GetActiveSessionEditor() const;
        bool SetActiveSessionEditor(const std::shared_ptr<SessionEditor>& editor);

        std::shared_ptr<Session> FindSession(const std::string& sessionId) const;
        std::shared_ptr<Session> FindSessionByName(const std::string& sessionName) const;

        std::vector<std::shared_ptr<Session>>& GetActiveSessions();

        bool HasAnyActiveSessions() const;

        bool SaveSession(const std::shared_ptr<Session>& session, const SessionSavedCallback& callback = {});

        bool SaveSessionAs(const std::shared_ptr<Session>& session, const SessionSavedCallback& callback = {});

        bool SaveSessionAs(
            const std::shared_ptr<Session>& session,
            const std::filesystem::path& newRootDirectory,
            const SessionSavedCallback& callback = {});

        bool SaveAllSessions(const SessionSavedCallback& callback = {});

    private:

        friend class Editor;

        // Generates a unique session ID. Not meant for display.
        std::string GenerateUniqueSessionId() const;

        std::string GenerateUniqueSessionName(const std::string& baseName) const;
        
        std::shared_ptr<Session> SpawnSession(
            const Session::ConstructionParams& params,
            const std::vector<void*>& loadSessionContextObjects);

        uint32_t UniqueSessionIdGen = 0;

        std::unordered_map<std::string, std::weak_ptr<SessionModuleFactory>> ModuleFactories;

        std::vector<std::shared_ptr<Session>> Sessions;
        std::unordered_map<std::string, std::weak_ptr<Session>> SessionMap;
        std::unordered_map<std::string, std::weak_ptr<Session>> SessionByDirectoryMap;

        std::shared_ptr<Session> ActiveSession;

        std::vector<std::shared_ptr<Session>> SessionsPendingCloseAfterSave;
        std::unordered_map<std::weak_ptr<Session>, SessionSavedCallback> SessionSaveCallbacks;
    };
}
