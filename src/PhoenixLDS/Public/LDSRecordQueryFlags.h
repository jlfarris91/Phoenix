
#pragma once
#include "Platform.h"

namespace Phoenix::LDS
{
    enum class ELDSObjectRecordQueryFlags : uint8
    {
        None = 0,

        // Don't search for records in base objects.
        Exact = 1,

        // Don't return the default value record defined in the type.
        IgnoreDefaultValue = 2,
    };

    enum class ELDSTypeRecordQueryFlags : uint8
    {
        None = 0,

        // Don't search for records in base types.
        Exact = 1
    };

    enum class ELDSRecordQueryFlags : uint8
    {
        None = 0,

        // Don't search base objects
        Exact = 1,

        // Don't search dynamic catalogs
        StaticOnly = 2,

        // Don't search world catalogs
        SessionOnly = 4,
    };
}
