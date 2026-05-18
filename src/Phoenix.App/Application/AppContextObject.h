#pragma once

#include <memory>

namespace Phoenix
{
    class Application;

    class IAppContextObject
    {
    public:
        virtual ~IAppContextObject() = default;
        virtual std::shared_ptr<Application> GetApplication() const = 0;
    };

    class AppContextObject : public IAppContextObject
    {
    public:
        virtual std::shared_ptr<Application> GetApplication() const override;
    protected:
        std::weak_ptr<Application> WeakApp;
    };
}
