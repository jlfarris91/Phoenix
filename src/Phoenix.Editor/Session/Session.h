#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace Phoenix
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:

        struct ConstructionParams
        {
            // The type of session like "Map" or "Catalog".
            std::string SessionType;

            // A unique identifier for this session, used for saving/loading and distinguishing between multiple sessions of the same type.
            std::string SessionId;

            // A non-unique name for this session, used for display purposes. Multiple sessions can have the same name.
            std::string SessionName;

            // The root directory for this session, used for saving/loading.
            std::filesystem::path RootDirectory;
        };

        Session(const ConstructionParams& params);

        const std::string& GetType() const;
        const std::string& GetId() const;
        const std::string& GetName() const;
        const std::string& GetDisplayName() const;
        const std::filesystem::path& GetRootDirectory() const;

        bool IsLoaded() const;
        bool IsLoading() const;
        bool IsSaving() const;

        // Returns true if the session has never been saved before, and thus should be treated as a new session.
        // This is used to determine whether to prompt the user to save changes when closing the session, among other things.
        bool IsNewSession() const;

        // Converts a path relative to the session's root directory to an absolute path.
        std::filesystem::path MakeAbsolutePath(const std::filesystem::path& relativePath) const;

        // Converts an absolute path to a path relative to the session's root directory.
        // If the absolute path is not within the session's root directory, returns an empty path.
        std::filesystem::path MakeRelativePath(const std::filesystem::path& absolutePath) const;

    private:

        friend class SessionEditor;

        void OnSessionLoaded(bool succeeded);
        void OnSessionSaved(bool succeeded);

        std::string Type;
        std::string Id;
        std::string Name;
        std::filesystem::path RootDirectory;

        std::optional<bool> LastLoadResult;
        bool bIsLoading = false;
        bool bIsLoaded = false;

        std::optional<bool> LastSaveResult;
        bool bIsSaving = false;
    };
}
