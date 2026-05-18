#include "MessageRouter.h"

#include <algorithm>

void Phoenix::MessageRouter::Unregister(const DelegateHandle& handle)
{
    for (auto i = Handlers.begin(); i != Handlers.end();)
    {
        auto& handlers = i->second;
        for (auto j = handlers.begin(); j != handlers.end();)
        {
            if (j->Handle == handle)
            {
                j = handlers.erase(j);
                break;
            }
            ++j;
        }
        if (handlers.empty())
        {
            i = Handlers.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

bool Phoenix::MessageRouter::AcceptMessage(type_id typeId, const void* data) const
{
    auto iter = Handlers.find(typeId);
    if (iter == Handlers.end())
    {
        return false;
    }

    return std::ranges::any_of(iter->second, [typeId, data](const auto& handler)
    {
        return handler.Func(typeId, data);
    });
}
