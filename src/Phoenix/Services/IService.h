#pragma once

#include <memory>

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class IService : public std::enable_shared_from_this<IService>
    {
        PHX_DECLARE_TYPE_INTERFACE(IService)

    public:
        virtual ~IService() = default;
    };
}
