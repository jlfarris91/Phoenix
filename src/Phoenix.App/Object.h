#pragma once

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class IObject : public std::enable_shared_from_this<IObject>
    {
        PHX_DECLARE_TYPE_INTERFACE(IObject)

    public:
        virtual ~IObject() = default;
    };
}
