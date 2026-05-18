#pragma once

#include <unordered_map>
#include "Phoenix/Hashing.h"
#include "Phoenix/Reflection/TypeName.h"

#include "Delegates.h"

namespace Phoenix
{
    class MessageRouter
    {
    public:

        virtual ~MessageRouter() = default;

        typedef Phoenix::hash32_t type_id;
        typedef std::function<bool(type_id messageTypeId, const void* message)> HandleMessageFunc;

        struct Handler
        {
            DelegateHandle Handle;
            HandleMessageFunc Func;
        };

        template <class TMessage, class ...TArgs>
        DelegateHandle Register(const TDelegate<bool(const TMessage&, TArgs...)>& handler)
        {
            constexpr auto typeId = Phoenix::StaticTypeName<TMessage>::TypeId;
            auto wrapper = [handler](const void* data) -> bool
            {
                return handler(static_cast<const TMessage*>(data));
            };
            Handlers[(type_id)typeId].push_back({ handler.GetHandler(), wrapper });
            return handler.GetHandle();
        }

        void Unregister(const DelegateHandle& handle);

        // Routes a message to the appropriate handler(s).
        template <class TMessage>
        bool SendMessage(const TMessage& message)
        {
            constexpr auto typeId = Phoenix::StaticTypeName<TMessage>::TypeId;
            return this->SendMessage((type_id)typeId, &message);
        }

    protected:

        virtual bool SendMessage(type_id typeId, const void* data) = 0;

        bool AcceptMessage(type_id typeId, const void* data) const;

    private:

        std::unordered_map<type_id, std::vector<Handler>> Handlers;
    };
}
