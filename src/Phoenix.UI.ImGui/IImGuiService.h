#pragma once

#include "Application/AppService.h"

namespace Phoenix::UI
{
    class IImGuiService : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(IImGuiService, IAppService)
    };
}
