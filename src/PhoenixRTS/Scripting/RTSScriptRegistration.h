#pragma once

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    // Call from application startup (before the first Session is created) to ensure
    // the RTS script registrations are linked in.
    //
    // This function exists solely to prevent the MSVC linker from dead-stripping
    // RTSScriptRegistration.obj from PhoenixRTS.lib. The actual work is done by
    // the static initializers defined by PHX_SCRIPT_REGISTRATION.
    PHOENIX_RTS_API void EnsureScriptRegistrations();
}
