#pragma once

#include <memory>

namespace Phoenix
{
    class Session;
    class SessionEditor;

    class ISessionContextObject
    {
    public:
        virtual ~ISessionContextObject() = default;
        virtual std::shared_ptr<Session> GetSession() const = 0;
        virtual std::shared_ptr<SessionEditor> GetSessionEditor() const = 0;
    };

    class SessionContextObject : public ISessionContextObject
    {
    public:
        virtual std::shared_ptr<Session> GetSession() const override;
        virtual std::shared_ptr<SessionEditor> GetSessionEditor() const override;
    protected:
        std::weak_ptr<SessionEditor> WeakSessionEditor;
    };
}