#pragma once

#include <cstdint>

#include "Phoenix/Name.h"

namespace Phoenix::Renderer
{
    struct HResource
    {
        uint32_t Id = 0;
        FName Type;

        bool IsValid() const { return Id != 0; }
        bool operator==(const HResource&) const = default;
    };
}
