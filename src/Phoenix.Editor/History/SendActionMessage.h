#pragma once

#include <memory>

#include "Messaging/Message.h"
#include "History/Action.h"

namespace Phoenix
{
    class SendActionMessage : public Message
    {
        PHX_DECLARE_TYPE_DERIVED(SendActionMessage, Message)
    public:
        std::shared_ptr<Action> Action;
    };
}
