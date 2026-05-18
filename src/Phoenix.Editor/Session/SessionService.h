#pragma once
#include <memory>

#include "SessionContextObject.h"
#include "Services/Service.h"

namespace Phoenix
{
    class ISessionService : public IService, public SessionContextObject
    {
        PHX_DECLARE_TYPE_DERIVED(ISessionService, IService)
    public:

        virtual void Initialize(const std::shared_ptr<SessionEditor>& sessionEditor);
        virtual void Shutdown();
    };
}
