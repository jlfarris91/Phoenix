#pragma once

#include "Phoenix/Reflection/Registration.h"

namespace Phoenix
{
    class Action;

    class Message
    {
        PHX_DECLARE_TYPE_INTERFACE(Message)
    public:
        virtual ~Message() = default;
    };
}
