
#pragma once
#include "Platform.h"

namespace Phoenix::LDS
{
    enum class ELDSRecordQueryFlags : uint8
    {
        None = 0,

        // Don't search for records in base objects.
        Exact = 1,

        // Don't return the default value record defined in the type.
        IgnoreDefaultValue = 2,
    };
}
