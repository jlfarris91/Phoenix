#pragma once

#include <memory>

namespace Phoenix
{
    class Session;
    class SessionEditor;
    class SessionManager;
    struct NewSessionArgs;

    class ISessionBuilder : public std::enable_shared_from_this<ISessionBuilder>
    {
    public:
        virtual ~ISessionBuilder() = default;

        // Returns true if the builder can construct a new session given the args.
        virtual bool CanBuildSession(const NewSessionArgs& args) const = 0;

        virtual std::shared_ptr<Session> BuildSession(
            const std::shared_ptr<SessionManager>& sessionManager,
            const std::shared_ptr<SessionEditor>& sessionEditor,
            const NewSessionArgs& args) = 0;
    };
}
