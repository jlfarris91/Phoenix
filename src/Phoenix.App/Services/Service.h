#pragma once

#include "Object.h"

namespace Phoenix
{
    class IService : public IObject
    {
        PHX_DECLARE_TYPE_DERIVED(IService, IObject)
    };
}
